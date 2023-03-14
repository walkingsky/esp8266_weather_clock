//**********************************************************************
// 本程序是网上开源的，我做了整理规范，修改了部分内容，添加了注释，编写了教程。

//*210409:
//**********************************************************************
#include "main.h"

#define VERSION "V101"

void readWifiConf() // 读取wifi配置
{
  // Read wifi conf from flash
  for (int i = 0; i < sizeof(wifiConf); i++)
  {
    ((char *)(&wifiConf))[i] = char(EEPROM.read(i));
  }
  // Make sure that there is a 0
  // that terminatnes the c string
  // if memory is not initalized yet.
  wifiConf.cstr_terminator = 0;
}

void writeWifiConf() // 保存wifi配置
{
  for (int i = 0; i < sizeof(wifiConf); i++)
  {
    EEPROM.write(i, ((char *)(&wifiConf))[i]);
  }
  EEPROM.commit();
}

void connect_wifi() // 联网
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiConf.wifi_ssid, wifiConf.wifi_password); // 用固定的账号密码连接网络
  uint8_t cnt = 0;                                        // 连接wifi的时间控制计数
  while (WiFi.status() != WL_CONNECTED)
  { // 未连接上的话
    for (uint8_t n = 0; n < 10; n++)
    { // 每500毫秒检测一次状态
      PowerOn_Loading(50);
    }
    cnt = cnt + 1;
    // Serial.print("cnt:");
    // Serial.println(cnt);
    if (cnt > wifi_connect_cnt)
    {
      Serial.print("\n超过重试次数");
      break;
    }
  }
  if (cnt > wifi_connect_cnt)
  {
    SmartConfigStatus = 0; // 设置mart config状态
    while (true)
    {
      bool success = smart_config();
      if (success == true)
        break;
    }
  }
  SmartConfigStatus = 4;
  wifiConnected = true; // 设置wifi连接状态
  while (loadNum < 194)
  { // 让动画走完
    PowerOn_Loading(1);
  }

  Serial.print("\nWiFi connected to: ");
  Serial.println(wifiConf.wifi_ssid);
  Serial.print("IP:   ");
  Serial.println(WiFi.localIP()); // 得到IP地址
}

void digitalClockDisplay() // 时间显示
{
  clk.setColorDepth(8);

  //--------------------中间时间区显示开始--------------------
  // 时分
  clk.createSprite(140, 48); // 创建Sprite，先在Sprite内存中画点，然后将内存中的点一次推向屏幕，这样刷新比较快
  clk.fillSprite(bgColor);   // 背景色
  // clk.loadFont(FxLED_48);
  clk.setTextDatum(CC_DATUM);              // 显示对齐方式
  clk.setTextColor(frontColor, bgColor);   // 文本的前景色和背景色
  clk.drawString(hourMinute(), 70, 24, 7); // 绘制时和分
  // clk.unloadFont();
  clk.pushSprite(28, 40); // Sprite中内容一次推向屏幕
  clk.deleteSprite();     // 删除Sprite

  // 秒
  clk.createSprite(40, 32);
  clk.fillSprite(bgColor);
  clk.loadFont(FxLED_32); // 加载字体
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(frontColor, bgColor);
  clk.drawString(num2str(second()), 20, 16);
  clk.unloadFont(); // 卸载字体
  clk.pushSprite(170, 60);
  clk.deleteSprite();
  //--------------------中间时间区显示结束--------------------

  //--------------------底部时间区显示开始--------------------
  clk.loadFont(ZdyLwFont_20); // 加载汉字字体

  // 星期
  clk.createSprite(58, 32);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(frontColor, bgColor);
  clk.drawString(week(), 29, 16); // 周几
  clk.pushSprite(1, 168);
  clk.deleteSprite();

  // 月日
  clk.createSprite(98, 32);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(frontColor, bgColor);
  clk.drawString(monthDay(), 49, 16); // 几月几日
  clk.pushSprite(61, 168);
  clk.deleteSprite();

  clk.unloadFont(); // 卸载字体
  //--------------------底部时间区显示结束--------------------
}

String week() // 星期
{
  String wk[7] = {"日", "一", "二", "三", "四", "五", "六"};
  String s = "周" + wk[weekday() - 1];
  return s;
}

String monthDay() // 月日
{
  String s = String(month());
  s = s + "月" + day() + "日";
  return s;
}

String hourMinute() // 时分
{
  String s = num2str(hour());
  s = s + ":" + num2str(minute());
  return s;
}

String num2str(int digits) // 数字转换成字符串，保持2位显示
{
  String s = "";
  if (digits < 10)
    s = s + "0";
  s = s + digits;
  return s;
}

void printDigits(int digits) // 打印时间数据
{
  Serial.print(":");
  if (digits < 10) // 打印两位数字
    Serial.print('0');
  Serial.print(digits);
}

time_t getNtpTime() // 获取NTP时间
{
  IPAddress ntpServerIP; // NTP服务器的IP地址

  while (Udp.parsePacket() > 0)
    ;                                          // 之前的数据没有处理的话一直等待 discard any previously received packets
  WiFi.hostByName(ntpServerName, ntpServerIP); // 从网站名获取IP地址

  sendNTPpacket(ntpServerIP); // 发送数据包
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = Udp.parsePacket(); // 接收数据
    if (size >= NTP_PACKET_SIZE)
    {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE); // 从缓冲区读取数据

      unsigned long secsSince1900;
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // 没获取到数据的话返回0
}

void sendNTPpacket(IPAddress &address) // 发送数据包到NTP服务器
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE); // 缓冲区清零

  packetBuffer[0] = 0b11100011; // LI, Version, Mode   填充缓冲区数据
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  Udp.beginPacket(address, 123);            // NTP服务器端口123
  Udp.write(packetBuffer, NTP_PACKET_SIZE); // 发送udp数据
  Udp.endPacket();                          // 发送结束
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) // 显示回调函数
{
  if (y >= tft.height())
    return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void getCityCode() // 发送HTTP请求并且将服务器响应通过串口输出
{
  String URL = "http://wgeo.weather.com.cn/ip/?_=" + String(now());

  httpClient.begin(wifiClient, URL);  // 配置请求地址。此处也可以不使用端口号和PATH而单纯的
  httpClient.setUserAgent("esp8266"); // 用户代理版本，其实没什么用 最重要是后端服务器支持
  // httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");//设置请求头中的User-Agent
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");

  int httpCode = httpClient.GET(); // 启动连接并发送HTTP请求
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);

  if (httpCode == HTTP_CODE_OK)
  { // 如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    String str = httpClient.getString();
    int aa = str.indexOf("id=");
    if (aa > -1)
    {                                               // 应答包里找到ID了
      cityCode = str.substring(aa + 4, aa + 4 + 9); // 9位长度
      Serial.println(cityCode);
      getCityWeater(); // 获取天气信息
      LastTime2 = millis();
    }
    else
    { // 没有找到ID
      Serial.println("获取城市代码失败");
    }
  }
  else
  {
    Serial.println("请求城市代码错误：");
    Serial.println(httpCode);
  }

  httpClient.end(); // 关闭与服务器连接
}

