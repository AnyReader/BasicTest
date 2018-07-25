
201807191912
ESP32_bt_I2S-master编译成功，还未测试。
https://github.com/od1969/ESP32_bt_I2S

看代码注释使用了PT8211

esp32_i2s_test
https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2s
只测出来16KHz方波


///////////////////////////////////////////////

gpio test
OK

Example: GPIO
This test code shows how to configure gpio and how to use gpio interrupt.

GPIO functions:
GPIO18: output
GPIO19: output
GPIO4: input, pulled up, interrupt from rising edge and falling edge
GPIO5: input, pulled up, interrupt from rising edge.
Test:
Connect GPIO18 with GPIO4
Connect GPIO19 with GPIO5
Generate pulses on GPIO18/19, that triggers interrupt on GPIO4/5


///////////////////////////////////////////////
timer_group test
OK

Example timer with auto reload


 Example timer without reload
Group[0], timer[0] alarm event
------- EVENT TIME --------
Counter: 0x000000000104c3ef
Time   : 3.41790380 s
-------- TASK TIME --------
Counter: 0x00000000010537af
Time   : 3.42383020 s

[13:58:23.930]收←◆
[13:58:23.955]收←◆
    Example timer with auto reload
Group[0], timer[1] alarm event
------- EVENT TIME --------
Counter: 0x0000000000000013
Time   : 0.00000380 s
-------- TASK TIME --------
Counter: 0x0000000000010b9b
Time   : 0.01370140 s

[13:58:24.982]收←◆
    Example timer wit
[13:58:25.013]收←◆hout reload
Group[0], timer[0] alarm event
------- EVENT TIME --------
Counter: 0x00000000020987de
Time   : 6.83580760 s
-------- TASK TIME --------
Counter: 0x00000000020a218d
Time   : 6.84367620 s

[13:58:28.400]收←◆
    Example timer without reload
Group[0], timer[0] alarm event
------- EVENT TIME --------
Counter: 0x00000000030e4bcd
Time   : 10.25371140 s
-------- TASK TIME --------
Counter: 0x00000000030f100a
Time   : 10.26375880 s


///////////////////////////////////////////////
LEDC (LED Controller) fade example
had  test
GPIO4 GPIO5 5KHz 占空比由0改变到40%左右  再到0

///////////////////////////////////////////////
brushed dc motor control example

pcnt_example_main.c
Pulse counter module - Example
Test OK 
输出100Hz占空比10% 计数

Sigma-delta Example
test OK
（占空比改变的PWM波）


https://github.com/FozzTexx/ws2812-demo
WS2812
示波器实测：
0:400ns 高电平（T0H）, 880ns 低电平（T0L）
1:960ns 高电平（T1H）,   T1L350ns 低电平（T1L）


//measure pwm frequency
 vTaskDelay(100 / portTICK_RATE_MS);//用示波器实测为198ms
 
 与程序log打印出的 5Hz 一致 
 -----Frequency : 5.00000000 Hz,pwmCount=50 ------
Group[0], timer[0] alarm event
------- EVENT TIME --------
Counter: 0x000000002faf0930
Time   : 160.00006080 s


 
 




