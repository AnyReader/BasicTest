// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#if 0
//esp32-bt-i2s
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "bt.h"
#include "bt_app_core.h"
#include "bt_app_av.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "i2sdac.h"

/* event for handler "bt_av_hdl_stack_up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

/* handler for bluetooth stack enabled events */
static void bt_av_hdl_stack_evt(uint16_t event, void *p_param);


void app_main()
{
    nvs_flash_init();

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed\n", __func__);
        return;
    }

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    if (esp_bluedroid_init() != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed\n", __func__);
        return;
    }

    if (esp_bluedroid_enable() != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed\n", __func__);
        return;
    }

    i2sdac_init();

    /* create application task */
    bt_app_task_start_up();

    /* Bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);



}


static void bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
  ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
  switch (event) {
  case BT_APP_EVT_STACK_UP: {
    /* set up device name */
    char *dev_name = "ESP32_DAC";
    esp_bt_dev_set_device_name(dev_name);

    /* initialize A2DP sink */
    esp_a2d_register_callback(&bt_app_a2d_cb);
    esp_a2d_register_data_callback(bt_app_a2d_data_cb);
    esp_a2d_sink_init();

    /* initialize AVRCP controller */
    esp_avrc_ct_init();
    esp_avrc_ct_register_callback(bt_app_rc_ct_cb);

    /* set discoverable and connectable mode, wait to be connected */
    esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
    break;
  }
  default:
    ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
    break;
  }
}



#endif


#if 0
/* I2S Example
    This example code will output 100Hz sine wave and triangle wave to 2-channel of I2S driver
    Every 5 seconds, it will change bits_per_sample [16, 24, 32] for i2s data
    This example code is in the Public Domain (or CC0 licensed, at your option.)
    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.


This example shows:
Init and using I2S module:

Generate 100Hz triangle wave in a channel, and sine wave in another, with 36Khz sample rates. Change bits per sample every 5 seconds

You can change bits per sample and sample rates with i2s_set_clk

*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"
#include <math.h>


#define SAMPLE_RATE     (36000)
#define I2S_NUM         (0)
#define WAVE_FREQ_HZ    (100)
#define PI 3.14159265

#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)

static void setup_triangle_sine_waves(int bits)
{
    int *samples_data = malloc(((bits+8)/16)*SAMPLE_PER_CYCLE*4);
    unsigned int i, sample_val;
    double sin_float, triangle_float, triangle_step = (double) pow(2, bits) / SAMPLE_PER_CYCLE;
    size_t i2s_bytes_write = 0;

    printf("\r\nTest bits=%d free mem=%d, written data=%d\n", bits, esp_get_free_heap_size(), ((bits+8)/16)*SAMPLE_PER_CYCLE*4);

    triangle_float = -(pow(2, bits)/2 - 1);

    for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
        sin_float = sin(i * PI / 180.0);
        if(sin_float >= 0)
            triangle_float += triangle_step;
        else
            triangle_float -= triangle_step;

        sin_float *= (pow(2, bits)/2 - 1);

        if (bits == 16) {
            sample_val = 0;
            sample_val += (short)sin_float;//triangle_float;
            sample_val = sample_val << 16;
            sample_val += (short) sin_float;
            samples_data[i] = sample_val;
        } else if (bits == 24) { //1-bytes unused
            samples_data[i*2] = ((int) triangle_float) << 8;
            samples_data[i*2 + 1] = ((int) sin_float) << 8;
        } else {
            samples_data[i*2] = ((int) triangle_float);
            samples_data[i*2 + 1] = ((int) sin_float);
        }

    }

    i2s_set_clk(I2S_NUM, SAMPLE_RATE, bits, 2);
    //Using push
    // for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
    //     if (bits == 16)
    //         i2s_push_sample(0, &samples_data[i], 100);
    //     else
    //         i2s_push_sample(0, &samples_data[i*2], 100);
    // }
    // or write
    //i2s_write(I2S_NUM, samples_data, ((bits+8)/16)*SAMPLE_PER_CYCLE*4, &i2s_bytes_write, 100);
    i2s_write_bytes(I2S_NUM, samples_data, i2s_bytes_write, 100);

    free(samples_data);
}
void app_main()
{
    //for 36Khz sample rates, we create 100Hz sine wave, every cycle need 36000/100 = 360 samples (4-bytes or 8-bytes each sample)
    //depend on bits_per_sample
    //using 6 buffers, we need 60-samples per buffer
    //if 2-channels, 16-bit each channel, total buffer is 360*4 = 1440 bytes
    //if 2-channels, 24/32-bit each channel, total buffer is 360*8 = 2880 bytes
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
        .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB,	//8211
        .dma_buf_count = 6,
        .dma_buf_len = 60,
        //.use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,
        .ws_io_num = 25,
        .data_out_num = 22,
        .data_in_num = -1                                                       //Not used
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);

    int test_bits = I2S_BITS_PER_SAMPLE_16BIT;
    while (1) {
        setup_triangle_sine_waves(test_bits);
        vTaskDelay(5000/portTICK_RATE_MS);
//        test_bits += 8;
//        if(test_bits > 32)
//            test_bits = 16;

    }

}

#endif


#if 0
//GPIO TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/uart.h"
#include "driver/rtc_io.h"

#include "esp_system.h"
//#include "esp_task_wdt.h"

#include "cjson.h"
#include "driver/timer.h"

#include "limits.h"
#include "lwip/netdb.h"

#define WIFI_SSID 	    "YZTEC"//"ChinaMobile"//"dong_zhang"//CONFIG_WIFI_SSID
#define WIFI_PASSWORD	"yizhi2017"//"{85208520}"//"qiangying"//CONFIG_WIFI_PASSWORD

#define ESPIDFV21RC 1

#if ESPIDFV21RC
  #include "esp_heap_alloc_caps.h"
#else
  #include "esp_heap_alloc_caps.h"
  #include "esp_heap_caps.h"
#endif

//Warning: This gets squeezed into IRAM.
volatile static uint32_t *currFbPtr __attribute__ ((aligned(4))) = NULL;

void app_main()
{

	 currFbPtr=pvPortMallocCaps(320*240*2, MALLOC_CAP_32BIT);//


}
#endif

#if 0
/****
 *
 *
 * uart Example and GPIO test  Charlin had tested ok in 201807251208
 *
 *
 */