void getCityWeater() // 获取城市天气
{
  String URL = "http://d1.weather.com.cn/weather_index/" + cityCode + ".html?_=" + String(now());

  httpClient.begin(wifiClient, URL);  // 配置请求地址。
  httpClient.setUserAgent("esp8266"); // 用户代理版本，其实没什么用 最重要是后端服务器支持
  // httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");//设置请求头中的User-Agent
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");

  int httpCode = httpClient.GET(); // 启动连接并发送HTTP请求
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);

  if (httpCode == HTTP_CODE_OK)
  { // 如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    String str = httpClient.getString();
    int indexStart = str.indexOf("weatherinfo\":"); // 寻找起始和结束位置
    int indexEnd = str.indexOf("};var alarmDZ");

    String jsonCityDZ = str.substring(indexStart + 13, indexEnd); // 复制字符串
    Serial.println(jsonCityDZ);

    indexStart = str.indexOf("dataSK ="); // 寻找起始和结束位置
    indexEnd = str.indexOf(";var dataZS");
    String jsonDataSK = str.substring(indexStart + 8, indexEnd); // 复制字符串
    Serial.println(jsonDataSK);

    indexStart = str.indexOf("\"f\":["); // 寻找起始和结束位置
    indexEnd = str.indexOf(",{\"fa");
    String jsonFC = str.substring(indexStart + 5, indexEnd); // 复制字符串
    Serial.println(jsonFC);

    weaterData(&jsonCityDZ, &jsonDataSK, &jsonFC); // 显示天气信息
    Serial.println("获取成功");
  }
  else
  {
    Serial.println("请求城市天气错误：");
    Serial.print(httpCode);
  }

  httpClient.end(); // 关闭与服务器连接
}

void weaterData(String *cityDZ, String *dataSK, String *dataFC) // 天气信息写到屏幕上
{
  DynamicJsonDocument doc(512);
  deserializeJson(doc, *dataSK);
  JsonObject sk = doc.as<JsonObject>();

  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20); // 加载汉字字体

  // 温度显示
  clk.createSprite(54, 32);                              // 创建Sprite
  clk.fillSprite(bgColor);                               // 填充颜色
  clk.setTextDatum(CC_DATUM);                            // 显示对齐方式
  clk.setTextColor(frontColor, bgColor);                 // 文本的前景色和背景色
  clk.drawString(sk["temp"].as<String>() + "℃", 27, 16); // 显示文本
  clk.pushSprite(185, 168);                              // Sprite中内容一次推向屏幕
  clk.deleteSprite();                                    // 删除Sprite

  // 城市名称显示
  clk.createSprite(88, 32);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(frontColor, bgColor);
  clk.drawString(sk["cityname"].as<String>(), 44, 16);
  clk.pushSprite(151, 1);
  clk.deleteSprite();

  // PM2.5空气指数显示
  uint16_t pm25BgColor = tft.color565(156, 202, 127); // 优
  String aqiTxt = "优";
  int pm25V = sk["aqi"];
  if (pm25V > 200)
  {
    pm25BgColor = tft.color565(136, 11, 32); // 重度，显示颜色和空气质量程度
    aqiTxt = "重度";
  }
  else if (pm25V > 150)
  {
    pm25BgColor = tft.color565(186, 55, 121); // 中度
    aqiTxt = "中度";
  }
  else if (pm25V > 100)
  {
    pm25BgColor = tft.color565(242, 159, 57); // 轻
    aqiTxt = "轻度";
  }
  else if (pm25V > 50)
  {
    pm25BgColor = tft.color565(247, 219, 100); // 良
    aqiTxt = "良";
  }
  clk.createSprite(50, 24);
  clk.fillSprite(bgColor);
  clk.fillRoundRect(0, 0, 50, 24, 4, pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0xFFFF);
  clk.drawString(aqiTxt, 25, 13);
  clk.pushSprite(5, 130);
  clk.deleteSprite();

  // 湿度显示
  clk.createSprite(56, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(frontColor, bgColor);
  clk.drawString(sk["SD"].as<String>(), 28, 13);
  // clk.drawString("100%",28,13);
  clk.pushSprite(180, 130);
  clk.deleteSprite();

  scrollText[0] = "实时天气 " + sk["weather"].as<String>(); // 滚动显示的数据缓冲区
  scrollText[1] = "空气质量 " + aqiTxt;
  scrollText[2] = "风向 " + sk["WD"].as<String>() + sk["WS"].as<String>();

  // 左上角滚动字幕
  deserializeJson(doc, *cityDZ);
  JsonObject dz = doc.as<JsonObject>();
  // Serial.println(sk["ws"].as<String>());
  // String aa = "今日天气:" + dz["weather"].as<String>() + "，温度:最低" + dz["tempn"].as<String>() + "，最高" + dz["temp"].as<String>() + " 空气质量:" + aqiTxt + "，风向:" + dz["wd"].as<String>() + dz["ws"].as<String>();
  // Serial.println(aa);
  scrollText[3] = "今日" + dz["weather"].as<String>();

  deserializeJson(doc, *dataFC);
  JsonObject fc = doc.as<JsonObject>();
  scrollText[4] = "最低温度" + fc["fd"].as<String>() + "℃";
  scrollText[5] = "最高温度" + fc["fc"].as<String>() + "℃";

  clk.unloadFont(); // 卸载字体
}

