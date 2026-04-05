#pragma once

#define UDP_SERVER_IP "192.168.0.155"
#define UDP_SERVER_PORT 1234
#define MAX_BUFFER_SIZE 128
#define CLIENT_MESSAGE "Hello UDP Server from ESP32!"

static void wifi_connect(void);
void udp_client_task(void *pvParameters);