/* uart Example and GPIO test
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "sdkconfig.h"

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO18: output
 * GPIO19: output
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Test:
 * Connect GPIO18 with GPIO4
 * Connect GPIO19 with GPIO5
 * Generate pulses on GPIO18/19, that triggers interrupt on GPIO4/5
 *
 */

#define GPIO_OUTPUT_IO_0    18
#define GPIO_OUTPUT_IO_1    19
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0     4
#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO 		15//CONFIG_BLINK_GPIO

void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void app_main()
{
    printf("Starting nvs_flash_init ...");
	nvs_flash_init();

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    for (int i = 3; i >= 0; i--) {
        printf("Countdown in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
//    printf("Restarting now.\n");
//    fflush(stdout);
//    esp_restart();

    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

    gpio_config_t io_conf;
       //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
       //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
       //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
       //disable pull-down mode
    io_conf.pull_down_en = 0;
       //disable pull-up mode
    io_conf.pull_up_en = 0;
       //configure GPIO with the given settings
    gpio_config(&io_conf);

       //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
       //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
       //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
       //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

       //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

       //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
       //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

       //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
       //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
       //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);

       //remove isr handler for gpio number.
    gpio_isr_handler_remove(GPIO_INPUT_IO_0);
       //hook isr handler for specific gpio pin again
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

    int cnt = 0;
    while(1) {
    	printf("cnt: %d\n", cnt++);
    	vTaskDelay(1000 / portTICK_RATE_MS);
    	gpio_set_level(GPIO_OUTPUT_IO_0, cnt % 2);
    	gpio_set_level(GPIO_OUTPUT_IO_1, cnt % 2);
    }
}

#endif

#if 0
/**
 * Timer test
 *
 *
 *
 *
 */


/* Timer group-hardware timer example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC   (3.4179) // sample test interval for the first timer
#define TIMER_INTERVAL1_SEC   (5.78)   // sample test interval for the second timer
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

/*
 * A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;

xQueueHandle timer_queue;

/*
 * A simple helper function to print the raw timer counter value
 * and the counter value converted to seconds
 */
static void inline print_timer_counter(uint64_t counter_value)
{
    printf("Counter: 0x%08x%08x\n", (uint32_t) (counter_value >> 32),
                                    (uint32_t) (counter_value));
    printf("Time   : %.8f s\n", (double) counter_value / TIMER_SCALE);
}

/*
 * Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */
void IRAM_ATTR timer_group0_isr(void *para)
{
    int timer_idx = (int) para;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[timer_idx].update = 1;
    uint64_t timer_counter_value =
        ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32
        | TIMERG0.hw_timer[timer_idx].cnt_low;

    /* Prepare basic event data
       that will be then sent back to the main program task */
    timer_event_t evt;
    evt.timer_group = 0;
    evt.timer_idx = timer_idx;
    evt.timer_counter_value = timer_counter_value;

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
    if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {
        evt.type = TEST_WITHOUT_RELOAD;
        TIMERG0.int_clr_timers.t0 = 1;
        timer_counter_value += (uint64_t) (TIMER_INTERVAL0_SEC * TIMER_SCALE);
        TIMERG0.hw_timer[timer_idx].alarm_high = (uint32_t) (timer_counter_value >> 32);
        TIMERG0.hw_timer[timer_idx].alarm_low = (uint32_t) timer_counter_value;
    } else if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_1) {
        evt.type = TEST_WITH_RELOAD;
        TIMERG0.int_clr_timers.t1 = 1;
    } else {
        evt.type = -1; // not supported even type
    }

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;

    /* Now just send the event data back to the main program task */
    xQueueSendFromISR(timer_queue, &evt, NULL);
}

/*
 * Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_sec - the interval of alarm to set
 */
static void example_tg0_timer_init(int timer_idx,
    bool auto_reload, double timer_interval_sec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

/*
 * The main task of this example program
 */
static void timer_example_evt_task(void *arg)
{
    while (1) {
        timer_event_t evt;
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        /* Print information that the timer reported an event */
        if (evt.type == TEST_WITHOUT_RELOAD) {
            printf("\n    Example timer without reload\n");
        } else if (evt.type == TEST_WITH_RELOAD) {
            printf("\n    Example timer with auto reload\n");
        } else {
            printf("\n    UNKNOWN EVENT TYPE\n");
        }
        printf("Group[%d], timer[%d] alarm event\n", evt.timer_group, evt.timer_idx);

        /* Print the timer values passed by event */
        printf("------- EVENT TIME --------\n");
        print_timer_counter(evt.timer_counter_value);  //3.41790380 s

        /* Print the timer values as visible by this task */
        printf("-------- TASK TIME --------\n");
        uint64_t task_counter_value;
        timer_get_counter_value(evt.timer_group, evt.timer_idx, &task_counter_value);
        print_timer_counter(task_counter_value);
    }
}

/*
 * In this example, we will test hardware timer0 and timer1 of timer group0.
 */
void app_main()
{
    timer_queue = xQueueCreate(10, sizeof(timer_event_t));
    example_tg0_timer_init(TIMER_0, TEST_WITHOUT_RELOAD, TIMER_INTERVAL0_SEC);
    example_tg0_timer_init(TIMER_1, TEST_WITH_RELOAD,    TIMER_INTERVAL1_SEC);
    xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);
}

#endif

#if 0
/* LEDC (LED Controller) fade example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

/*
 * About this example
 *
 * 1. Start with initializing LEDC module:
 *    a. Set the timer of LEDC first, this determines the frequency
 *       and resolution of PWM.
 *    b. Then set the LEDC channel you want to use,
 *       and bind with one of the timers.
 *
 * 2. You need first to install a default fade function,
 *    then you can use fade APIs.
 *
 * 3. You can also set a target duty directly without fading.
 *
 * 4. This example uses GPIO18/19/4/5 as LEDC output,
 *    and it will change the duty repeatedly.
 *
 * 5. GPIO18/19 are from high speed channel group.
 *    GPIO4/5 are from low speed channel group.
 *
 */
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (19)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH2_GPIO       (4)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO       (5)
#define LEDC_LS_CH3_CHANNEL    LEDC_CHANNEL_3

#define LEDC_TEST_CH_NUM       (4)
#define LEDC_TEST_DUTY         (4000)
#define LEDC_TEST_FADE_TIME    (3000)

void app_main()
{
    int ch;

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        //.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
    	.bit_num = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,           // timer mode
        .timer_num = LEDC_HS_TIMER            // timer index
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_LS_MODE;
    ledc_timer.timer_num = LEDC_LS_TIMER;
    ledc_timer_config(&ledc_timer);

    /*
     * Prepare individual configuration
     * for each channel of LED Controller
     * by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer,
     *         then frequency and bit_num of these channels
     *         will be the same
     */
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
        {
            .channel    = LEDC_HS_CH0_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH0_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .timer_sel  = LEDC_HS_TIMER
        },
        {
            .channel    = LEDC_HS_CH1_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH1_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .timer_sel  = LEDC_HS_TIMER
        },
        {
            .channel    = LEDC_LS_CH2_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_LS_CH2_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .timer_sel  = LEDC_LS_TIMER
        },
        {
            .channel    = LEDC_LS_CH3_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_LS_CH3_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .timer_sel  = LEDC_LS_TIMER
        },
    };

    // Set LED Controller with previously prepared configuration
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);

    while (1) {
        printf("1. LEDC fade up to duty = %d\n", LEDC_TEST_DUTY);
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }
        vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);

        printf("2. LEDC fade down to duty = 0\n");
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, 0, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }
        vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);

        printf("3. LEDC set duty = %d without fade\n", LEDC_TEST_DUTY);
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, LEDC_TEST_DUTY);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("4. LEDC set duty = 0 without fade\n");
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, 0);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