void scrollBanner() // 天气滚动条显示
{
  now1 = millis();
  if (now1 - LastTime1 > 2500)
  {
    // 2.5秒切换一次显示内容
    if (scrollText[Dis_Count])
    { // 如果滚动显示缓冲区有数据
      clkb.setColorDepth(8);
      clkb.loadFont(ZdyLwFont_20); // 加载汉字字体
      // 循环，7循环数，是因为一次Dis_Scroll调用大概占用7ms，7次调用大概等于一个动画显示的延时50ms
      // 如果一个动画帧延时100，则要设置 100/7 约为14或15
      for (int a = 0; a < 7; pos--, a++)
      { // 24点，每次移动一个点，从下往上移
        if (pos > 0)
          Dis_Scroll(pos);
        else
        {
          pos = 24;
          break;
        }
      }
      if (pos > 0 && pos != 24) // 大于0，说明没有循环完，但是要退出显示动画图片，回来还要继续移动
      {
        // clkb.deleteSprite();                      //删除Sprite，这个我移动到Dis_Scroll函数里了
        clkb.unloadFont(); // 卸载字体
        return;
      }
      clkb.unloadFont(); // 卸载字体
      if (Dis_Count >= 5)
      {                // 总共显示五条信息
        Dis_Count = 0; // 回第一个
      }
      else
      {
        Dis_Count += 1; // 准备切换到下一个
      }
      // Serial.println(Dis_Count);
    }
    LastTime1 = now1;
  }
}

void Dis_Scroll(int pos)
{                             // 字体滚动
  clkb.createSprite(148, 24); // 创建Sprite，先在Sprite内存中画点，然后将内存中的点一次推向屏幕，这样刷新比较快
  clkb.fillSprite(bgColor);   // 背景色
  clkb.setTextWrap(false);
  clkb.setTextDatum(CC_DATUM);                          // 显示对齐方式
  clkb.setTextColor(frontColor, bgColor);               // 文本的前景色和背景色
  clkb.drawString(scrollText[Dis_Count], 74, pos + 12); // 打显示内容
  clkb.pushSprite(2, 4);                                // Sprite中内容一次推向屏幕
  clkb.deleteSprite();                                  // 删除Sprite
}

