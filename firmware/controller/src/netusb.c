#include "netusb.h"

#include "device/usbd.h"
#include "tusb_config.h"

#include <dhserver.h>
#include <lwip/netif.h>
#include <pico/platform/common.h>
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
    {{0}, INIT_MYSUBNET_IP(12), 24 * 60 * 60},
    {{0}, INIT_MYSUBNET_IP(13), 24 * 60 * 60},
};

static const dhcp_config_t netusb_dhcp_config = {
    .router = INIT_IP4(0, 0, 0, 0),
    .port   = 67,
    .dns    = INIT_IP4(0, 0, 0, 0),
    "usb",
    TU_ARRAY_SIZE(netusb_dhcp_entries),
    netusb_dhcp_entries
};

// this function glues liwp -> tud for xmit
static err_t linkoutput_fn(__unused struct netif *netif, struct pbuf *p) {
	for (;;) {
		if (!tud_ready()) {
			return ERR_USE;
		}
		if (tud_network_can_xmit(p->tot_len)) {
			tud_network_xmit(p, 0 /*unused arg*/);
			return ERR_OK;
		}
	}
	tud_task();
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

static struct pbuf *received_frame;

void tud_network_init_cb() {
	if (received_frame) {
		pbuf_free(received_frame);
		received_frame = NULL;
	}
}

bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
	if (received_frame) {
		return false;
	}
	if (size) {
		struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
		if (p) {
			memcpy(p->payload, src, size);
			received_frame = p;
		}
	}
	return true;
}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
	struct pbuf *p = (struct pbuf *)ref;
	(void)arg;
	return pbuf_copy_partial(p, dst, p->tot_len, 0);
}

static inline void service_traffic() {
	if (received_frame) {
		if (netusb_netif.input(received_frame, &netusb_netif) != ERR_OK) {
			pbuf_free(received_frame);
		}
		received_frame = NULL;
		tud_network_recv_renew();
	}
	sys_check_timeouts();
}

void netusb_task() {
	tud_task();
	service_traffic();
}

static bool netusb_netif_added = false;

bool netusb_init(void) {

	if (!tud_init(BOARD_TUD_RHPORT)) {
		printf("[netusb]: tud_init failed\n");
		return false;
	}

	struct netif *intf = &netusb_netif;

	lwip_init();

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

	printf("[netusb]: setup complete");

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
	static const ip4_addr_t broadcast = INIT_MYSUBNET_IP(255);
	return &broadcast;
}