#endif


#if 0
/* brushed dc motor control example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * This example will show you how to use MCPWM module to control brushed dc motor.
 * This code is tested with L298 motor driver.
 * User may need to make changes according to the motor driver they use.
*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#define GPIO_PWM0A_OUT 15   //Set GPIO 15 as PWM0A
#define GPIO_PWM0B_OUT 16   //Set GPIO 16 as PWM0B

static void mcpwm_example_gpio_initialize()
{
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}

/**
 * @brief Configure MCPWM module for brushed dc motor
 */
static void mcpwm_example_brushed_motor_control(void *arg)
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    while (1) {
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 50.0);
        vTaskDelay(2000 / portTICK_RATE_MS);
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 30.0);
        vTaskDelay(2000 / portTICK_RATE_MS);
        brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

void app_main()
{
    printf("Testing brushed motor...\n");
    xTaskCreate(mcpwm_example_brushed_motor_control, "mcpwm_examlpe_brushed_motor_control", 4096, NULL, 5, NULL);
}

#endif



#if 0
/* Pulse counter module - Example
   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "soc/gpio_sig_map.h"

/**
 * TEST CODE BRIEF
 *
 * Use PCNT module to count rising edges generated by LEDC module.
 *
 * Functionality of GPIOs used in this example:
 *   - GPIO18 - output pin of a sample 1 Hz pulse generator,
 *   - GPIO4 - pulse input pin,
 *   - GPIO5 - control input pin.
 *
 * Load example, open a serial port to view the message printed on your screen.
 *
 * To do this test, you should connect GPIO18 with GPIO4.
 * GPIO5 is the control signal, you can leave it floating with internal pull up,
 * or connect it to ground. If left floating, the count value will be increasing.
 * If you connect GPIO5 to GND, the count value will be decreasing.
 *
 * An interrupt will be triggered when the counter value:
 *   - reaches 'thresh1' or 'thresh0' value,
 *   - reaches 'l_lim' value or 'h_lim' value,
 *   - will be reset to zero.
 */