void imgAnim()
{
  int x = 80, y = 94, dt = 30; // 瘦子版dt=10毫秒 胖子30较为合适

  TJpgDec.drawJpg(x, y, i0, sizeof(i0)); // 打一张图片延时一段时间，达到动画效果
  delay(dt);
  TJpgDec.drawJpg(x, y, i1, sizeof(i1));
  delay(dt);
  TJpgDec.drawJpg(x, y, i2, sizeof(i2));
  delay(dt);
  TJpgDec.drawJpg(x, y, i3, sizeof(i3));
  delay(dt);
  TJpgDec.drawJpg(x, y, i4, sizeof(i4));
  delay(dt);
  TJpgDec.drawJpg(x, y, i5, sizeof(i5));
  delay(dt);
  TJpgDec.drawJpg(x, y, i6, sizeof(i6));
  delay(dt);
  TJpgDec.drawJpg(x, y, i7, sizeof(i7));
  delay(dt);
  TJpgDec.drawJpg(x, y, i8, sizeof(i8));
  delay(dt);
  TJpgDec.drawJpg(x, y, i9, sizeof(i9));
  delay(dt);
}
unsigned long oldTime = 0, imgNum = 1;
void imgDisplay()
{
  int x = 75, y = 94, dt;
  switch (Gif_Mode)
  { // 修改动画的播放速度
  case 1:
    dt = 100;
    break;
  case 2:
    dt = 50;
    break;
  case 3:
    dt = 100;
    break;
  case 4:
    dt = 100;
    break;
  case 5:
    dt = 50;
    break;
  }
  if (millis() - oldTime >= dt)
  {
    imgNum = imgNum + 1;
    oldTime = millis();
  }
  else
    return;
  if (Gif_Mode == 5)
  {
    switch (imgNum)
    {
    case 1:
      TJpgDec.drawJpg(x, y, my_1, sizeof(my_1));
      break;
    case 2:
      TJpgDec.drawJpg(x, y, my_2, sizeof(my_2));
      break;
    case 3:
      TJpgDec.drawJpg(x, y, my_3, sizeof(my_3));
      break;
    case 4:
      TJpgDec.drawJpg(x, y, my_4, sizeof(my_4));
      break;
    case 5:
      TJpgDec.drawJpg(x, y, my_5, sizeof(my_5));
      break;
    case 6:
      TJpgDec.drawJpg(x, y, my_6, sizeof(my_6));
      break;
    case 7:
      TJpgDec.drawJpg(x, y, my_7, sizeof(my_7));
      break;
    case 8:
      TJpgDec.drawJpg(x, y, my_8, sizeof(my_8));
      break;
    case 9:
      TJpgDec.drawJpg(x, y, my_9, sizeof(my_9));
      break;
    case 10:
      TJpgDec.drawJpg(x, y, my_10, sizeof(my_10));
      break;
    case 11:
      TJpgDec.drawJpg(x, y, my_11, sizeof(my_11));
      break;
    case 12:
      TJpgDec.drawJpg(x, y, my_12, sizeof(my_12));
      imgNum = 1;
      // Gif_Mode++;
      break;
    }
  }
  else if (Gif_Mode == 3)
  { // 动画-龙猫转圈
    switch (imgNum)
    {
    case 1:
      TJpgDec.drawJpg(x, y, img_0, sizeof(img_0));
      break;
    case 2:
      TJpgDec.drawJpg(x, y, img_1, sizeof(img_1));
      break;
    case 3:
      TJpgDec.drawJpg(x, y, img_2, sizeof(img_2));
      break;
    case 4:
      TJpgDec.drawJpg(x, y, img_3, sizeof(img_3));
      break;
    case 5:
      TJpgDec.drawJpg(x, y, img_4, sizeof(img_4));
      break;
    case 6:
      TJpgDec.drawJpg(x, y, img_5, sizeof(img_5));
      break;
    case 7:
      TJpgDec.drawJpg(x, y, img_6, sizeof(img_6));
      break;
    case 8:
      TJpgDec.drawJpg(x, y, img_7, sizeof(img_7));
      break;
    case 9:
      TJpgDec.drawJpg(x, y, img_8, sizeof(img_8));
      break;
    case 10:
      TJpgDec.drawJpg(x, y, img_9, sizeof(img_9));
      break;
    case 11:
      TJpgDec.drawJpg(x, y, img_10, sizeof(img_10));
      break;
    case 12:
      TJpgDec.drawJpg(x, y, img_11, sizeof(img_11));
      break;
    case 13:
      TJpgDec.drawJpg(x, y, img_12, sizeof(img_12));
      break;
    case 14:
      TJpgDec.drawJpg(x, y, img_13, sizeof(img_13));
      break;
    case 15:
      TJpgDec.drawJpg(x, y, img_14, sizeof(img_14));
      break;
    case 16:
      TJpgDec.drawJpg(x, y, img_15, sizeof(img_15));
      break;
    case 17:
      TJpgDec.drawJpg(x, y, img_16, sizeof(img_16));
      break;
    case 18:
      TJpgDec.drawJpg(x, y, img_17, sizeof(img_17));
      break;
    case 19:
      TJpgDec.drawJpg(x, y, img_18, sizeof(img_18));
      break;
    case 20:
      TJpgDec.drawJpg(x, y, img_19, sizeof(img_19));
      break;
    case 21:
      TJpgDec.drawJpg(x, y, img_20, sizeof(img_20));
      break;
    case 22:
      TJpgDec.drawJpg(x, y, img_21, sizeof(img_21));
      break;
    case 23:
      TJpgDec.drawJpg(x, y, img_22, sizeof(img_22));
      break;
    case 24:
      TJpgDec.drawJpg(x, y, img_23, sizeof(img_23));
      break;
    case 25:
      TJpgDec.drawJpg(x, y, img_24, sizeof(img_24));
      break;
    case 26:
      TJpgDec.drawJpg(x, y, img_25, sizeof(img_25));
      break;
    case 27:
      TJpgDec.drawJpg(x, y, img_26, sizeof(img_26));
      break;
    case 28:
      TJpgDec.drawJpg(x, y, img_27, sizeof(img_27));
      break;
    case 29:
      TJpgDec.drawJpg(x, y, img_28, sizeof(img_28));
      break;
    case 30:
      TJpgDec.drawJpg(x, y, img_29, sizeof(img_29));
      break;
    case 31:
      TJpgDec.drawJpg(x, y, img_30, sizeof(img_30));
      break;
    case 32:
      TJpgDec.drawJpg(x, y, img_31, sizeof(img_31));
      break;
    case 33:
      TJpgDec.drawJpg(x, y, img_32, sizeof(img_32));
      break;
    case 34:
      TJpgDec.drawJpg(x, y, img_33, sizeof(img_33));
      break;
    case 35:
      TJpgDec.drawJpg(x, y, img_34, sizeof(img_34));
      break;
    case 36:
      TJpgDec.drawJpg(x, y, img_35, sizeof(img_35));
      break;
    case 37:
      TJpgDec.drawJpg(x, y, img_36, sizeof(img_36));
      break;
    case 38:
      TJpgDec.drawJpg(x, y, img_37, sizeof(img_37));
      break;
    case 39:
      TJpgDec.drawJpg(x, y, img_38, sizeof(img_38));
      break;
    case 40:
      TJpgDec.drawJpg(x, y, img_39, sizeof(img_39));
      break;
    case 41:
      TJpgDec.drawJpg(x, y, img_40, sizeof(img_40));
      break;
    case 42:
      TJpgDec.drawJpg(x, y, img_41, sizeof(img_41));
      break;
    case 43:
      TJpgDec.drawJpg(x, y, img_42, sizeof(img_42));
      break;
    case 44:
      TJpgDec.drawJpg(x, y, img_43, sizeof(img_43));
      break;
    case 45:
      TJpgDec.drawJpg(x, y, img_44, sizeof(img_44));
      break;
    case 46:
      TJpgDec.drawJpg(x, y, img_45, sizeof(img_45));
      break;
    case 47:
      TJpgDec.drawJpg(x, y, img_46, sizeof(img_46));
      break;
    case 48:
      TJpgDec.drawJpg(x, y, img_47, sizeof(img_47));
      break;
    case 49:
      TJpgDec.drawJpg(x, y, img_48, sizeof(img_48));
      break;
    case 50:
      TJpgDec.drawJpg(x, y, img_49, sizeof(img_49));
      break;
    case 51:
      TJpgDec.drawJpg(x, y, img_50, sizeof(img_50));
      break;
    case 52:
      TJpgDec.drawJpg(x, y, img_51, sizeof(img_51));
      break;
    case 53:
      TJpgDec.drawJpg(x, y, img_52, sizeof(img_52));
      break;
    case 54:
      TJpgDec.drawJpg(x, y, img_53, sizeof(img_53));
      break;
    case 55:
      TJpgDec.drawJpg(x, y, img_54, sizeof(img_54));
      break;
    case 56:
      TJpgDec.drawJpg(x, y, img_55, sizeof(img_55));
      break;
    case 57:
      TJpgDec.drawJpg(x, y, img_56, sizeof(img_56));
      break;
    case 58:
      TJpgDec.drawJpg(x, y, img_57, sizeof(img_57));
      break;
    case 59:
      TJpgDec.drawJpg(x, y, img_58, sizeof(img_58));
      break;
    case 60:
      TJpgDec.drawJpg(x, y, img_59, sizeof(img_59));
      break;
    case 61:
      TJpgDec.drawJpg(x, y, img_60, sizeof(img_60));
      break;
    case 62:
      TJpgDec.drawJpg(x, y, img_61, sizeof(img_61));
      break;
    case 63:
      TJpgDec.drawJpg(x, y, img_62, sizeof(img_62));
      break;
    case 64:
      TJpgDec.drawJpg(x, y, img_63, sizeof(img_63));
      break;
    case 65:
      TJpgDec.drawJpg(x, y, img_64, sizeof(img_64));
      break;
    case 66:
      TJpgDec.drawJpg(x, y, img_65, sizeof(img_65));
      break;
    case 67:
      TJpgDec.drawJpg(x, y, img_66, sizeof(img_66));
      break;
    case 68:
      TJpgDec.drawJpg(x, y, img_67, sizeof(img_67));
      break;
    case 69:
      TJpgDec.drawJpg(x, y, img_68, sizeof(img_68));
      break;
    case 70:
      TJpgDec.drawJpg(x, y, img_69, sizeof(img_69));
      break;
    case 71:
      TJpgDec.drawJpg(x, y, img_70, sizeof(img_70));
      break;
    case 72:
      TJpgDec.drawJpg(x, y, img_71, sizeof(img_71));
      break;
    case 73:
      TJpgDec.drawJpg(x, y, img_72, sizeof(img_72));
      break;
    case 74:
      TJpgDec.drawJpg(x, y, img_73, sizeof(img_73));
      break;
    case 75:
      TJpgDec.drawJpg(x, y, img_74, sizeof(img_74));
      break;
    case 76:
      TJpgDec.drawJpg(x, y, img_75, sizeof(img_75));
      break;
    case 77:
      TJpgDec.drawJpg(x, y, img_76, sizeof(img_76));
      break;
    case 78:
      TJpgDec.drawJpg(x, y, img_77, sizeof(img_77));
      break;
    case 79:
      TJpgDec.drawJpg(x, y, img_78, sizeof(img_78));
      break;
    case 80:
      TJpgDec.drawJpg(x, y, img_79, sizeof(img_79));
      imgNum = 1;
      // Gif_Mode++;
      break;
    }
  }

  else if (Gif_Mode == 1)
  { // 动画-打乒乓
    switch (imgNum)
    {
    case 1:
      TJpgDec.drawJpg(x, y, pingpang_0, sizeof(pingpang_0));
      break;
    case 2:
      TJpgDec.drawJpg(x, y, pingpang_1, sizeof(pingpang_1));
      break;
    case 3:
      TJpgDec.drawJpg(x, y, pingpang_2, sizeof(pingpang_2));
      break;
    case 4:
      TJpgDec.drawJpg(x, y, pingpang_3, sizeof(pingpang_3));
      break;
    case 5:
      TJpgDec.drawJpg(x, y, pingpang_4, sizeof(pingpang_4));
      break;
    case 6:
      TJpgDec.drawJpg(x, y, pingpang_5, sizeof(pingpang_5));
      break;
    case 7:
      TJpgDec.drawJpg(x, y, pingpang_6, sizeof(pingpang_6));
      break;
    case 8:
      TJpgDec.drawJpg(x, y, pingpang_7, sizeof(pingpang_7));
      break;
    case 9:
      TJpgDec.drawJpg(x, y, pingpang_8, sizeof(pingpang_8));
      break;
    case 10:
      TJpgDec.drawJpg(x, y, pingpang_9, sizeof(pingpang_9));
      break;
    case 11:
      TJpgDec.drawJpg(x, y, pingpang_10, sizeof(pingpang_10));
      break;
    case 12:
      TJpgDec.drawJpg(x, y, pingpang_12, sizeof(pingpang_12));
      break;
    case 13:
      TJpgDec.drawJpg(x, y, pingpang_13, sizeof(pingpang_13));
      break;
    case 14:
      TJpgDec.drawJpg(x, y, pingpang_14, sizeof(pingpang_14));
      break;
    case 15:
      TJpgDec.drawJpg(x, y, pingpang_15, sizeof(pingpang_15));
      break;
    case 16:
      TJpgDec.drawJpg(x, y, pingpang_16, sizeof(pingpang_16));
      break;
    case 17:
      TJpgDec.drawJpg(x, y, pingpang_17, sizeof(pingpang_17));
      break;
    case 18:
      TJpgDec.drawJpg(x, y, pingpang_18, sizeof(pingpang_18));
      break;
    case 19:
      TJpgDec.drawJpg(x, y, pingpang_19, sizeof(pingpang_19));
      break;
    case 20:
      TJpgDec.drawJpg(x, y, pingpang_20, sizeof(pingpang_20));
      break;
    case 21:
      TJpgDec.drawJpg(x, y, pingpang_21, sizeof(pingpang_21));
      break;
    case 22:
      TJpgDec.drawJpg(x, y, pingpang_22, sizeof(pingpang_22));
      break;
    case 23:
      TJpgDec.drawJpg(x, y, pingpang_23, sizeof(pingpang_23));
      break;
    case 24:
      TJpgDec.drawJpg(x, y, pingpang_24, sizeof(pingpang_24));
      break;
    case 25:
      TJpgDec.drawJpg(x, y, pingpang_25, sizeof(pingpang_25));
      break;
    case 26:
      TJpgDec.drawJpg(x, y, pingpang_26, sizeof(pingpang_26));
      break;
    case 27:
      TJpgDec.drawJpg(x, y, pingpang_27, sizeof(pingpang_27));
      imgNum = 1;
      // Gif_Mode++;
      break;
    }
  }
  else if (Gif_Mode == 4)
  { // 动画-太空人
    switch (imgNum)
    {
    case 1:
      TJpgDec.drawJpg(x, y, i0, sizeof(i0));
      break; // 打一张图片延时一段时间，达到动画效果

    case 2:
      TJpgDec.drawJpg(x, y, i1, sizeof(i1));
      break;

    case 3:
      TJpgDec.drawJpg(x, y, i2, sizeof(i2));
      break;

    case 4:
      TJpgDec.drawJpg(x, y, i3, sizeof(i3));
      break;

    case 5:
      TJpgDec.drawJpg(x, y, i4, sizeof(i4));
      break;

    case 6:
      TJpgDec.drawJpg(x, y, i5, sizeof(i5));
      break;

    case 7:
      TJpgDec.drawJpg(x, y, i6, sizeof(i6));
      break;

    case 8:
      TJpgDec.drawJpg(x, y, i7, sizeof(i7));
      break;

    case 9:
      TJpgDec.drawJpg(x, y, i8, sizeof(i8));
      break;

    case 10:
      TJpgDec.drawJpg(x, y, i9, sizeof(i9));
      imgNum = 1;
      // Gif_Mode = 1;
      break;
    }
  }
  else if (Gif_Mode == 2)
  { // 动画-龙猫跳绳
    switch (imgNum)
    {
    case 1:
      TJpgDec.drawJpg(x, 84, quan_0, sizeof(quan_0));
      break;
    case 2:
      TJpgDec.drawJpg(x, 84, quan_1, sizeof(quan_1));
      break;
    case 3:
      TJpgDec.drawJpg(x, 84, quan_2, sizeof(quan_2));
      break;
    case 4:
      TJpgDec.drawJpg(x, 84, quan_3, sizeof(quan_3));
      break;
    case 5:
      TJpgDec.drawJpg(x, 84, quan_4, sizeof(quan_4));
      break;
    case 6:
      TJpgDec.drawJpg(x, 84, quan_5, sizeof(quan_5));
      break;
    case 7:
      TJpgDec.drawJpg(x, 84, quan_6, sizeof(quan_6));
      break;
    case 8:
      TJpgDec.drawJpg(x, 84, quan_7, sizeof(quan_7));
      break;
    case 9:
      TJpgDec.drawJpg(x, 84, quan_8, sizeof(quan_8));
      break;
    case 10:
      TJpgDec.drawJpg(x, 84, quan_9, sizeof(quan_9));
      break;
    case 11:
      TJpgDec.drawJpg(x, 84, quan_10, sizeof(quan_10));
      break;
    case 12:
      TJpgDec.drawJpg(x, 84, quan_11, sizeof(quan_11));
      break;
    case 13:
      TJpgDec.drawJpg(x, 84, quan_12, sizeof(quan_12));
      break;
    case 14:
      TJpgDec.drawJpg(x, 84, quan_13, sizeof(quan_13));
      break;
    case 15:
      TJpgDec.drawJpg(x, 84, quan_14, sizeof(quan_14));
      break;
    case 16:
      TJpgDec.drawJpg(x, 84, quan_15, sizeof(quan_15));
      break;
    case 17:
      TJpgDec.drawJpg(x, 84, quan_16, sizeof(quan_16));
      break;
    case 18:
      TJpgDec.drawJpg(x, 84, quan_17, sizeof(quan_17));
      break;
    case 19:
      TJpgDec.drawJpg(x, 84, quan_18, sizeof(quan_18));
      break;
    case 20:
      TJpgDec.drawJpg(x, 84, quan_19, sizeof(quan_19));
      break;
    case 21:
      TJpgDec.drawJpg(x, 84, quan_20, sizeof(quan_20));
      break;
    case 22:
      TJpgDec.drawJpg(x, 84, quan_21, sizeof(quan_21));
      break;
    case 23:
      TJpgDec.drawJpg(x, 84, quan_22, sizeof(quan_22));
      break;
    case 24:
      TJpgDec.drawJpg(x, 84, quan_23, sizeof(quan_23));
      break;
    case 25:
      TJpgDec.drawJpg(x, 84, quan_24, sizeof(quan_24));
      break;
    case 26:
      TJpgDec.drawJpg(x, 84, quan_25, sizeof(quan_25));
      break;
    case 27:
      TJpgDec.drawJpg(x, 84, quan_26, sizeof(quan_26));
      break;
    case 28:
      TJpgDec.drawJpg(x, 84, quan_27, sizeof(quan_27));
      break;
    case 29:
      TJpgDec.drawJpg(x, 84, quan_28, sizeof(quan_28));
      break;
    case 30:
      TJpgDec.drawJpg(x, 84, quan_29, sizeof(quan_29));
      break;
    case 31:
      TJpgDec.drawJpg(x, 84, quan_30, sizeof(quan_30));
      break;
    case 32:
      TJpgDec.drawJpg(x, 84, quan_31, sizeof(quan_31));
      break;
    case 33:
      TJpgDec.drawJpg(x, 84, quan_32, sizeof(quan_32));
      break;
    case 34:
      TJpgDec.drawJpg(x, 84, quan_33, sizeof(quan_33));
      break;
    case 35:
      TJpgDec.drawJpg(x, 84, quan_34, sizeof(quan_34));
      break;
    case 36:
      TJpgDec.drawJpg(x, 84, quan_35, sizeof(quan_35));
      break;
    case 37:
      TJpgDec.drawJpg(x, 84, quan_36, sizeof(quan_36));
      break;
    case 38:
      TJpgDec.drawJpg(x, 84, quan_37, sizeof(quan_37));
      break;
    case 39:
      TJpgDec.drawJpg(x, 84, quan_38, sizeof(quan_38));
      break;
    case 40:
      TJpgDec.drawJpg(x, 84, quan_39, sizeof(quan_39));
      imgNum = 1;
      // Gif_Mode++;
      break;
    }
  }
}

