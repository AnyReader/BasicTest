
201807191912
ESP32_bt_I2S-master����ɹ�����δ���ԡ�
https://github.com/od1969/ESP32_bt_I2S

������ע��ʹ����PT8211

esp32_i2s_test
https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2s
ֻ�����16KHz����


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

[13:58:23.930]�ա���
[13:58:23.955]�ա���
    Example timer with auto reload
Group[0], timer[1] alarm event
------- EVENT TIME --------
Counter: 0x0000000000000013
Time   : 0.00000380 s
-------- TASK TIME --------
Counter: 0x0000000000010b9b
Time   : 0.01370140 s

[13:58:24.982]�ա���
    Example timer wit
[13:58:25.013]�ա���hout reload
Group[0], timer[0] alarm event
------- EVENT TIME --------
Counter: 0x00000000020987de
Time   : 6.83580760 s
-------- TASK TIME --------
Counter: 0x00000000020a218d
Time   : 6.84367620 s

[13:58:28.400]�ա���
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
GPIO4 GPIO5 5KHz ռ�ձ���0�ı䵽40%����  �ٵ�0

///////////////////////////////////////////////
brushed dc motor control example

pcnt_example_main.c
Pulse counter module - Example
Test OK 
���100Hzռ�ձ�10% ����

Sigma-delta Example
test OK
��ռ�ձȸı��PWM����


https://github.com/FozzTexx/ws2812-demo
WS2812
ʾ����ʵ�⣺
0:400ns �ߵ�ƽ��T0H��, 880ns �͵�ƽ��T0L��
1:960ns �ߵ�ƽ��T1H��,   T1L350ns �͵�ƽ��T1L��


//measure pwm frequency
 vTaskDelay(100 / portTICK_RATE_MS);//��ʾ����ʵ��Ϊ198ms
 
 �����log��ӡ���� 5Hz һ�� 
 -----Frequency : 5.00000000 Hz,pwmCount=50 ------
Group[0], timer[0] alarm event
------- EVENT TIME --------
Counter: 0x000000002faf0930
Time   : 160.00006080 s


 
 