#define PCNT_TEST_UNIT      PCNT_UNIT_0
#define PCNT_H_LIM_VAL      10
#define PCNT_L_LIM_VAL     -10
#define PCNT_THRESH1_VAL    5
#define PCNT_THRESH0_VAL   -5

#define PCNT_INPUT_SIG_IO   4  // Pulse Input GPIO
#define PCNT_INPUT_CTRL_IO  5  // Control GPIO HIGH=count up, LOW=count down
#define LEDC_OUTPUT_IO      18 // Output GPIO of a sample 1 Hz pulse generator

xQueueHandle pcnt_evt_queue;   // A queue to handle pulse counter events

/* A sample structure to pass events from the PCNT
 * interrupt handler to the main program.
 */
typedef struct {
    int unit;  // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
static void IRAM_ATTR pcnt_example_intr_handler(void *arg)
{
    uint32_t intr_status = PCNT.int_st.val;
    int i;
    pcnt_evt_t evt;
    portBASE_TYPE HPTaskAwoken = pdFALSE;

    for (i = 0; i < PCNT_UNIT_MAX; i++) {
        if (intr_status & (BIT(i))) {
            evt.unit = i;
            /* Save the PCNT event type that caused an interrupt
               to pass it to the main program */
            evt.status = PCNT.status_unit[i].val;
            PCNT.int_clr.val = BIT(i);
            xQueueSendFromISR(pcnt_evt_queue, &evt, &HPTaskAwoken);
            if (HPTaskAwoken == pdTRUE) {
                portYIELD_FROM_ISR();
            }
        }
    }
}

/* Configure LED PWM Controller
 * to output sample pulses at 1 Hz with duty of about 10%
 */
static void ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer;
    ledc_timer.speed_mode   = LEDC_HIGH_SPEED_MODE;
    ledc_timer.timer_num        = LEDC_TIMER_1;
    //ledc_timer.duty_resolution  = LEDC_TIMER_10_BIT;
    ledc_timer.bit_num  = LEDC_TIMER_10_BIT;
    ledc_timer.freq_hz          = 1;  // set output frequency at 1 Hz
    ledc_timer_config(&ledc_timer);

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel;
    ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel.channel    = LEDC_CHANNEL_1;
    ledc_channel.timer_sel  = LEDC_TIMER_1;
    ledc_channel.intr_type  = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num   = LEDC_OUTPUT_IO;
    ledc_channel.duty       = 100; // set duty at about 10%   =duty/(2^bit_num)
    ledc_channel_config(&ledc_channel);
}

