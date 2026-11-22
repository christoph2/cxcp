#include "app_config.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "xcp.h"

Xcp_OptionsType Xcp_Options = {
#if defined(CONFIG_EXAMPLE_IPV6)
    .ipv6 = true,
#else
    .ipv6 = false,
#endif
    .tcp  = false,
    .port = (uint16_t)XCP_ON_ETHERNET_PORT,
};

void app_main(void) {
    esp_err_t err = nvs_flash_init();

    if ((err == ESP_ERR_NVS_NO_FREE_PAGES) || (err == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    {
        esp_task_wdt_config_t wdt_config = {
            .timeout_ms     = 10000,
            .idle_core_mask = 0,
            .trigger_panic  = true,
        };
        esp_err_t wdt_err = esp_task_wdt_deinit();

        if ((wdt_err != ESP_OK) && (wdt_err != ESP_ERR_INVALID_STATE)) {
            ESP_ERROR_CHECK(wdt_err);
        }
        ESP_ERROR_CHECK(esp_task_wdt_init(&wdt_config));
    }

    ESP_ERROR_CHECK(example_connect());
    FlsEmu_Init(&FlsEmu_Config);
    Xcp_Init();

    for (;;) {
        XcpTl_MainFunction();
        Xcp_MainFunction();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