void PowerOn_Loading(uint8_t delayTime) // 开机联网显示的进度条，输入延时时间
{
  clk.setColorDepth(8);
  clk.createSprite(200, 50); // 创建Sprite
  clk.fillSprite(0x0000);    // 填充颜色

  clk.drawRoundRect(0, 0, 200, 16, 8, 0xFFFF);     // 画一个圆角矩形
  clk.fillRoundRect(3, 3, loadNum, 10, 5, 0xFFFF); // 画一个填充的圆角矩形
  clk.setTextDatum(CC_DATUM);                      // 显示对齐方式
  clk.setTextColor(TFT_GREEN, 0x0000);             // 文本的前景色和背景色
  if (SmartConfigStatus == 1)
    clk.drawString("Waiting for Config", 100, 40, 2);
  else if (SmartConfigStatus == 2)
    clk.drawString("Connecting to WiFi", 100, 40, 2);
  else if (SmartConfigStatus == 4)
    clk.drawString("WiFi Connected.", 100, 40, 2);
  else
    clk.drawString("Connecting to WiFi", 100, 40, 2);                    // 显示文本
  clk.pushSprite(20, 110);                                               // Sprite中内容一次推向屏幕
  clk.deleteSprite();                                                    // 删除Sprite
  if (wifiConnected == false && loadNum > 160 && SmartConfigStatus == 0) // wifi没有连接时，进度条不再增长
  {
  }
  else if (SmartConfigStatus != 0 && SmartConfigStatus != 4 && loadNum > 180) // 在smart config 状态时，进度条反复回退
  {
    loadNum = 161;
  }
  else
  {
    loadNum += 1;
  }
  // 进度条位置变化，直到加载完成
  if (loadNum >= 194)
  {
    loadNum = 194;
  }
  delay(delayTime);
}

