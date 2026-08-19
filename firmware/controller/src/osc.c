#include "osc.h"
#include "netusb.h"

#include <lwip/def.h>
#include <lwip/err.h>
#include <lwip/ip_addr.h>
#include <lwip/opt.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <pico/error.h>
#include <stdint.h>
#include <stdio.h>

#include <string.h>

static struct udp_pcb *pcb      = NULL;
static osc_recv_fn     recv_fn  = NULL;
static void           *recv_arg = NULL;

void osc_recv(osc_recv_fn _recv_fn, void *_recv_arg) {
	recv_fn  = _recv_fn;
	recv_arg = _recv_arg;
}

static void osc_recv_proc(
    void            *arg,
    struct udp_pcb  *osc_pcb,
    struct pbuf     *p,
    const ip_addr_t *addr,
    u16_t            port
);

err_t osc_init() {
	udp_init();
	pcb = udp_new();
	if (pcb == NULL) {
		return ERR_MEM;
	}
	err_t err = udp_bind(pcb, IP_ADDR_ANY, OSC_PORT_IN);
	if (err != ERR_OK) {
		osc_deinit();
		return err;
	}
	udp_recv(pcb, osc_recv_proc, NULL);

	err = udp_connect(pcb, netusb_broadcast_ip(), OSC_PORT_OUT);
	if (err != ERR_OK) {
		osc_deinit();
		return err;
	}
	return ERR_OK;
}

void osc_deinit() {
	if (pcb != NULL) {
		udp_remove(pcb);
		pcb = NULL;
	}
}

inline static uint16_t osc_pad_strlen(uint16_t len) {
	return len + 4 - len % 4;
}

inline static uint16_t _strnlen(const char *str, size_t buffer_len) {
	const char *end = memchr(str, '\0', buffer_len);
	if (end == NULL) {
		return buffer_len + 1;
	}
	return end - str;
}

inline static uint16_t osc_encode_string(char *dst, const char *str) {
	uint16_t strsize = strlen(str);
	uint16_t padding = 4 - strsize % 4;
	memcpy(dst, str, strsize);
	for (uint8_t i = strsize; i < strsize + padding; ++i) {
		dst[i] = 0;
	}
	return strsize + padding;
}

static const char *osc_type_tag[6] = {",i", ",f", ",s", ",r", ",T", ",F"};

inline static OSC_type_e osc_from_type_tag(char tag) {
	switch (tag) {
	case 'i':
		return OSC_INT32;
	case 'f':
		return OSC_FLOAT32;
	case 's':
		return OSC_STRING;
	case 'r':
		return OSC_RGBA;
	case 'T':
		return OSC_TRUE;
	case 'F':
		return OSC_FALSE;
	}
	return OSC_TYPE_UNKNOWN;
}

inline static uint16_t osc_argument_size(const OSC_argument_t *a) {
	switch (a->type) {
	case OSC_INT32:
	case OSC_FLOAT32:
	case OSC_RGBA:
		return 4;
	case OSC_TRUE:
	case OSC_FALSE:
		return 0;
	case OSC_STRING:
		return osc_pad_strlen(strlen(a->data.string));
	}
	return 0;
}

inline static uint16_t osc_argument_encode(char *dst, const OSC_argument_t *a) {
	switch (a->type) {
	case OSC_RGBA:
	case OSC_FLOAT32:
	case OSC_INT32: {
		uint32_t be_value = lwip_htonl(a->data.integer);
		memcpy(dst, &be_value, sizeof(uint32_t));
		return sizeof(uint32_t);
	}
	case OSC_TRUE:
	case OSC_FALSE:
		return 0;
	case OSC_STRING:
		return osc_encode_string(dst, a->data.string);
	}
	return 0;
}

