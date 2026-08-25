#include "netusb.h"

#include "atomic.h"
#include "class/hid/hid_device.h"
#include "class/net/net_device.h"
#include "device/usbd.h"
#include "tusb_config.h"

#include <dhserver.h>
#include <lwip/netif.h>
#include <pico/error.h>
#include <pico/platform/common.h>
#include <pico/util/queue.h>
#include <tusb.h>

#include <lwip/apps/httpd.h>
#include <lwip/apps/mdns.h>
#include <lwip/etharp.h>
#include <lwip/init.h>
#include <lwip/ip4_addr.h>
#include <lwip/timeouts.h>

#include <pico/unique_id.h>

static struct netif netusb_netif;

uint8_t tud_network_mac_address[6] = {0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

#define INIT_IP4(a, b, c, d)                                                   \
	{ PP_HTONL(LWIP_MAKEU32(a, b, c, d)) }
#define INIT_MYSUBNET_IP(d) INIT_IP4(10, 42, 84, d)

static const ip4_addr_t netusb_ipaddr  = INIT_MYSUBNET_IP(2);
static const ip4_addr_t netusb_netmask = INIT_IP4(255, 255, 255, 0);
static const ip4_addr_t netusb_gateway = INIT_IP4(0, 0, 0, 0);

static dhcp_entry_t netusb_dhcp_entries[] = {
    {{0}, INIT_MYSUBNET_IP(11), 24 * 60 * 60},
};

static const dhcp_config_t netusb_dhcp_config = {
    .router = INIT_IP4(0, 0, 0, 0),
    .port   = 67,
    .dns    = INIT_IP4(0, 0, 0, 0),
    "usb",
    TU_ARRAY_SIZE(netusb_dhcp_entries),
    netusb_dhcp_entries
};

static queue_t tx_queue, keyboard_queue;

// this function glues liwp -> tud for xmit
static err_t linkoutput_fn(__unused struct netif *netif, struct pbuf *p) {
	pbuf_ref(p);
	if (queue_try_add(&tx_queue, &p) == false) {
		pbuf_free(p);
		return ERR_USE;
	}
	return ERR_OK;
}

static err_t netif_init_cb(struct netif *netif) {
	LWIP_ASSERT("netif != NULL", (netif != NULL));
	netif->mtu = CFG_TUD_NET_MTU;

	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
	               NETIF_FLAG_LINK_UP | NETIF_FLAG_UP | NETIF_FLAG_IGMP;

	netif->state      = NULL;
	netif->name[0]    = 'E';
	netif->name[1]    = 'X';
	netif->linkoutput = linkoutput_fn;
	// how do we send ethernet frames?
	netif->output     = etharp_output;
	return ERR_OK;
}

void tud_network_init_cb() {}

static bool rx_pending = false;

bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
	if (size == 0) {
		return true;
	}

	struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
	if (p == NULL) {
		printf("[netusb]: error could not allocate PBUF for reception\n");
		rx_pending = true;
		return false;
	}
	err_t err = pbuf_take(p, src, size);
	if (err != ERR_OK) {
		pbuf_free(p);
		printf(
		    "[netusb]: error could not copy PBUF memory for reception: %d\n",
		    err
		);
		rx_pending = true;
		return false;
	}

	err = netusb_netif.input(p, &netusb_netif);
	if (err != ERR_OK) {
		printf("[netusb]: error could not process input packet: %d\n", err);
		pbuf_free(p);
	}
	tud_network_recv_renew();

	return true;
}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
	struct pbuf *p = (struct pbuf *)ref;
	(void)arg;
	return pbuf_copy_partial(p, dst, p->tot_len, 0);
}