/* Initialize PCNT functions:
 *  - configure and initialize PCNT
 *  - set up the input filter
 *  - set up the counter events to watch
 */
static void pcnt_example_init(void)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_TEST_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(PCNT_TEST_UNIT, 100);
    pcnt_filter_enable(PCNT_TEST_UNIT);

    /* Set threshold 0 and 1 values and enable events to watch */
    pcnt_set_event_value(PCNT_TEST_UNIT, PCNT_EVT_THRES_1, PCNT_THRESH1_VAL);
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_THRES_1);
    pcnt_set_event_value(PCNT_TEST_UNIT, PCNT_EVT_THRES_0, PCNT_THRESH0_VAL);
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_THRES_0);
    /* Enable events on zero, maximum and minimum limit values */
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_ZERO);
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_H_LIM);
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_TEST_UNIT);
    pcnt_counter_clear(PCNT_TEST_UNIT);

    /* Register ISR handler and enable interrupts for PCNT unit */
    pcnt_isr_register(pcnt_example_intr_handler, NULL, 0, NULL);
    pcnt_intr_enable(PCNT_TEST_UNIT);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(PCNT_TEST_UNIT);
}

void app_main()
{
    /* Initialize LEDC to generate sample pulse signal */
    ledc_init();

    /* Initialize PCNT event queue and PCNT functions */
    pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
    pcnt_example_init();

    int16_t count = 0;
    pcnt_evt_t evt;
    portBASE_TYPE res;
    while (1) {
        /* Wait for the event information passed from PCNT's interrupt handler.
         * Once received, decode the event type and print it on the serial monitor.
         */
        res = xQueueReceive(pcnt_evt_queue, &evt, 1000 / portTICK_PERIOD_MS);
        if (res == pdTRUE) {
            pcnt_get_counter_value(PCNT_TEST_UNIT, &count);
            printf("Event PCNT unit[%d]; cnt: %d\n", evt.unit, count);
            if (evt.status & PCNT_STATUS_THRES1_M) {
                printf("THRES1 EVT\n");
            }
            if (evt.status & PCNT_STATUS_THRES0_M) {
                printf("THRES0 EVT\n");
            }
            if (evt.status & PCNT_STATUS_L_LIM_M) {
                printf("L_LIM EVT\n");
            }
            if (evt.status & PCNT_STATUS_H_LIM_M) {
                printf("H_LIM EVT\n");
            }
            if (evt.status & PCNT_STATUS_ZERO_M) {
                printf("ZERO EVT\n");
            }
        } else {
            pcnt_get_counter_value(PCNT_TEST_UNIT, &count);
            printf("Current counter value :%d\n", count);
        }
    }
}