inline static int osc_argument_parse(
    OSC_argument_t *a, const char *src, uint16_t len, uint16_t *read
) {
	switch (a->type) {
	case OSC_RGBA:
	case OSC_FLOAT32:
	case OSC_INT32: {
		if (len < 4) {
			return PICO_ERROR_BUFFER_TOO_SMALL;
		}
		uint32_t be_value;
		memcpy(&be_value, src, sizeof(uint32_t));
		a->data.integer = lwip_ntohl(be_value);
		*read           = sizeof(uint32_t);
		return PICO_OK;
	}
	case OSC_TRUE:
	case OSC_FALSE:
		*read = 0;
		return PICO_OK;
	case OSC_STRING: {
		a->data.string = (char *)src;
		uint16_t l     = _strnlen(src, len);
		if (l == len + 1) {
			return PICO_ERROR_INVALID_DATA;
		}
		*read = osc_pad_strlen(l);
		return PICO_OK;
	}
	}
	return PICO_ERROR_NOT_FOUND;
}

err_t osc_send(const OSC_message_t *m) {
	if (m == NULL || m->address == NULL) {
		return ERR_ARG;
	}
	// we need 4 char to encode a single argument type.
	uint16_t size = osc_pad_strlen(strlen(m->address)) + 4 +
	                osc_argument_size(&m->argument);

	if (size > PBUF_POOL_BUFSIZE) {
		return ERR_BUF;
	}

	struct pbuf *packet = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_POOL);
	if (packet == NULL) {
		return ERR_BUF;
	}
	uint16_t cur = 0;
	cur += osc_encode_string(packet->payload, m->address);
	cur += osc_encode_string(
	    &((char *)packet->payload)[cur],
	    osc_type_tag[m->argument.type]
	);
	cur += osc_argument_encode(&((char *)packet->payload)[cur], &m->argument);
	err_t err = udp_send(pcb, packet);
	pbuf_free(packet);
	return err;
}

static void osc_recv_proc(
    void            *arg,
    struct udp_pcb  *osc_pcb,
    struct pbuf     *p,
    const ip_addr_t *addr,
    u16_t            port
) {
	(void)arg;
	static OSC_message_t res;

	size_t address_len = _strnlen(p->payload, p->len);
	if (address_len == p->len + 1) {
		printf(
		    "[osc]: invalid formatted message: .address is not NULL "
		    "terminated.\n"
		);
		pbuf_free(p);
		return;
	}
	address_len = osc_pad_strlen(address_len);
	if (address_len >= (p->len - 4)) {
		printf(
		    "[osc]: invalid formatted message .address=%s\n",
		    (char *)p->payload
		);
		pbuf_free(p);
		return;
	}

	res.address = p->payload;

	const char *type_tag        = p->payload + address_len;
	size_t      type_tag_length = _strnlen(type_tag, p->len - address_len);
	if (type_tag_length == p->len - address_len + 1) {
		printf(
		    "[osc]: invalid formatted message .address=%s: .type_tag is not "
		    "null terminated\n",
		    res.address
		);
		pbuf_free(p);
		return;
	}
	if (type_tag_length < 2 || type_tag[0] != ',') {
		printf(
		    "[osc]: invalid formatted message .address=%s,.type_tag=%s\n",
		    res.address,
		    type_tag
		);
		pbuf_free(p);
		return;
	}
	if (type_tag_length > 2) {
		printf("[osc]: only single argument message are supported\n");
		pbuf_free(p);
		return;
	}
	res.argument.type = osc_from_type_tag(type_tag[1]);
	if (res.argument.type == OSC_TYPE_UNKNOWN) {
		printf(
		    "[osc]: invalid formatted message .address=%s,.type_tag=%s\n",
		    res.address,
		    type_tag
		);
		pbuf_free(p);
		return;
	}
	size_t offset = address_len + osc_pad_strlen(type_tag_length);

	uint16_t read = 0;
	int      err  = osc_argument_parse(
        &res.argument,
        p->payload + offset,
        p->len - offset,
        &read
    );
	if (err != PICO_OK) {
		printf(
		    "[osc]: invalid formatted message .address=%s,.type_tag=%s: could "
		    "not parse argument: %d\n",
		    res.address,
		    type_tag,
		    err
		);
		pbuf_free(p);
		return;
	}
	offset += read;

	if (recv_fn != NULL) {
		recv_fn(recv_arg, &res);
	} else {
		printf("[osc]: received Message .address=%s\n", res.address);
	}

	pbuf_free(p);
}
