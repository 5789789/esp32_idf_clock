#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "console.h"
#include "oled.h"
#include "cmd_wifi.h"

#include "esp_sntp.h"

static const char *TAG = "main";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "cn.ntp.org.cn");
    sntp_setservername(1, "edu.ntp.org.cn");
    sntp_setservername(2, "ntp1.aliyun.com");
    sntp_setservername(3, "ntp2.aliyun.com");
    sntp_setservername(4, "cn.pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    sntp_init();
}


static void obtain_time(void)
{


    initialize_sntp();

    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
      setenv("TZ", "CST-8", 1);
    tzset();


   
}

void app_main(void)
{
    
    oled_init();
    console_task_init();
    wifi_para_init();

    obtain_time();


    while (1)
    {

    
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
