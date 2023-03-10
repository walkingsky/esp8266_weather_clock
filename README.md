# esp8266_weather_clock
esp8266 的一个增改他人的 天气时钟 固件程序



本程序的硬件及软件均基于**ESP8266太空人天气时钟**，硬件及软件开源地址：https://oshwhub.com/nanxiangxiao/tai-kong-ren-shi-zhong_copy



软件增加功能：

1. 增加自动配置wifi网络功能（基于esp的smartconfig库）。手机端使用espTouch 软件app可以配置esp8266的wifi网络参数。
2. 增加wifi参数保存功能（基于esp的eeprom库）
3. 增加了一个黑色背景的gif动画（一个跑步的小老头），和源程序中自带的4种动画（白色背景），一共5个动画，可以配置使用哪个动画显示
4. 增加httpserver功能：通过http可以修改设置动画的类型，还可以通过http升级固件（http://ip/update）
5. 增加OTA升级功能，可通过OAT进行固件烧录
6. 增加mqtt功能。通过mqtt消息广播设置显示动画的类型
