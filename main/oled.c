#include "oled.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "u8g2_esp32_hal.h"

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <string.h>
#include "time_driver.h"

#include "esp_sntp.h"
#define PIN_SDA 21
#define PIN_SCL 22
#define I2C_ADDRESS 0x78
static const char *TAG = "oled";

static void oled_task(void *pvParameters);
static void oled_hal_init(void);
//static void test_oled(void);

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
#if 0
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
#endif
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
    wifi_icon_sta = WIFI_ICON_NONE;
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
const char *week_buf[]={"Sun","Mon","Tue","Wed","Thur","Fri","Sat"};
void check_need_update_y_m_d(struct tm *new_time)
{
  static struct tm old_time={0};
  char time_buf[30];
  if((old_time.tm_year!=new_time->tm_year)\
      ||(old_time.tm_mon!=new_time->tm_mon)\
      ||(old_time.tm_mday!=new_time->tm_mday))
      {
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 100, 8);
        u8g2_SetDrawColor(&u8g2, 1);   
        memcpy(&old_time,new_time,sizeof(struct tm));
        sprintf(time_buf,"%02d-%02d %s",old_time.tm_mon+1,old_time.tm_mday,week_buf[old_time.tm_wday]);
        ESP_LOGI(TAG,"set date %s",time_buf);
        u8g2_SetFont(&u8g2, u8g2_font_courB08_tf);
        u8g2_DrawStr(&u8g2, 10, 8, time_buf);     
        u8g2_SendBuffer(&u8g2);
      }




}
void show_time(void)
{
  static uint32_t time_count = 0;
  static unsigned char old_hour=24;
  static unsigned char old_min=0;
  unsigned char hour=0;
  unsigned char min=0;
  char time_str[10];
  timer_event_t evt;
  if (xQueueReceive(timer_queue, &evt, 0) == pdTRUE)
  {
    time_count++;
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    hour=timeinfo.tm_hour;
    min=timeinfo.tm_min;
    if (hour!=old_hour || min!=old_min)
    {
      old_hour=hour;
      old_min=min;
      sprintf(time_str,"%02d:%02d",old_hour,old_min);
      u8g2_SetFont(&u8g2, u8g2_font_inb21_mf);
      u8g2_DrawStr(&u8g2, 16, 40, time_str);
      check_need_update_y_m_d(&timeinfo);      
    }
    u8g2_SetFont(&u8g2, u8g2_font_inb21_mf);
    if (time_count % 2 == 0)
    {
      
    u8g2_DrawStr(&u8g2, 54, 40, ":");
 
    }
    else
    {
        u8g2_DrawStr(&u8g2, 54, 40, " ");
    }

    

     u8g2_SendBuffer(&u8g2);
  }

  // u8g2_SetDrawColor(&u8g2, 1);
  // u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_gb2312a);
  // u8g2_DrawUTF8(&u8g2, 0, 22, "03-01 12 12 日");
  // u8g2_SendBuffer(&u8g2);
}
 #include <string.h>
#define OLED_PX 128
#define FONT_SIZE 8
void show_text(const char *text)
{
  int test_len=17*FONT_SIZE;
  static unsigned char time_count =0;
  static int text_pos=OLED_PX;
  unsigned char offset=2;
  time_count ++;
  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_gb2312a);
  if(time_count>=1)//移动速度
  {
    time_count=0;
    //移动文字
    if(test_len>OLED_PX)
    {
      u8g2_SetDrawColor(&u8g2, 0);
      u8g2_DrawBox(&u8g2, 0, 45, 128, 18);
      u8g2_SetDrawColor(&u8g2, 1);
      u8g2_DrawUTF8(&u8g2, text_pos, 61, text);
      text_pos-=offset;//移动
      //重新移动
      if(text_pos<(-test_len))
      {
        text_pos=OLED_PX;
      }

    }
    else
    {
      //显示屏可以显示
      u8g2_DrawUTF8(&u8g2, 0, 61, text);
    }

  }

  
  u8g2_SendBuffer(&u8g2);
}

unsigned char cacl_rol_1(unsigned char data)
{
    unsigned  char count = 0;
    while (data & 0x80)
    {
        count ++;
        data <<= 1;
    }
    return count;

}
unsigned char cacl_test_size(const char *test)
{
    unsigned char ret;
    unsigned char cacl_size = 0;
    while (*test)
    {
        ret = cacl_rol_1(*test);
        if(ret == 0) //ascall码
        {
            test++;
            cacl_size++;
        }
        else
        {
            test += 3;
            cacl_size += 2;
        }
    }
    return cacl_size;


}

static void oled_task(void *pvParameters)
{
  const char * text="一路向前 莫问前程";
  unsigned char text_len;
  text_len=cacl_test_size(text);
  ESP_LOGI(TAG,"test len %d",text_len);
  //test_oled();
  time_driver_init();



  while (1)
  {
    show_text(text);
    show_time();
    show_wifi_icon();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