void netusb_task() {
	tud_task();

	bool need_renew;
	ATOMIC_CORE_BLOCK() {
		need_renew = rx_pending;
		rx_pending = false;
	}
	if (need_renew) {
		tud_network_recv_renew();
	}

	sys_check_timeouts();

	for (;;) {
		struct pbuf *p;
		if (tud_ready() == false || queue_try_peek(&tx_queue, &p) == false) {
			// not ready, or nothing to TX
			break;
		}
		if (tud_network_can_xmit(p->tot_len) == false) {
			// not ready to TX that much
			break;
		}

		queue_try_remove(&tx_queue, &p);

		// sending it.
		tud_network_xmit(p, 0);
		// release it.

		pbuf_free(p);
	}

	keyboard_keypress_t kp;

	if (tud_hid_ready() == false ||
	    queue_try_peek(&keyboard_queue, &kp) == false) {
		return;
	}
	uint8_t keycode[6] = {kp.key, 0, 0, 0, 0, 0};
	if (tud_hid_keyboard_report(0, kp.modifiers, keycode) == false) {
		printf("[netusb] could not send keyboard event\n");
		return;
	} else {
		printf("[netusb] send keyboard event %x %x\n", kp.modifiers, kp.key);
	}
	queue_try_remove(&keyboard_queue, &kp);
}

static bool netusb_netif_added = false;

bool netusb_init(void) {

	if (!tud_init(BOARD_TUD_RHPORT)) {
		printf("[netusb]: tud_init failed\n");
		return false;
	}

	struct netif *intf = &netusb_netif;

	lwip_init();

	queue_init(&tx_queue, sizeof(struct pbuf *), 32);
	queue_init(&keyboard_queue, sizeof(keyboard_keypress_t), 32);

	pico_unique_board_id_t board_id;
	pico_get_unique_board_id(&board_id);

	memcpy(tud_network_mac_address, &board_id.id[sizeof(board_id.id) - 6], 6);
	tud_network_mac_address[0] = 0x02 | (tud_network_mac_address[0] & 0x01);
	intf->hwaddr_len           = sizeof(tud_network_mac_address);
	memcpy(
	    intf->hwaddr,
	    tud_network_mac_address,
	    sizeof(tud_network_mac_address)
	);
	intf->hwaddr[5] ^= 0x01;

	printf(
	    "[netusb]: Using MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
	    tud_network_mac_address[0],
	    tud_network_mac_address[1],
	    tud_network_mac_address[2],
	    tud_network_mac_address[3],
	    tud_network_mac_address[4],
	    tud_network_mac_address[5]
	);

	intf = netif_add(
	    intf,
	    &netusb_ipaddr,
	    &netusb_netmask,
	    &netusb_gateway,
	    NULL,
	    netif_init_cb,
	    ethernet_input
	);
	if (intf == NULL) {
		printf("[netusb]:error adding netif.\n");
		return false;
	}

	netif_set_default(intf);
	netusb_netif_added = true;

	while (!netif_is_up(intf)) {
		tight_loop_contents();
	}
	while (dhserv_init(&netusb_dhcp_config) != ERR_OK) {
		tight_loop_contents();
	}

	httpd_init();
	mdns_resp_init();
	mdns_resp_add_netif(intf, "woodpecker");

	printf("[netusb]: setup complete\n");

	return true;
}

void netusb_deinit() {

	if (netusb_netif_added) {
		mdns_resp_remove_netif(&netusb_netif);
		dhserv_free();
		netif_remove(&netusb_netif);
		netusb_netif_added = false;
	}
	netusb_netif.flags = 0;
	tud_deinit(BOARD_TUD_RHPORT);
}

const ip4_addr_t *netusb_own_ip() {
	return &netusb_ipaddr;
}

const ip4_addr_t *netusb_broadcast_ip() {
	static const ip4_addr_t broadcast = INIT_MYSUBNET_IP(11);
	return &broadcast;
}

int netusb_enqueue_keypress(keyboard_keypress_t kp) {
	if (queue_try_add(&keyboard_queue, &kp) == false) {
		printf(
		    "[netusb] could not enqueue: %x, key: %x\n",
		    kp.modifiers,
		    kp.key
		);
		return PICO_ERROR_BUFFER_TOO_SMALL;
	}
	printf(
	    "[netusb] enqueued keypress modifiers: %x, key: %x\n",
	    kp.modifiers,
	    kp.key
	);
	return PICO_OK;
}
