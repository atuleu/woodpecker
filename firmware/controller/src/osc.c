#include "osc.h"
#include "netusb.h"

#include <lwip/def.h>
#include <lwip/err.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
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
	return ERR_OK;
}

void osc_deinit() {
	if (pcb != NULL) {
		udp_remove(pcb);
		pcb = NULL;
	}
}

inline static uint16_t osc_strlen(const char *str) {
	uint16_t res = strlen(str);
	return res + 4 - res % 4;
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
		return osc_strlen(a->data.string);
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

inline static uint16_t osc_argument_parse(OSC_argument_t *a, const char *src) {
	switch (a->type) {
	case OSC_RGBA:
	case OSC_FLOAT32:
	case OSC_INT32: {
		uint32_t be_value;
		memcpy(&be_value, src, sizeof(uint32_t));
		a->data.integer = lwip_ntohl(be_value);
		return sizeof(uint32_t);
	}
	case OSC_TRUE:
	case OSC_FALSE:
		return 0;
	case OSC_STRING: {
		a->data.string = (char *)src;
		return osc_strlen(src);
	}
	}
	return 0;
}

err_t osc_send(const OSC_message_t *m) {
	if (m == NULL || m->address == NULL) {
		return ERR_ARG;
	}
	uint16_t size = osc_strlen(m->address) + 4 +
	                osc_argument_size(&m->argument
	                ); // we need 4 char to encode a single argument type.

	struct pbuf *packet = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_POOL);
	uint16_t     cur    = 0;
	cur += osc_encode_string(packet->payload, m->address);
	cur += osc_encode_string(
	    &((char *)packet->payload)[cur],
	    osc_type_tag[m->argument.type]
	);
	cur += osc_argument_encode(&((char *)packet->payload)[cur], &m->argument);
	err_t err = udp_sendto(pcb, packet, netusb_broadcast_ip(), OSC_PORT_OUT);
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
	static OSC_message_t res;

	size_t address_len = osc_strlen(p->payload);
	if (address_len >= (p->len - 4)) {
		printf(
		    "[osc]: invalid formatted message .address=%s\n",
		    (char *)p->payload
		);
		return;
	}

	res.address = p->payload;

	const char *type_tag        = p->payload + address_len;
	size_t      type_tag_length = strlen(type_tag);
	if (type_tag_length < 2 || type_tag[0] != ',') {
		printf(
		    "[osc]: invalid formatted message .address=%s,.type_tag=%s\n",
		    res.address,
		    type_tag
		);
	}
	if (type_tag_length > 2) {
		printf("[osc]: only single argument message are supported");
	}
	res.argument.type = osc_from_type_tag(type_tag[1]);
	if (res.argument.type == OSC_TYPE_UNKNOWN) {
		printf(
		    "[osc]: invalid formatted message .address=%s,.type_tag=%s\n",
		    res.address,
		    type_tag
		);
		return;
	}
	size_t offset = address_len + osc_strlen(type_tag);
	offset += osc_argument_parse(&res.argument, p->payload + offset);

	if (recv_fn != NULL) {
		recv_fn(recv_arg, &res);
	} else {
		printf("[osc]: received Message .address=%s\n", res.address);
	}

	pbuf_free(p);
}