// 自动配网
bool smart_config()
{
  uint8 cnt = 1;
  WiFi.mode(WIFI_STA);     // 这里一定要将WIFI设置为客户端模式才能进行配网
  WiFi.beginSmartConfig(); // 将esp8266设置为智能配网模式
  Serial.println("Waiting for SmartConfig.");
  SmartConfigStatus = 1;
  while (!WiFi.smartConfigDone())
  {
    // delay(500);
    for (uint8_t n = 0; n < 10; n++)
    { // 每500毫秒检测一次状态
      PowerOn_Loading(50);
    }
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("SmartConfig received.");
  SmartConfigStatus = 2;

  // Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    // delay(500);
    for (uint8_t n = 0; n < 10; n++)
    { // 每500毫秒检测一次状态
      PowerOn_Loading(50);
    }
    cnt = cnt + 1;
    if (cnt > wifi_connect_cnt / 2)
    {
      SmartConfigStatus = 3;
      WiFi.stopSmartConfig(); // 停止smartconfig，为下一轮配置准备
      return false;
    }
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");
  SmartConfigStatus = 4;
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // 串口输出现在的IP地址
  // WiFi.mode(WIFI_AP_STA);         // 这里将模式设置回AP和STA双模式，不设置亲测也是可以的，但是不能只设置为AP模式，要不然联网后没办法连上互联网。
  delay(5);
  strcpy(wifiConf.wifi_ssid, WiFi.SSID().c_str());
  strcpy(wifiConf.wifi_password, WiFi.psk().c_str());
  writeWifiConf();
  return true;
}
// 切换背景色，切换字体颜色
void change_color()
{
  if (Gif_Mode == 5)
  {
    frontColor = TFT_YELLOW; // 背景颜色
    bgColor = TFT_BLACK;     // 前景颜色
  }
  else
  {
    frontColor = TFT_BLACK; // 背景颜色
    bgColor = 0xFFFF;       // 前景颜色
  }

  tft.fillScreen(0x0000);                // 清屏
  tft.setTextColor(frontColor, bgColor); // 设置字体颜色

  TJpgDec.drawJpg(0, 0, watchtop, sizeof(watchtop));         // 显示顶部图标 240*20
  TJpgDec.drawJpg(0, 220, watchbottom, sizeof(watchbottom)); // 显示底部图标 240*20

  // 绘制一个窗口
  tft.setViewport(0, 20, 240, 200);              // 中间的显示区域大小
  tft.fillScreen(0x0000);                        // 清屏
  tft.fillRoundRect(0, 0, 240, 200, 5, bgColor); // 实心圆角矩形
  // tft.resetViewport();

  // 绘制线框
  tft.drawFastHLine(0, 34, 240, frontColor); // 这些坐标都是窗体内部坐标
  tft.drawFastVLine(150, 0, 34, frontColor);
  tft.drawFastHLine(0, 166, 240, frontColor);
  tft.drawFastVLine(60, 166, 34, frontColor);
  tft.drawFastVLine(160, 166, 34, frontColor);

  getCityCode(); // 通过IP地址获取城市代码

  TJpgDec.drawJpg(161, 171, temperature, sizeof(temperature)); // 温度图标
  TJpgDec.drawJpg(159, 130, humidity, sizeof(humidity));       // 湿度图标

  digitalClockDisplay();
}
// mqtt 收到订阅消息后的回调函数
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  Serial.printf("%s\n", (char *)payload);
  if (0 == strcmp(topic, MQTT_TOPIC_PIC))
  {
    // 直接返回一个数值，用来控制显示的图片
    if (length == 1 && ((int)payload[0] - 48) > 0 && ((int)payload[0] - 48) < 6)
    {
      int old_mode = Gif_Mode;
      Gif_Mode = (int)payload[0] - 48;
      imgNum = 1;
      if ((old_mode == 5 && Gif_Mode < 5) || (old_mode < 5 && Gif_Mode == 5))
      {
        // 切换背景色，切换字体颜色
        change_color();
        // 保存 Gif_Mode 到eeprom
        wifiConf.gif_mode = Gif_Mode;
        writeWifiConf();
      }
    }
    Serial.printf("length:%d,Gif_Mode:%d\n", length, Gif_Mode);
  }
  else if (0 == strcmp(topic, MQTT_TOPIC_LED))
  {
    String message;
    for (int i = 0; i < length; i++)
    {
      message = message + (char)payload[i]; // convert *byte to string
    }
    Serial.print(message);
    if (message == "on")
    {
      digitalWrite(LED, LOW);
    } // LED on
    if (message == "off")
    {
      digitalWrite(LED, HIGH);
    } // LED off
    Serial.println();
    Serial.println("-----------------------");
  }
  else if (0 == strcmp(topic, MQTT_TOPIC_LED))
  {
  }
}