#endif


#if 0
/* Sigma-delta Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/sigmadelta.h"
/*
 * This test code will configure sigma-delta and set GPIO4 as a signal output pin.
 * If you connect this GPIO4 with a LED, you will see the LED blinking slowly.
 */

/*
 * Configure and initialize the sigma delta modulation
 * on channel 0 to output signal on GPIO4
 */
static void sigmadelta_example_init(void)
{
    sigmadelta_config_t sigmadelta_cfg = {
        .channel = SIGMADELTA_CHANNEL_0,
        .sigmadelta_prescale = 80,
        .sigmadelta_duty = 0,
        .sigmadelta_gpio = 4,
    };
    sigmadelta_config(&sigmadelta_cfg);
}

/*
 *  Perform the sigma-delta modulation test
 *  by changing the duty of the output signal.
 */
void app_main()
{
    sigmadelta_example_init();

    int8_t duty = 0;
    int inc = 1;
    while (1) {
        sigmadelta_set_duty(SIGMADELTA_CHANNEL_0, duty);
        /* By changing delay time, you can change the blink frequency of LED */
        vTaskDelay(10 / portTICK_PERIOD_MS);

        duty += inc;
        if (duty == 127 || duty == -127) {
            inc = (-1) * inc;
        }
    }
}

#endif


#if 0
/* Created 19 Nov 2016 by Chris Osborn <fozztexx@fozztexx.com>
 * http://insentricity.com
 *
 * Demo of driving WS2812 RGB LEDs using the RMT peripheral.
 *
 * This code is placed in the public domain (or CC0 licensed, at your option).
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/rmt_struct.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <stdio.h>
#include "ws2812.h"

#define WS2812_PIN	13

#define delay_ms(ms) vTaskDelay((ms) / portTICK_RATE_MS)

void rainbow(void *pvParameters)
{
  const uint8_t anim_step = 10;
  const uint8_t anim_max = 255;//oringin is 250 change to 255 by Charlin
  const uint32_t pixel_count = 64; // Number of your "pixels"
  const uint8_t delayTime = 25; // duration between color changes
  rgbVal color = makeRGBVal(anim_max, 0, 0);
  uint8_t step = 0;
  rgbVal color2 = makeRGBVal(anim_max, 0, 0);
  uint8_t step2 = 0;
  rgbVal *pixels;

  pixels = malloc(sizeof(rgbVal) * pixel_count);

  while (1) {
    color = color2;
    step = step2;

    for (uint8_t i = 0; i < pixel_count; i++) {
      pixels[i] = color;

      if (i == 1) {
        color2 = color;
        step2 = step;
      }

      switch (step) {
      case 0:
        color.g += anim_step;
        if (color.g >= anim_max)
          step++;
        break;
      case 1:
        color.r -= anim_step;
        if (color.r == 0)
          step++;
        break;
      case 2:
        color.b += anim_step;
        if (color.b >= anim_max)
          step++;
        break;
      case 3:
        color.g -= anim_step;
        if (color.g == 0)
          step++;
        break;
      case 4:
        color.r += anim_step;
        if (color.r >= anim_max)
          step++;
        break;
      case 5:
        color.b -= anim_step;
        if (color.b == 0)
          step = 0;
        break;
      }
    }

    ws2812_setColors(pixel_count, pixels);
    printf("free mem=%d \r\n", esp_get_free_heap_size());
    delay_ms(delayTime);
  }
}

void app_main()
{
  nvs_flash_init();

  ws2812_init(WS2812_PIN);
  xTaskCreate(rainbow, "ws2812 rainbow demo", 4096, NULL, 10, NULL);

  return;
}


#endif


//measure pwm frequency
//author:Charlin
//https://github.com/espressif/esp-idf/tree/master/examples/peripherals/gpio
//https://github.com/espressif/esp-idf/tree/master/examples/peripherals/timer_group



/* GPIO Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "esp_types.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_system.h"
#include "sys/time.h"
#include "stdint.h"

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO18: output
 * GPIO19: output
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Test:
 * Connect GPIO18 with GPIO4
 * Connect GPIO19 with GPIO5
 * Generate pulses on GPIO18/19, that triggers interrupt on GPIO4/5
 *
 */

