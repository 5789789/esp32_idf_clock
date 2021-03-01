#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "console.h"
#include "oled.h"
#include "cmd_wifi.h"

void app_main(void)
{
    
    oled_init();
    console_task_init();
    wifi_para_init();
    while (1)
    {
        

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
