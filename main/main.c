#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "console.h"


void app_main(void)
{
    console_task_init();
    int i = 0;
    while (1) {
        //printf("[%d] Hello world!\n", i);
        i++;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
