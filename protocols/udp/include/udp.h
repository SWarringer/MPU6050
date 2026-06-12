#pragma once

#include <lwip/netdb.h>
#define UDP_SERVER_IP "192.168.0.172"
#define UDP_SERVER_PORT 1234
#define MAX_BUFFER_SIZE 128
#define CLIENT_MESSAGE "Hello UDP Server from ESP32!"

static void wifi_connect(void);
void udp_client_task(void *pvParameters);
void send_and_receive(int sock, struct sockaddr_in dest_addr, char *rx_buffer);
