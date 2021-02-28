#include "oled.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "u8g2_esp32_hal.h"

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>

#define PIN_SDA 21
#define PIN_SCL 22
#define I2C_ADDRESS 0x78
static const char *TAG = "oled";

static void oled_task(void *pvParameters);
static void oled_hal_init(void);
static void test_oled(void);

u8g2_t u8g2;

void oled_init(void)
{
  oled_hal_init();
  xTaskCreate((TaskFunction_t)oled_task, /* 任务函数 */
              (const char *)"oled_task", /* 任务名称*/
              (uint16_t)2048,            /* 任务堆栈大小，单位为字节*/
              (void *)NULL,              /* 传递给任务函数的参数*/
              (UBaseType_t)15,           /* 任务优先级,最高优先级为24 */
              (TaskHandle_t *)NULL);
}

static void oled_hal_init(void)
{
  u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
  u8g2_esp32_hal.sda = PIN_SDA;
  u8g2_esp32_hal.scl = PIN_SCL;
  u8g2_esp32_hal_init(u8g2_esp32_hal);
  u8g2_Setup_ssd1306_i2c_128x64_noname_f(
      &u8g2,
      U8G2_R0,
      //u8x8_byte_sw_i2c,
      u8g2_esp32_i2c_byte_cb,
      u8g2_esp32_gpio_and_delay_cb); // init u8g2 structure
  u8x8_SetI2CAddress(&u8g2.u8x8, I2C_ADDRESS);

  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0); // wake up display

  u8g2_ClearBuffer(&u8g2);
}

static void test_oled(void)
{

#if 1
  ESP_LOGI(TAG, "u8g2_DrawBox");
  u8g2_DrawBox(&u8g2, 0, 26, 80, 6);
  u8g2_DrawFrame(&u8g2, 0, 26, 100, 6);

  ESP_LOGI(TAG, "u8g2_SetFont");
  u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
  ESP_LOGI(TAG, "u8g2_DrawStr");
  u8g2_DrawStr(&u8g2, 2, 17, "Hi nkolban!");
  u8g2_SetFont(&u8g2, u8g2_font_wqy13_t_gb2312a);
  u8g2_DrawUTF8(&u8g2, -5, 50, "测试123456");
  ESP_LOGI(TAG, "u8g2_SendBuffer");
  u8g2_SendBuffer(&u8g2);
#endif
#if 0
	 u8g2_SetFontMode(&u8g2, 1);  // Transparent
    u8g2_SetFontDirection(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(&u8g2, 0, 20, "U");
    
    u8g2_SetFontDirection(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_inb30_mn);
    u8g2_DrawStr(&u8g2, 21,8,"8");
        
    u8g2_SetFontDirection(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(&u8g2, 51,30,"g");
    u8g2_DrawStr(&u8g2, 67,30,"\xb2");
    
    u8g2_DrawHLine(&u8g2, 2, 35, 47);
    u8g2_DrawHLine(&u8g2, 3, 36, 47);
    u8g2_DrawVLine(&u8g2, 45, 32, 12);
    u8g2_DrawVLine(&u8g2, 46, 33, 12);
  
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);
    u8g2_DrawStr(&u8g2, 1,54,"github.com/olikraus/u8g2");


    u8g2_SendBuffer(&u8g2);
#endif
}

#define WIFI__SHOW 0
#define WIFI_ICON_TOGGLE 1
#define WIFI_ICON_NONE 255

static uint8_t wifi_icon_sta = WIFI_ICON_TOGGLE;
void show_wifi_icon(void)
{
  static uint8_t toggle_count = 0;

  switch (wifi_icon_sta)
  {
  case WIFI__SHOW:
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_open_iconic_embedded_1x_t);
    u8g2_DrawGlyph(&u8g2, 120, 8, 80);
    u8g2_SendBuffer(&u8g2);
    wifi_icon_sta=WIFI_ICON_NONE;
    break;
  case WIFI_ICON_TOGGLE:
    toggle_count++;
    if (toggle_count == 30)
    {
      //show
        u8g2_SetDrawColor(&u8g2, 1);
      u8g2_SetFont(&u8g2, u8g2_font_open_iconic_embedded_1x_t);
      u8g2_DrawGlyph(&u8g2, 120, 8, 80);
      u8g2_SendBuffer(&u8g2);
    }
    else if (toggle_count >= 60)
    {
      //hide
      u8g2_SetDrawColor(&u8g2, 0);
      u8g2_DrawBox(&u8g2, 120, 0, 8, 8);
      u8g2_SendBuffer(&u8g2);
      u8g2_SetDrawColor(&u8g2, 1);
      toggle_count = 0;
    }

    break;
  default:
    break;
  }
}

void show_time(void)
{
  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
  u8g2_DrawStr(&u8g2, 0, 22, "12:12");
  u8g2_SendBuffer(&u8g2);
}

static void oled_task(void *pvParameters)
{
  //test_oled();

  show_time();

  while (1)
  {
    show_wifi_icon();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