#define GPIO_OUTPUT_IO_0    18
#define GPIO_OUTPUT_IO_1    19
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0     4
#define GPIO_INPUT_IO_1     33
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;

unsigned int pwmCount=0;
int getTime=0;
int64_t current_time;
static int64_t last_time = 0;

int64_t current_time;

struct timeval system_time;

struct timeval tv1, tv2;
uint64_t t1;
uint64_t t2;

static inline uint64_t timeval_to_usec(struct timeval* tv)
{
    return 1000000LL * tv->tv_sec + tv->tv_usec;
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    if(gpio_num == GPIO_INPUT_IO_1 )
    {
    	if(pwmCount==1)
    	{
    		/* 获取此时的时间 */
        	gettimeofday(&system_time,NULL);
        	/* 转换成ms */
        	//current_time = system_time.tv_usec*1000+(system_time.tv_sec/1000);

        	//printf("currentTime %llu",current_time);  //reboot why

            gettimeofday(&tv1, NULL);
            //gettimeofday(&tv2, NULL);
            t1 = timeval_to_usec(&tv1);

    	}
    	pwmCount++;
    	if(pwmCount>=1001)
    	{
    		//gettimeofday(&system_time,NULL);
    		//last_time = system_time.tv_usec*1000+(system_time.tv_sec/1000);

    		//f=(last_time-current_time)/1000  that is  T=1000/((last_time-current_time))
    		//C=T*1.443/(R1 + 2*R2)  = 1443.0/((last_time-current_time)*104.0)
    		//uint32_t getTime= (last_time-current_time);
    		//printf("currentTime %d",getTime);
    		//printf("======Capacity : %.8f F,Timevalue=%.9d ------\n", (double) ((double)1443.0/(getTime*104.0)),getTime); // //reboot why

    		gettimeofday(&tv2, NULL);
    		t2 = timeval_to_usec(&tv2);

    		pwmCount=0;
    		 xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    	}

    }


}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d/%d \n", io_num, gpio_get_level(io_num),pwmCount);

    		printf("t1=%lld t2=%lld t2-t1=%lld\r\n", t1, t2, t2 - t1);
    		uint64_t  t=(double)(t2 - t1)/1000.0;
    		printf("-----Capacity : %.9f uF ------\n", (double)t/(double)(0.693*104000));
           // printf("==Counter: 0x%08x%08x\n", (uint32_t) (current_time >> 32), (uint32_t) (current_time));
           // printf("--Counter: 0x%08x%08x\n", (uint32_t) (last_time >> 32),(uint32_t) (last_time));
        }
    }
}

//timer
#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC   (10.0f)//(3.4179) // sample test interval for the first timer
#define TIMER_INTERVAL1_SEC   (5.78)   // sample test interval for the second timer
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

/*
 * A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;

xQueueHandle timer_queue;

/*
 * A simple helper function to print the raw timer counter value
 * and the counter value converted to seconds
 */
static void inline print_timer_counter(uint64_t counter_value)
{
    printf("Counter: 0x%08x%08x\n", (uint32_t) (counter_value >> 32),
                                    (uint32_t) (counter_value));
    printf("Time   : %.8f s\n", (double) counter_value / TIMER_SCALE);
}

/*
 * Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */
void IRAM_ATTR timer_group0_isr(void *para)
{
    int timer_idx = (int) para;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[timer_idx].update = 1;
    uint64_t timer_counter_value =
        ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32
        | TIMERG0.hw_timer[timer_idx].cnt_low;

    /* Prepare basic event data
       that will be then sent back to the main program task */
    timer_event_t evt;
    evt.timer_group = 0;
    evt.timer_idx = timer_idx;
    evt.timer_counter_value = timer_counter_value;

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
    if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {
        evt.type = TEST_WITHOUT_RELOAD;
        TIMERG0.int_clr_timers.t0 = 1;
        timer_counter_value += (uint64_t) (TIMER_INTERVAL0_SEC * TIMER_SCALE);
        TIMERG0.hw_timer[timer_idx].alarm_high = (uint32_t) (timer_counter_value >> 32);
        TIMERG0.hw_timer[timer_idx].alarm_low = (uint32_t) timer_counter_value;
    } else if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_1) {
        evt.type = TEST_WITH_RELOAD;
        TIMERG0.int_clr_timers.t1 = 1;
    } else {
        evt.type = -1; // not supported even type
    }

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;

    /* Now just send the event data back to the main program task */
    xQueueSendFromISR(timer_queue, &evt, NULL);
}

