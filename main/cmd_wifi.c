/* Console example — WiFi commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "cmd_decl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "cmd_wifi.h"
#include "cmd_nvs.h"
#define JOIN_TIMEOUT_MS (10000)

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

static void initialise_wifi(void)
{
    esp_log_level_set("wifi", ESP_LOG_WARN);
    static bool initialized = false;
    if (initialized)
    {
        return;
    }
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif);
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
    initialized = true;
}

static bool wifi_join(const char *ssid, const char *pass, int timeout_ms)
{
    initialise_wifi();
    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    if (pass)
    {
        strlcpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());

    int bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                                   pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS);
    return (bits & CONNECTED_BIT) != 0;
}

/** Arguments used by 'join' function */
static struct
{
    struct arg_int *timeout;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} join_args, wifi_para_args;

static int connect(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&join_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, join_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Connecting to '%s'",
             join_args.ssid->sval[0]);

    /* set default value*/
    if (join_args.timeout->count == 0)
    {
        join_args.timeout->ival[0] = JOIN_TIMEOUT_MS;
    }

    bool connected = wifi_join(join_args.ssid->sval[0],
                               join_args.password->sval[0],
                               join_args.timeout->ival[0]);
    if (!connected)
    {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    ESP_LOGI(__func__, "Connected");
    return 0;
}

void register_wifi(void)
{
    join_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    join_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    join_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    join_args.end = arg_end(2);

    const esp_console_cmd_t join_cmd = {
        .command = "join",
        .help = "Join WiFi AP as a station",
        .hint = NULL,
        .func = &connect,
        .argtable = &join_args};

    ESP_ERROR_CHECK(esp_console_cmd_register(&join_cmd));
}

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi_types.h"
#include "esp_log.h"
static const char *TAG = "cmd_wifi";

const char *wifi_config = "wifi_config";

void wifi_para_init(void)
{
    nvs_handle handle;

    wifi_config_t wifi_config_stored;
    memset(&wifi_config_stored, 0x0, sizeof(wifi_config_stored));
    uint32_t len = sizeof(wifi_config_stored);

    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));

esp_err_t err;
 err=nvs_get_blob(handle, wifi_config, &wifi_config_stored, &len);
    switch (err) {
    case ESP_OK:
         printf("get wifi para: ssid:%s passwd:%s\r\n", wifi_config_stored.sta.ssid, wifi_config_stored.sta.password);
             bool connected = wifi_join((const char *)wifi_config_stored.sta.ssid,
                               (const char *)wifi_config_stored.sta.password,
                               JOIN_TIMEOUT_MS);
    if (!connected)
    {
        ESP_LOGW(__func__, "Connection timed out");
  
    }
    ESP_LOGI(__func__, "Connected");
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        printf("The value is not initialized yet!\n");
        break;
    default :
        printf("Error (%s) reading!\n", esp_err_to_name(err));
    }



   

    nvs_close(handle);
}

static int set_wifi_para(int argc, char **argv)
{

    int nerrors = arg_parse(argc, argv, (void **)&wifi_para_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, wifi_para_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Connecting to '%s'",
             wifi_para_args.ssid->sval[0]);

    /* set default value*/
    if (wifi_para_args.timeout->count == 0)
    {
        wifi_para_args.timeout->ival[0] = JOIN_TIMEOUT_MS;
    }

  

    nvs_handle handle;
    wifi_config_t wifi_config_to_store;
    memset(&wifi_config_to_store, '\0', sizeof(wifi_config_to_store));
    strcpy((char *)wifi_config_to_store.sta.ssid,wifi_para_args.ssid->sval[0]);
    strncpy((char *)(wifi_config_to_store.sta.password) ,wifi_para_args.password->sval[0],63);
    	ESP_LOGI(TAG, "set wifi para: ssid:%s passwd:%s\r\n", wifi_config_to_store.sta.ssid, wifi_config_to_store.sta.password);

    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));

    ESP_ERROR_CHECK(nvs_set_blob(handle, wifi_config, &wifi_config_to_store, sizeof(wifi_config_to_store)));

    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);

    // bool connected = wifi_join(join_args.ssid->sval[0],
    //                            join_args.password->sval[0],
    //                            join_args.timeout->ival[0]);
    // if (!connected)
    // {
    //     ESP_LOGW(__func__, "Connection timed out");
    //     return 1;
    // }
    // ESP_LOGI(__func__, "Connected");
    return 0;
}

void register_set_wifi_para(void)
{
    wifi_para_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    wifi_para_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    wifi_para_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    wifi_para_args.end = arg_end(2);

    const esp_console_cmd_t join_cmd = {
        .command = "set_wifi",
        .help = "set Join WiFi AP para",
        .hint = NULL,
        .func = &set_wifi_para,
        .argtable = &wifi_para_args};

    ESP_ERROR_CHECK(esp_console_cmd_register(&join_cmd));
}
