#include <stdio.h>
#include <lwip/sockets.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "Lcd.h"


#define ESP32_STATIC_IP   //�Ƿ�ʹ�þ�̬IP

#ifdef ESP32_STATIC_IP 
//IP��ַ��
#define DEVICE_IP "192.168.10.1"
//����
#define DEVICE_GW "192.168.10.1"
//����
#define DEVICE_NETMASK "255.255.255.0"
#endif

#define  DEFAULT_SSID "taobao.com"        //��Ҫ���ӵ�WIFI����
#define  DEFAULT_PWD "12345678"   //wifi��Ӧ������

static ip4_addr_t s_ip_addr,s_gw_addr,s_netmask_addr;


//����s��0����ʼ������1�������С���2��������
void lcd_display(int s)
{
  char lcd_buff[100]={0};

  Gui_DrawFont_GBK24(15,0,RED,WHITE,(u8 *)"��Ϊ����");

  Gui_DrawFont_GBK16(16,34,VIOLET,WHITE,(u8 *)"������Ϊ����");
  Gui_DrawFont_GBK16(32,50,BLUE,WHITE,(u8 *)"��ӭ��");

  //��ʾ���ӵ�wifi��Ϣ
  LCD_P6x8Str(0,70,BLACK,WHITE,(u8 *)"mode:AP");
  sprintf(lcd_buff, "ssid:%s",DEFAULT_SSID);
  LCD_P6x8Str(0,80,BLACK,WHITE,(u8 *)lcd_buff);
  sprintf(lcd_buff, "psw:%s",DEFAULT_PWD);
  LCD_P6x8Str(0,90,BLACK,WHITE,(u8 *)lcd_buff);

  if(2==s)
  {
    //2��������
    sprintf(lcd_buff, "ip:%s      ",ip4addr_ntoa(&s_ip_addr));
    LCD_P6x8Str(0,100,BLUE,WHITE,(u8 *)lcd_buff);        
    sprintf(lcd_buff, "gw:%s",ip4addr_ntoa(&s_gw_addr));
    LCD_P6x8Str(0,110,BLUE,WHITE,(u8 *)lcd_buff);        
    sprintf(lcd_buff, "mask:%s",ip4addr_ntoa(&s_netmask_addr));
    LCD_P6x8Str(0,120,BLUE,WHITE,(u8 *)lcd_buff);        
  }
  else
  {
    LCD_P6x8Str(0,100,RED,WHITE,(u8 *)"wifi connecting......");
  }
  
}

// wifi�¼�������
// ctx     :��ʾ������¼����Ͷ�Ӧ��Я���Ĳ���
// event   :��ʾ������¼�����
// ESP_OK  : succeed
// ����    :ʧ�� 
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_AP_START://AP����
      {
        tcpip_adapter_ip_info_t ipInfo;
        printf("\nwifi_softap_start\n");
        //�޸��豸���֣����Բ�д   
        ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP, "yueweidianzi"));
        ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP,&ipInfo));

        //ȡ���豸��IP�����غ�����  
        s_ip_addr=ipInfo.ip;
        s_gw_addr=ipInfo.gw;
        s_netmask_addr=ipInfo.netmask;
        //��ʾ
        lcd_display(2);
      }
        break;
    case SYSTEM_EVENT_AP_STACONNECTED://���豸����
        printf("station:"MACSTR" join, AID=%d.\r\n",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;

    case SYSTEM_EVENT_AP_STADISCONNECTED://���ӵ��豸�Ͽ�
        printf("station:"MACSTR"leave, AID=%d.\r\n",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
        
    default:
        break;
    }
    return ESP_OK;
}

//����WIFI��AP
void wifi_init_ap()
{
    tcpip_adapter_init();//tcp/IP����

#ifdef ESP32_STATIC_IP 
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    tcpip_adapter_ip_info_t ipInfo;
     
    inet_pton(AF_INET,DEVICE_IP,&ipInfo.ip);
    inet_pton(AF_INET,DEVICE_GW,&ipInfo.gw);
    inet_pton(AF_INET,DEVICE_NETMASK,&ipInfo.netmask);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP,&ipInfo));
    tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
#endif

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));//����wifi�¼��ص�����

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));//wifiĬ�ϳ�ʼ��
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD,
            .ssid_len = 0,
            .max_connection = 1,//���ֻ�ܱ�4��stationͬʱ����
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));//����wifi����ģʽΪAP
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));//����AP����
    ESP_ERROR_CHECK(esp_wifi_start());    //wifi����
}


//�û�������ڣ��൱��main����
void app_main()
{    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //��ʾ����ʼ���Լ���ʾ��ص���ʾ
    Lcd_Init();
    lcd_display(0);

    
    wifi_init_ap();//����WIFI��AP
}
