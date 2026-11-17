#include <cstdio>
#include <cstdint>

#include "esp_mac.h"

extern "C" void app_main()
{
    uint8_t mac[6];

    esp_err_t err = esp_read_mac(mac, ESP_MAC_WIFI_STA);

    if (err == ESP_OK) {
        printf("WiFi STA MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
               mac[0], mac[1], mac[2],
               mac[3], mac[4], mac[5]);
    } else {
        printf("Failed to read MAC address: %s\n", esp_err_to_name(err));
    }
}