void setUpOverTheAirProgramming() // OAT升级
{

  // Change OTA port.
  // Default: 8266
  // ArduinoOTA.setPort(8266);

  // Change the name of how it is going to
  // show up in Arduino IDE.
  // Default: esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // Re-programming passowrd.
  // No password by default.
  // ArduinoOTA.setPassword("123");

  ArduinoOTA.begin();
}

/* 3. 处理访问网站根目录“/”的访问请求 */
void handleRoot()
{
  String htmlCode = "<!DOCTYPE html>\n";
  htmlCode += " <html>\n";
  htmlCode += "   <head>\n";
  htmlCode += "     <meta charset=\"UTF-8\"/>\n";
  htmlCode += "     <title>ESP8266控制</title>\n";
  htmlCode += "   </head>\n";
  htmlCode += "   <body>\n<div style=\"width:600px;margin:0 auto;\">\n";
  htmlCode += "     <h2 align=\"center\">esp8266显示屏参数控制</h2>";
  htmlCode += "     <p>\n<form action=\"/gifmode\" method=\"POST\">\n";
  htmlCode += "       <a>设置动图样式：</a>\n";
  htmlCode += "     	<select name=\"gifmode\">\n";
  if (Gif_Mode == 1)
    htmlCode += "   		<option value=\"1\" selected>乒乓球</option>\n";
  else
    htmlCode += "   		<option value=\"1\">乒乓球</option>\n";
  if (Gif_Mode == 2)
    htmlCode += "   		<option value=\"2\" selected>跳绳的龙猫</option>\n";
  else
    htmlCode += "   		<option value=\"2\">跳绳的龙猫</option>\n";
  if (Gif_Mode == 3)
    htmlCode += "   		<option value=\"3\" selected>跳舞的龙猫</option>\n";
  else
    htmlCode += "   		<option value=\"3\">跳舞的龙猫</option>\n";
  if (Gif_Mode == 4)
    htmlCode += "   		<option value=\"4\" selected>太空人</option>\n";
  else
    htmlCode += "   		<option value=\"4\">太空人</option>\n";
  if (Gif_Mode == 5)
    htmlCode += "   		<option value=\"5\" selected>跑步的老头</option>\n";
  else
    htmlCode += "   		<option value=\"5\">跑步的老头</option>\n";
  htmlCode += "     	</select>\n";
  htmlCode += "     	<input type=\"submit\" value=\"提交\" />\n";
  htmlCode += "     </form>\n</p>\n";
  htmlCode += "     <h2 align=\"center\">重启设备</h2>";
  htmlCode += "     <p>\n<form action=\"/restart\" method=\"POST\">\n";
  htmlCode += "       <a>点击按钮重启设备：</a>\n";
  htmlCode += "     	<input type=\"hidden\" name=\"restart\" value=\"yes\" \/>\n";
  htmlCode += "     	<input type=\"submit\" value=\"确认重启\" />\n";
  htmlCode += "     </form>\n</p>\n";
  htmlCode += "     </div>\n";
  htmlCode += "   </body>\n";
  htmlCode += "</html>\n";
  esp8266_server.send(200, "text/html", htmlCode); // NodeMCU将调用此函数。
}

