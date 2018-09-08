/**
 * Copyright (c) 2018 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __WIFI_ESP8266_H__
#define __WIFI_ESP8266_H__

#include <net/net_pkt.h>
#include <net/net_if.h>
#include <net/net_l2.h>
#include <net/net_context.h>
#include <net/net_offload.h>
#include <net/wifi_mgmt.h>

#define ESP8266_MAX_CONNECTIONS	4
#define ESP8266_BUF_SIZE	128
#define ESP8266_TCP_RCV_BUF_MAX (3096)

#define EVENT_GOT_IP	 BIT(0)
#define EVENT_CONNECT	 BIT(1)
#define EVENT_DISCONNECT BIT(2)
#define EVENT_SNTP       BIT(3)
#define EVENT_TCP        BIT(4)

struct socket_data {
	struct net_context	*context;
	net_context_connect_cb_t	connect_cb;
	net_tcp_accept_cb_t		accept_cb;
	net_context_send_cb_t		send_cb;
	net_context_recv_cb_t		recv_cb;
	void			*connect_data;
	void			*send_data;
	void			*recv_data;
	void			*accept_data;
	struct net_pkt		*rx_pkt;
	struct net_buf		*pkt_buf;
	int			ret;
	struct	k_sem		wait_sem;
	char tmp[16];
};

struct esp8266_data {
	struct net_if *iface;
	unsigned char mac[6];

	/* max sockets */
	struct socket_data socket_data[4];
	u8_t sock_map;

	/* fifos + accounting info */
	u8_t rx_buf[ESP8266_BUF_SIZE];	/* circular due to parsing */
	u8_t tx_buf[ESP8266_BUF_SIZE];
	u8_t rx_head;
	u8_t rx_tail;
	u8_t rx_full;
	size_t tx_head;
	size_t tx_tail;

	struct k_delayed_work work;
	u8_t event;

    u8_t transparent;

	int transaction;
	int resp;
	int initialized;
	int connected;

	/* delayed work requests */

	/* device management */
	struct device *uart_dev;
	struct device *gpio_dev;
};
struct esp8266_data foo_data;
char rx_buf[ESP8266_TCP_RCV_BUF_MAX];

time_t k_time(time_t *ptr);

#endif /* __WIFI_ESP8266_H__ */
