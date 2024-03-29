#ifndef __MAIN_H__
#define __MAIN_H__

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <TJpg_Decoder.h>
#include <WIFIClient.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
// #include <ds18b20.h>
#include <PubSubClient.h>     //mqtt库
#include <ESP8266WebServer.h> // web服务器通信库需要使用
#include <ESP8266HTTPUpdateServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 3 // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11

#include "font/ZdyLwFont_20.h" //字体头文件
#include "font/FxLED_32.h"

#include "img\main_img\main_img.h" //龙猫相关动画
#include "img/pangzi/i0.h"         //太空人图片
#include "img/pangzi/i1.h"
#include "img/pangzi/i2.h"
#include "img/pangzi/i3.h"
#include "img/pangzi/i4.h"
#include "img/pangzi/i5.h"
#include "img/pangzi/i6.h"
#include "img/pangzi/i7.h"
#include "img/pangzi/i8.h"
#include "img/pangzi/i9.h"

unsigned int Gif_Mode = 5;      //
uint8_t wifi_connect_cnt = 120; // wifi连接重试次数。配合500毫秒的间隔，实际时间为120*0.5s = 60秒钟

#include "img/temperature.h"  //温度图标
#include "img/humidity.h"     //湿度图标
#include "img/watch_top.h"    //顶部图标
#include "img/watch_bottom.h" //底部图标

TFT_eSPI tft = TFT_eSPI(); // 引脚请自行配置tft_espi库中的 User_Setup.h文件
TFT_eSprite clk = TFT_eSprite(&tft);
TFT_eSprite clkb = TFT_eSprite(&tft); // 字体滚动用

// const char *WIFI_SSID = "ChinaNet-www"; // 家里无线路由器的账号和密码，----------要修改成自己的----------，引号不要去掉
// const char *WIFI_PASSWORD = "PCbp#mn3h80ID&Pk";

bool wifiConnected = false;    // wifi 是否连接上的标记
uint8_t SmartConfigStatus = 0; // 是否在smartconfig自动配网状态，0，未开始，1，等待配置 2，收到配置，连接wifi，3 连接wifi配置错误 ，4。wifi正常连接
int pos = 24;                  // 天气信息滚动使用的字体位置变量。提升成全局变量，解决动画播放卡顿问题

HTTPClient httpClient;            // 创建 HTTPClient 对象
WiFiClient wifiClient;            // 创建WIFIClient对象，新版的HTTPClient要使用
uint8_t loadNum = 6;              // 开机启动进度条的起始位置
uint16_t frontColor = TFT_YELLOW; // 背景颜色
uint16_t bgColor = TFT_BLACK;     // 前景颜色
String cityCode = "101010100";    // 天气城市代码 //通过geo服务自动获取城市后，这个设置可以随便配置了
uint8_t Dis_Count = 0;            // 滚动显示内容计数
String scrollText[8];             // 滚动显示的数据缓冲区
String local_IP;                  // 本地的ip地址

//---------------------NTP相关参数---------------------
static const char ntpServerName[] = "ntp1.aliyun.com"; // NTP服务器
const int timeZone = 8;                                // 时区，东八区为北京时间
WiFiUDP Udp;
unsigned int localPort = 8888;      // 连接时间服务器的本地端口号
time_t prevDisplay = 0;             // 上一次获取到的时间
const int NTP_PACKET_SIZE = 48;     // NTP发送数据包长度
byte packetBuffer[NTP_PACKET_SIZE]; // NTP数据包缓冲区

//---------------------Time 相关参数---------------------
int Led_Flag = HIGH;    // 默认当前灭灯
bool Led_State = false; // 灯状态
unsigned long now1;     // 轮流滚动时间计时
unsigned long LastTime1 = 0;
unsigned long now2; // 定时获取天气
unsigned long LastTime2 = 0;
unsigned long LastTime3 = 0;
unsigned long now3;
unsigned long LastTime4 = 0; //

unsigned long LastTime5 = 0; // dht11 传感器获取时间间隔

//-------------------------mqtt 相关-----------
// MQTT Broker
const char *mqtt_broker = "www.walkingsky.top";
const char *topic = "esp8266/#";
const char *mqtt_username = "esp";
const char *mqtt_password = "esp8266";
const int mqtt_port = 1883;
#define MQTT_TOPIC_PIC "esp8266/pic"
#define MQTT_TOPIC_LED "esp8266/led"
#define MQTT_TOPIC_CMD "esp8266/cmd"

#define LED 16

//---------------------http server 相关------------
/* 2. 创建一个web服务器对象，使用80端口，HTTP网络服务器标准端口号即为80 */
ESP8266WebServer esp8266_server(80);
ESP8266HTTPUpdateServer httpUpdater;

void handleNotFound();
void handleRoot();
void handle_Gif_Mode();
//------------------------eeprom 参数-------------------------
struct WifiConf
{
    char wifi_ssid[50];
    char wifi_password[50];
    // Make sure that there is a 0
    // that terminatnes the c string
    // if memory is not initalized yet.
    char city_id[10];
    char cstr_terminator = 0; // makse sure
    uint16_t gif_mode;
    uint16_t frontColor;
};
WifiConf wifiConf;
void writeWifiConf(); // 写入eeprom wificonfig 参数
//-------------------------------------------------

char temperature_log[10] = {'-'};
char relative_humidity_log[10] = {'-'};
float temperature1 = 0;
float relative_humidity1 = 0;

WiFiClient esp_client;

PubSubClient mqtt_client(esp_client);

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
String num2str(int digits);
void sendNTPpacket(IPAddress &address);

#endif