/* 4. 设置处理404情况的函数'handleNotFound' */
void handleNotFound()
{                                                           // 当浏览器请求的网络资源无法在服务器找到时，
  esp8266_server.send(404, "text/plain", "404: Not found"); // NodeMCU将调用此函数。
}

/*设置 图片样式*/
void handle_Gif_Mode()
{
  if (esp8266_server.hasArg("gifmode"))
  {
    int value = 0;
    // esp8266_server.arg("gifmode").toCharArray(value, 1);
    value = (int)esp8266_server.arg("gifmode").toInt();
    Serial.printf("http server提交的gifmode 参数为%d\n", value);

    // 直接返回一个数值，用来控制显示的图片
    if (value > 0 && value < 6)
    {
      int old_mode = Gif_Mode;
      Gif_Mode = value;
      imgNum = 1;
      if ((old_mode == 5 && Gif_Mode < 5) || (old_mode < 5 && Gif_Mode == 5))
      {
        // 切换背景色，切换字体颜色
        change_color();
        // 保存 Gif_Mode 到eeprom
        wifiConf.gif_mode = Gif_Mode;
        writeWifiConf();
      }
    }
  }
  esp8266_server.sendHeader("Location", "/", true); // Redirect to our html web page
  esp8266_server.send(302, "text/plane", "");
}

void handle_restart()
{
  esp8266_server.sendHeader("Location", "/", true); // Redirect to our html web page
  esp8266_server.send(302, "text/plane", "");
  if (esp8266_server.hasArg("restart"))
  {
    int restart = 2;
    restart = esp8266_server.arg("restart").compareTo("yes");
    if (restart == 0)
    {
      delay(1000); // 等待1s让浏览器得到返回结果
      ESP.restart();
    }
  }
}

void setup()
{
  Serial.begin(115200); // 初始化串口
  Serial.println();     // 打印回车换行

  tft.init();                            // TFT初始化
  tft.setRotation(2);                    // 旋转角度0-3
  tft.fillScreen(0x0000);                // 清屏
  tft.setTextColor(frontColor, bgColor); // 设置字体颜色

  EEPROM.begin(512);
  readWifiConf();
  connect_wifi(); // 联网处理

  Gif_Mode = wifiConf.gif_mode;
  if (Gif_Mode == 5)
  {
    frontColor = TFT_YELLOW; // 背景颜色
    bgColor = TFT_BLACK;     // 前景颜色
  }
  else
  {
    frontColor = TFT_BLACK; // 背景颜色
    bgColor = 0xFFFF;       // 前景颜色
  }

  // 连接mqtt服务器
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqtt_callback);
  while (!mqtt_client.connected())
  {
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(mqtt_client.state());
      delay(2000);
    }
  }
  mqtt_client.subscribe(topic);

  setUpOverTheAirProgramming(); // 开启OTA升级服务

  Serial.println("Starting UDP"); // 连接时间服务器
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  TJpgDec.setJpgScale(1);          // 设置放大倍数
  TJpgDec.setSwapBytes(true);      // 交换字节
  TJpgDec.setCallback(tft_output); // 回调函数

  TJpgDec.drawJpg(0, 0, watchtop, sizeof(watchtop));         // 显示顶部图标 240*20
  TJpgDec.drawJpg(0, 220, watchbottom, sizeof(watchbottom)); // 显示底部图标 240*20

  // 绘制一个窗口
  tft.setViewport(0, 20, 240, 200);              // 中间的显示区域大小
  tft.fillScreen(0x0000);                        // 清屏
  tft.fillRoundRect(0, 0, 240, 200, 5, bgColor); // 实心圆角矩形
  // tft.resetViewport();

  // 绘制线框
  tft.drawFastHLine(0, 34, 240, frontColor); // 这些坐标都是窗体内部坐标
  tft.drawFastVLine(150, 0, 34, frontColor);
  tft.drawFastHLine(0, 166, 240, frontColor);
  tft.drawFastVLine(60, 166, 34, frontColor);
  tft.drawFastVLine(160, 166, 34, frontColor);

  getCityCode(); // 通过IP地址获取城市代码

  TJpgDec.drawJpg(161, 171, temperature, sizeof(temperature)); // 温度图标
  TJpgDec.drawJpg(159, 130, humidity, sizeof(humidity));       // 湿度图标

  httpUpdater.setup(&esp8266_server);
  /* 3. 开启http网络服务器功能 */
  esp8266_server.begin();                    // 启动http网络服务器
  esp8266_server.on("/", handleRoot);        // 设置请求根目录时的处理函数函数
  esp8266_server.onNotFound(handleNotFound); // 设置无法响应时的处理函数

  esp8266_server.on("/gifmode", handle_Gif_Mode); // 设置请求开灯目录时的处理函数函数
  esp8266_server.on("/restart", handle_restart);  // 设置请求开灯目录时的处理函数函数
}

void loop()
{
  if (timeStatus() != timeNotSet)
  { // 已经获取到数据的话
    if (now() != prevDisplay)
    { // 如果本次数据和上次不一样的话，刷新
      prevDisplay = now();
      digitalClockDisplay();
    }
  }

  if (millis() - LastTime2 > 600000)
  { // 10分钟更新一次天气
    LastTime2 = millis();
    getCityWeater();
  }
  scrollBanner(); // 天气数据滚动显示 //该函数执行时，会使imgDisplay的动画卡顿一下
  imgDisplay();   // 龙猫动画

  ArduinoOTA.handle(); // OTA升级

  if (millis() - LastTime4 > 2000) // 2秒钟更新温度
  {
    // ds18b20_getTemperature(); // 读取温度
    LastTime4 = millis();
  }

  // mqtt
  mqtt_client.loop();
  // http server
  esp8266_server.handleClient();
}