/*
 * Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_sec - the interval of alarm to set
 */
static void example_tg0_timer_init(int timer_idx,
    bool auto_reload, double timer_interval_sec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}


#define _555_R1  10000  //10kohm
#define _555_R2  47000	//47Kohm
#define _555_C  0.0000001  //0.1uf
/*
 * The main task of this example program
 */
static void timer_example_evt_task(void *arg)
{
    while (1) {
        timer_event_t evt;
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        /* Print information that the timer reported an event */
        if (evt.type == TEST_WITHOUT_RELOAD) {
            //printf("\n    Example timer without reload \n");
            if(pwmCount)
            {
            	printf("-----Frequency : %.8f Hz,pwmCount=%d ------\n", (double) ((double)pwmCount/(TIMER_INTERVAL0_SEC)),pwmCount);
            	printf("-----Capacity : %.8f F ------\n", (double) (((double)1.443*TIMER_INTERVAL0_SEC/(double)pwmCount)/104000.0));
            	//c=1.443/(f*(R13+2*R14))   F = 1.44 / ( (R1 + 2R2) * C)    T=( (R1 + 2R2) * C)  /1.443  C=T*1.443/(R1 + 2*R2)
            	//R13=10K R14=47K   C=(TIMER_INTERVAL0_SEC)*1.443/((_555_R1 + 2*_555_R2)*(double)pwmCount)
            	pwmCount=0;
            }

        } else if (evt.type == TEST_WITH_RELOAD) {
            printf("\n    Example timer with auto reload\n");
        } else {
            printf("\n    UNKNOWN EVENT TYPE\n");
        }

        printf("Group[%d], timer[%d] alarm event\n", evt.timer_group, evt.timer_idx);

        /* Print the timer values passed by event */
        printf("------- EVENT TIME --------\n");
        print_timer_counter(evt.timer_counter_value);

        /* Print the timer values as visible by this task */
//        printf("-------- TASK TIME --------\n");
//        uint64_t task_counter_value;
//        timer_get_counter_value(evt.timer_group, evt.timer_idx, &task_counter_value);
//        print_timer_counter(task_counter_value);

    }
}




void app_main()
{
	//timer
//    timer_queue = xQueueCreate(11, sizeof(timer_event_t));
//    example_tg0_timer_init(TIMER_0, TEST_WITHOUT_RELOAD, TIMER_INTERVAL0_SEC);
    //example_tg0_timer_init(TIMER_1, TEST_WITH_RELOAD,    TIMER_INTERVAL1_SEC);
//    xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);


	//gpio
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);

    //remove isr handler for gpio number.
    gpio_isr_handler_remove(GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin again
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

    int cnt = 0;



    while(1) {
        //printf("cnt: %d\n", cnt++);
        cnt++;
        //vTaskDelay(100 / portTICK_RATE_MS);//示波器实测 200ms
        vTaskDelay(10 / portTICK_RATE_MS);//示波器实测 50Hz 20ms
        //vTaskDelay(1000 / portTICK_RATE_MS);//示波器实测1Hz  1s
        gpio_set_level(GPIO_OUTPUT_IO_0, cnt % 2);
        gpio_set_level(GPIO_OUTPUT_IO_1, cnt % 2);
#if 0
        struct timeval tv1, tv2;
        gettimeofday(&tv1, NULL);
        gettimeofday(&tv2, NULL);
        uint64_t t1 = timeval_to_usec(&tv1);
        uint64_t t2 = timeval_to_usec(&tv2);
        printf("t1=%lld t2=%lld t2-t1=%lld\r\n", t1, t2, t2 - t1);  //1000000us
#endif

    }
}


