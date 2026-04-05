#include "udp.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
static const char *TAG = "UDP_CLIENT";

void udp_client_task(void *pvParameters) {
  char rx_buffer[MAX_BUFFER_SIZE];
  char host_ip[] = UDP_SERVER_IP;
  int addr_family = AF_INET;
  int ip_protocol = IPPROTO_UDP;
  struct sockaddr_in dest_addr; // For IPv4

  // Prepare destination address (server's address)
  dest_addr.sin_addr.s_addr = inet_addr(host_ip);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(UDP_SERVER_PORT);

  // Create socket
  int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
  if (sock < 0) {
    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    vTaskDelete(NULL);
    return;
  }
  ESP_LOGI(TAG, "Socket created, sending to %s:%d", host_ip, UDP_SERVER_PORT);

  // Set a timeout for recvfrom
  struct timeval timeout;
  timeout.tv_sec = 5; // 5 seconds timeout
  timeout.tv_usec = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
      0) {
    ESP_LOGE(TAG, "Error setting socket receive timeout: errno %d", errno);
    // Continue without timeout if setting fails, or handle error
  }

  for (int i = 0; i < 5; i++) { // Send a few messages
    // Send message
    int err = sendto(sock, CLIENT_MESSAGE, strlen(CLIENT_MESSAGE), 0,
                     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
      ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
      // No break, client might try again
    } else {
      ESP_LOGI(TAG, "Message sent: %s", CLIENT_MESSAGE);
    }

    // Receive response (echo)
    ESP_LOGI(TAG, "Waiting for response...");
    struct sockaddr_storage source_addr; // Can store IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);
    int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                       (struct sockaddr *)&source_addr, &socklen);

    if (len < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        ESP_LOGE(TAG, "Receive timeout / no data received.");
      } else {
        ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
      }
    } else {
      rx_buffer[len] = 0; // Null-terminate
      ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
      // Optionally, check if source_addr matches the server's address
    }
    vTaskDelay(
        pdMS_TO_TICKS(2000)); // Wait 2 seconds before sending next message
  }

  ESP_LOGI(TAG, "Client task finished. Shutting down socket.");
  close(sock);
  vTaskDelete(NULL);
}

// In your app_main:
// ESP_ERROR_CHECK(nvs_flash_init());
// ESP_ERROR_CHECK(esp_netif_init());
// ESP_ERROR_CHECK(esp_event_loop_create_default());
// ESP_ERROR_CHECK(example_connect()); // Your Wi-Fi connection function
// xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
