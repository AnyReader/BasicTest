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
    //printf("free mem=%d \r\n", esp_get_free_heap_size());
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



#if 0
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
#include "sys/time.h"
#include "esp_system.h"


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
uint64_t timeValue=0;

uint64_t currentTime;
uint64_t nextTime;

uint64_t minusTime;

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
    	pwmCount++;
    	if(pwmCount==1)
    	{
    		 gettimeofday(&tv1, NULL);
    		 t1 = timeval_to_usec(&tv1);
    	}
    	else if(pwmCount>=1001)
    	{
    		gettimeofday(&tv2, NULL);
    		t2 = timeval_to_usec(&tv2);
    		pwmCount=0;
    		xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    	}
    }


   //
}


static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            //printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
    		printf("t1=%lld t2=%lld t2-t1=%lld\r\n", t1, t2, t2 - t1);
    		uint64_t  t=(double)(t2 - t1)/1000.0;
    		if(t>1000)
    		printf("-----Capacity : %.9f uF ------\n", (double)t/(double)(0.693*104000));//c= t*k
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
            	printf("-----Capacity : %.8fuF ------\n", (double) (((double)14.43*TIMER_INTERVAL0_SEC/(double)pwmCount)/1.04));
            	//pwmCount=0;
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
    }
}

#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/uart.h"

//#include "esp_task_wdt.h"

#include "cjson.h"
#include "driver/timer.h"

#include "limits.h"
#include "esp_log.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_task_wdt.h"  //watchdog
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "soc/spi_reg.h"
#include "driver/hspi.h"
#include "soc/gpio_reg.h"
#include "esp_attr.h"

#include "soc/gpio_struct.h"
#include "freertos/semphr.h"
#include "esp_err.h"

#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "driver/adc.h"


#define	 WIFI_SSID 	    "dong_zhang"//"ChinaMobile"//"dong_zhang"//////CONFIG_WIFI_SSID
#define	 WIFI_PASSWORD	 "qiangying"//"{85208520}"//"qiangying"//////CONFIG_WIFI_PASSWORD
#define	 srcMAC		"B4-E6-2D-B8-3C-F5"//steven//"30-AE-A4-43-86-60"//"A4-E9-75-3D-DC-DF"//"B4-E6-2D-B8-3C-F5"
static char	 strMac[15];
static uint8_t wifiMac[6];
//tcp
int g_iSock_fd;
#define SERVER_IP  		"39.106.151.85"//"192.168.1.103"//
#define REMOTE_PORT		8088
uint32_t port=8086;

//#include "spiram.h"
#define ESPDHT11	0  //DHT11与DS18B20 不共存  能否先测量DHT11再测量湿度？
#if ESPDHT11
  #include "dht11.h"
#define DHT_GPIO 3
	uint8_t dhtData[4];
#endif

#define ADCENABLE  0  //通过ADC测量与通过电容测试不共存
#if (ADCENABLE==1)
#include "driver/adc.h"

#define ADC1_TEST_CHANNEL (ADC1_CHANNEL_5)  //io33   get Humidity via adc
#endif


#define		capHumidity		1//measure Humidity via capacity
#if (capHumidity==1)

#define GPIO_INPUT_IO_0     0
#define GPIO_INPUT_IO_1     33
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

#define _555_R1 10000  //10K
#define _555_R2 47000  //47K
#define _555_Rsum    (_555_R1+2*_555_R2)

static xQueueHandle gpio_evt_queue = NULL;

double capacity=0.0f;
unsigned int pwmCount=0;
uint64_t timeValue=0;

uint64_t currentTime;
uint64_t nextTime;

uint64_t minusTime;

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
    	pwmCount++;
    	if(pwmCount==1)
    	{
    		gettimeofday(&tv1, NULL);
    		t1 = timeval_to_usec(&tv1);
    	}
    	else if(pwmCount>=1001)//测量1000个脉冲 占用多少时间
    	{
    		gettimeofday(&tv2, NULL);
    		t2 = timeval_to_usec(&tv2);
    		pwmCount=0;
    	    //remove isr handler for gpio number.
    	    gpio_isr_handler_remove(GPIO_INPUT_IO_1);
    		xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    	}
    }
    else if(gpio_num == GPIO_INPUT_IO_0 )
    {

    }
}



static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;)
    {

        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
    		printf("t1=%lld t2=%lld t2-t1=%lld\r\n", t1, t2, t2 - t1);
    		uint64_t  t=(double)(t2 - t1)/1000.0; //unit: us // T = [ C( R1 + 2 R2) ln2) ]   ==> C = T/[( R1 + 2 R2) ln2]
    		//if(t>100)
    		{
    			capacity=1000000.0 *(double)t/(double)(0.693*_555_Rsum);  //unit: pF
    			printf("-----Capacity : %.9fpF ------\n", capacity);//c= t*k
    		    //hook isr handler for specific gpio pin again
    		    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
    		}

        }

    }
}

#endif //end of capHumidity

#define ESPDS18B20	0
#if ESPDS18B20==1
#include "ds18b20.h" //Include library
const int DS_PIN = 3; //GPIO where you connected ds18b20

uint8_t dsROM[8][5]=
{
	{0x28, 0xFF ,0x8E ,0x4D ,0xA2 ,0x16 ,0x05 ,0x9C},
	{0x28, 0xFF ,0x8E ,0x4D ,0xA2 ,0x16 ,0x05 ,0x9C},
	{0x28, 0xFF ,0x8E ,0x4D ,0xA2 ,0x16 ,0x05 ,0x9C},
	{0x28, 0xFF ,0x8E ,0x4D ,0xA2 ,0x16 ,0x05 ,0x9C},
	{0x28, 0xFF ,0x8E ,0x4D ,0xA2 ,0x16 ,0x05 ,0x9C}
};
#endif

#define ESPIDFV21RC 1

#define BTENABLE		0

#if BTENABLE
//bt start
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

static void bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
  ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
  switch (event) {
  case BT_APP_EVT_STACK_UP: {
    /* set up device name */
    char *dev_name = "GreenBox";
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
//bt end
#endif



#if ESPIDFV21RC
  #include "esp_heap_alloc_caps.h"
#else
  #include "esp_heap_alloc_caps.h"
  #include "esp_heap_caps.h"
#endif



size_t free8start=0, free32start=0, free8=0, free32=0;

static const char* TAG = "ESP-CAM";

void tcp_server_task(void* arg);
static void tcp_cli_task(void *pvParameters);

//=======================================================================

uint8_t colorR=0,colorG=0,colorB=255;
uint8_t brightValue=10;

uint8_t nurseMode=0;//护理模式
typedef enum
{
	manual				=	0,	//手动护理
	semiAutomatic		=	1,	//半自动护理
	automatic			=	2,	//自动护理
	customized			=	3,	//自定义护理
	aiMode				=	4,	//人工智能护理
	undefined			=	5		//未定义
}runMode;

uint8_t ledMode=0;//Led模式 显示效果


uint8_t setHumidity,getHumidity;//0~100%
uint8_t setTemp,getTemp;//10~40C
#define setTempMax 40
#define setTempMin 10
uint16_t getIllum=0;

uint8_t boolValue;//几个布尔状态值
//BIT0:LED开关
//BIT1:Fan开关
//BIT2:PUMP开关(水泵开关)
//BIT3:Audio开关
//BIT4:Live开关（活体检测开关）
//BIT5:Heater开关
//BIT6:PUMP开关(气泵开关)

//uint8_t ledModel=0;



//start of CAMENABLE
#define CAMENABLE	1
#if (CAMENABLE==1)
//Warning: This gets squeezed into IRAM.
volatile static uint32_t *currFbPtr __attribute__ ((aligned(4))) = NULL;

#endif
//end of CAMENABLE


//
#define ESPBH1750	1  //start of BH1750
#if ESPBH1750==1
  #include "bh1750.h"
#define I2C_SDA 14
#define I2C_SCL 27
#endif  //end of BH1750


//relay
#define RELAY_0_IO_NUM   	15	//
#define RELAY_1_IO_NUM		12	//io12  test ok

#define close_relay_0()		gpio_set_level(RELAY_0_IO_NUM, 0)
#define open_relay_0()		gpio_set_level(RELAY_0_IO_NUM, 1)

#define close_relay_1()		gpio_set_level(RELAY_1_IO_NUM, 0)   //
#define open_relay_1()		gpio_set_level(RELAY_1_IO_NUM, 1)  //test ok

void gpioInit()
{
    gpio_pad_select_gpio(RELAY_0_IO_NUM);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(RELAY_0_IO_NUM , GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(RELAY_0_IO_NUM , GPIO_PULLDOWN_ONLY);
    gpio_set_level(RELAY_0_IO_NUM, 1);

    gpio_pad_select_gpio(RELAY_1_IO_NUM);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(RELAY_1_IO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(RELAY_1_IO_NUM , GPIO_PULLDOWN_ONLY);
    gpio_set_level(RELAY_1_IO_NUM, 0);

}

#define ESPWS2812	1

//=======================================================================
#if ESPWS2812==1 //(0)
#include "ws2812.h" //Include library

#define WS2812_PIN	13
const uint8_t pixel_count = 66; // Number of your "pixels"
wsRGB_t colorRGB;
wsRGB_t *pixels;
#define RMT_TX_CHANNEL 	RMT_CHANNEL_0

#define delay_ms(ms) vTaskDelay((ms) / portTICK_RATE_MS)

xTaskHandle xHandleXDisplay=NULL;

unsigned char PWM_TABLE[11]=
{
	1,
	2,
	3,
	5,
	9,
	16,
	28,
	48,
	84,
	147,
	255
};

#define CONST_RESPIRATION_LAMP_SERIES 80
unsigned char RESPIRATION_LAMP_TABLE[] =
{
    1,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    3,
    3,
    3,
    3,
    3,
    4,
    4,
    4,
    5,
    5,
    5,
    6,
    6,
    6,
    7,
    7,
    8,
    9,
    9,
    10,
    11,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    20,
    21,
    23,
    24,
    26,
    28,
    30,
    32,
    34,
    37,
    39,
    42,
    45,
    48,
    52,
    56,
    60,
    64,
    68,
    73,
    79,
    84,
    90,
    97,
    104,
    111,
    119,
    128,
    137,
    147,
    157,
    168,
    180,
    193,
    207,
    222,
    238,
    255,
    255
};

xTaskHandle xHandleLedDisplay = NULL;
void rgbDisplay(void *pvParameters)
{

	while(1)
	{
		//esp_task_wdt_reset();
		esp_task_wdt_feed();
#if ESPWS2812==1
    	if(ledMode==2)
    	{
    		//green
    		//pulsing LED/pulsing rhythm/breathing LED
    		for(int i=0;i<CONST_RESPIRATION_LAMP_SERIES-20;i++)
    		{
    			colorRGB.r=0;colorRGB.g=RESPIRATION_LAMP_TABLE[i];colorRGB.b=0;
    			for (uint8_t i = 0; i < pixel_count; i++)
    			{
    				pixels[i] = colorRGB;
    			}
    			WS2812B_setLeds(pixels,pixel_count);
    			ets_delay_us(20000);//15ms  //18
    		}

    		for(int i=CONST_RESPIRATION_LAMP_SERIES-20;i>0;i--)
    		{
    			colorRGB.r=0;colorRGB.g=RESPIRATION_LAMP_TABLE[i];colorRGB.b=0;
    			for (uint8_t i = 0; i < pixel_count; i++)
    			{
    		           pixels[i] = colorRGB;
    			}
    			WS2812B_setLeds(pixels,pixel_count);
    			ets_delay_us(20000);//15ms
    		}

    		colorRGB.r=0;colorRGB.g=0;colorRGB.b=0;
    		for (uint8_t i = 0; i < pixel_count; i++)
    		{
    			pixels[i] = colorRGB;
    		}
    		WS2812B_setLeds(pixels,pixel_count);
    		ets_delay_us(100000);//100ms

    		//blue
    		//pulsing LED/pulsing rhythm/breathing LED
    		for(int i=0;i<CONST_RESPIRATION_LAMP_SERIES-20;i++)
    		{
    			colorRGB.r=0;colorRGB.g=0;colorRGB.b=RESPIRATION_LAMP_TABLE[i];
    			for (uint8_t i = 1; i < pixel_count; i++)
    			{
    				pixels[i] = colorRGB;
    			}
    			WS2812B_setLeds(pixels,pixel_count);
    			ets_delay_us(20000);//15ms  //18
    		}

    		for(int i=CONST_RESPIRATION_LAMP_SERIES-20;i>0;i--)
    		{
    			colorRGB.r=0;colorRGB.g=0;colorRGB.b=RESPIRATION_LAMP_TABLE[i];
    			for (uint8_t i = 0; i < pixel_count; i++)
    			{
    		           pixels[i] = colorRGB;
    			}
    			WS2812B_setLeds(pixels,pixel_count);
    			ets_delay_us(20000);//15ms
    		}

    		colorRGB.r=0;colorRGB.g=0;colorRGB.b=0;
    		for (uint8_t i = 0; i < pixel_count; i++)
    		{
    			pixels[i] = colorRGB;
    		}
    		WS2812B_setLeds(pixels,pixel_count);
    		ets_delay_us(100000);//100ms
//red
    		for(int i=0;i<CONST_RESPIRATION_LAMP_SERIES-20;i++)
    		{
    			colorRGB.r=RESPIRATION_LAMP_TABLE[i];colorRGB.g=0;colorRGB.b=0;
    			for (uint8_t i = 1; i < pixel_count; i++)
    			{
    				pixels[i] = colorRGB;
    			}
    			WS2812B_setLeds(pixels,pixel_count);
    			ets_delay_us(20000);//15ms  //18
    		}

    		for(int i=CONST_RESPIRATION_LAMP_SERIES-20;i>0;i--)
    		{
    			colorRGB.r=RESPIRATION_LAMP_TABLE[i];colorRGB.g=0;colorRGB.b=0;
    			for (uint8_t i = 0; i < pixel_count; i++)
    			{
    		           pixels[i] = colorRGB;
    			}
    			WS2812B_setLeds(pixels,pixel_count);
    			ets_delay_us(20000);//15ms
    		}

    		colorRGB.r=0;colorRGB.g=0;colorRGB.b=0;
    		for (uint8_t i = 0; i < pixel_count; i++)
    		{
    			pixels[i] = colorRGB;
    		}
    		WS2812B_setLeds(pixels,pixel_count);
    		ets_delay_us(100000);//100ms
    	}
    	else if(ledMode==3) //flow LED
    	{
    		for (uint8_t i = 0; i < pixel_count; i++)
    		{
    		    colorRGB.r=0;colorRGB.g=0;colorRGB.b=PWM_TABLE[10];
    		    pixels[i] = colorRGB;
    		    WS2812B_setLeds(pixels,i);
    		    vTaskDelay(500/portTICK_PERIOD_MS);
    		}
    		for (uint8_t i = 0; i < pixel_count; i++)
    		{
    		     colorRGB.r=PWM_TABLE[10];colorRGB.g=0;colorRGB.b=0;
    		     pixels[i] = colorRGB;
    		     WS2812B_setLeds(pixels,i);
    		     vTaskDelay(500/portTICK_PERIOD_MS);
    		 }
    		 for (uint8_t i = 0; i < pixel_count; i++)
    		 {
    		     colorRGB.r=0;colorRGB.g=PWM_TABLE[10];colorRGB.b=0;
    		     pixels[i] = colorRGB;
    		     WS2812B_setLeds(pixels,i);
    		     vTaskDelay(500/portTICK_PERIOD_MS);
    		 }
    		 for (uint8_t i = 0; i < pixel_count; i++)
    		 {
    		     colorRGB.r=PWM_TABLE[10];colorRGB.g=PWM_TABLE[10];colorRGB.b=0;
    		     pixels[i] = colorRGB;
    		     WS2812B_setLeds(pixels,i);
    		     vTaskDelay(500/portTICK_PERIOD_MS);
    		 }
    		 for (uint8_t i = 0; i < pixel_count; i++)
    		 {
    		     colorRGB.r=0;colorRGB.g=PWM_TABLE[10];colorRGB.b=PWM_TABLE[10];
    		     pixels[i] = colorRGB;
    		     WS2812B_setLeds(pixels,i);
    		     vTaskDelay(500/portTICK_PERIOD_MS);
    		 }
    		 for (uint8_t i = 0; i < pixel_count; i++)
    		 {
    		     colorRGB.r=160;colorRGB.g=32;colorRGB.b=240;
    		     pixels[i] = colorRGB;
    		     WS2812B_setLeds(pixels,i);
    		     vTaskDelay(500/portTICK_PERIOD_MS);
    		 }

    	}
#endif
	}
}

#endif  //end of  ESPWS2812 //(0)



static void socket_deinit()
{
   // close(g_iSock_fd);
	if(g_iSock_fd>=0)
    closesocket(g_iSock_fd);
    g_iSock_fd = g_iSock_fd -1;
    port++;
}


static void socket_init()
{
#if 1
//    struct addrinfo *res;
    int err;
    struct sockaddr_in saddr;// = { 0 };

 //   tcpip_adapter_init();

   // err = getaddrinfo("localhost", "80", &hints, &res);

//    if (err != 0 || res == NULL) {
//        printf("DNS lookup failed: %d", errno);
//        return;
//    }

SOCKBEGIN:
	if(g_iSock_fd>=0)
	{
		do
		{
			socket_deinit();
		}while(g_iSock_fd>=0);
	}

do{
    g_iSock_fd =  socket(AF_INET, SOCK_STREAM, 0);//socket(res->ai_family, res->ai_socktype, 0);

    if (g_iSock_fd < 0)
    {
    	printf( "Failed to allocate socket %d.\r\n",errno);//Failed to allocate socket 23.
    	socket_deinit();
      //  freeaddrinfo(res);
        vTaskDelay(500 /portTICK_RATE_MS);
        //goto SOCKBEGIN;
        //return;
    }
}while(g_iSock_fd < 0);

#if 1 //bind local port
do{
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);//htons(8086);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(g_iSock_fd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in));
    if (err < 0)
    {
    	printf("Failed to bind socket %d\r\n",errno); //Failed to bind socket 98
        //freeaddrinfo(res);
        socket_deinit();
        vTaskDelay(500 /portTICK_RATE_MS);
        //goto SOCKBEGIN;
        //return;
    }
}while(err < 0);
#endif

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr =inet_addr(SERVER_IP);
    saddr.sin_port = htons(REMOTE_PORT);

do{
	err=connect(g_iSock_fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
    if (err!= 0)
    {// err=connect(g_iSock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    	printf("Socket connection failed: %d\r\n", errno);//Socket connection failed: 104
    	socket_deinit();
      //  freeaddrinfo(res);

        vTaskDelay(500 /portTICK_RATE_MS);
        goto SOCKBEGIN;
        //return;
    }
}while(err!=0);
//    freeaddrinfo(res);
#endif
    printf("connect server success %d/%d....\r\n",err,g_iSock_fd);//0 0

}



#if(ESPWS2812==1)
/**
void rainbow(void *pvParameters)
{
  const uint8_t anim_step = 10;
  const uint8_t anim_max = 255;//oringin is 250 change to 255 by Charlin
//  const uint32_t pixel_count = 66; // Number of your "pixels"
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
    //printf("free mem=%d \r\n", esp_get_free_heap_size());
    delay_ms(delayTime);
    //vTaskDelete(xHandleXDisplay);
  }
}
**/
#endif

void user_make_thread(char *xBuffer, uint32_t xLength)
{
	//if(xBuffer[0]!='{') ;//return;  //cjson   &&data_buffer[length]!='}'如果有空格或者回车

	cJSON * root = NULL;
	cJSON * item = NULL;//cjson对象

	root = cJSON_Parse(xBuffer);

	if (!root)
	{
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
	}
	else
	{
		char *out = cJSON_PrintUnformatted(root);
		printf("%s\n", "wu格式的方式print Json:");
		printf("%s\n\n",out);
		if(strcmp(out,"{\"status\":\"ok\"}")==0)
		{
			if(out)	free(out);
			if(root)cJSON_Delete(root);
			return;
		}
		if(out)	free(out);
		//printf("%s\n", "无格式方式打印json：");
		//printf("%s\n\n", cJSON_PrintUnformatted(root));//Memory Leak

		   //printf("%s\n", "获取pid下的cjson对象");
		item = cJSON_GetObjectItem(root, "pid");
		if(item!=NULL&&item->type==cJSON_Number)
			printf("pid %d\n", item->valueint);

		item = cJSON_GetObjectItem(root, "type");
		if(item!=NULL&&(item->type == cJSON_String))
		{
			//printf("type:%s\n", cJSON_Print(item));//"WIFI1" //memory leak
			if(strcmp(item->valuestring,"APP2")==0)
			{
				printf("\r\nAPP Begin to Control WiFi!\r\n");

			}
			else if(strcmp(item->valuestring,"APP0")==0)
			{
				printf("\r\nAPP logined!\r\n");
			}
		}

//					   printf("%s\n", "获取swVer下的cjson对象");
//					   item = cJSON_GetObjectItem(root, "swVer");
//						  if(item==NULL);
//						  else
//					   printf("swVer:%s\n", cJSON_Print(item));//memory leak


		item = cJSON_GetObjectItem(root, "srcMac");
		if(item!=NULL&&(item->type == cJSON_String))
		{
			 printf("%s:", item->valuestring);
		}
		item = cJSON_GetObjectItem(root , "dstMac");
		if(item!=NULL&&(item->type == cJSON_String))
		{
			 printf("%s:", item->valuestring);
		}
		item = cJSON_GetObjectItem(root , "fan");//and 手动模式
		if(item!=NULL&&(item->type == cJSON_Number))
		{

			 printf("fan %d:", item->valueint);
		}



	item = cJSON_GetObjectItem(root, "red");
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		colorR=item->valueint;
		printf("red %d\n", colorR);
	}
	item = cJSON_GetObjectItem(root, "green");
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		colorG=item->valueint;
		printf("green %d\n", colorG);
	}
	item = cJSON_GetObjectItem(root, "blue");
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		colorB=item->valueint;
		printf("blue %d\n", colorB);
	}

#if 0
	item = cJSON_GetObjectItem(root, "illum");//lx read only
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		printf("illum %d\n", item->valueint);
	}
#endif

	item = cJSON_GetObjectItem(root, "bright");
	if(item!=NULL && (item->type == cJSON_Number))
	{
		brightValue=item->valueint;
		switch(brightValue)
		{
			case 0:
			break;
			case 10:
			break;
			case 20:
			break;
			case 30:
			break;
			case 40:
			break;
			case 50:
			break;
			case 60:
			break;
			case 70:
			break;
			case 80:
			break;
			case 90:
			break;
			case 100:
			break;
			default:
				break;

		}
		//colorR=colorR*brightValue/100;
		//colorG=colorG*brightValue/100;
		//colorB=colorB*brightValue/100;
		printf("	R%d G%d B%d\n", colorR,colorG,colorB);
	}
	item = cJSON_GetObjectItem(root, "led"); //led switch on/off
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		printf("led %d\n", item->valueint);
		switch(item->valueint)
		{
			case 0:
				boolValue &= ((~BIT0)&0xFF);
#if(ESPWS2812==1)
				colorRGB.r=0;colorRGB.g= 0;colorRGB.b=0;
				//colorRGB=makeRGBVal(0, 0, 0);
				//ws2812_reset(WS2812_PIN);//turn off
#endif
				break;
			case 1:
				boolValue |= ((BIT0)&0x01);//turn on
#if(ESPWS2812==1)
				//colorRGB=makeRGBVal((colorR*brightValue/100), (colorG*brightValue/100), (colorB*brightValue/100));
				colorRGB.r=(colorR*brightValue/100);colorRGB.g= (colorG*brightValue/100);colorRGB.b=(colorB*brightValue/100);
#endif
				break;
			default:break;
		}
	}
#if(ESPWS2812==1)

	for(int i=0;i<pixel_count;i++)
	{
		pixels[i] = colorRGB;
	}
	//ws2812_setColors(pixel_count,  pixels);
	WS2812B_setLeds(pixels, pixel_count);
#endif
	item = cJSON_GetObjectItem(root, "ledMode");
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		ledMode=item->valueint;
		printf("ledMode %d\n", ledMode);
		if(ledMode==1)
		{
			if(xHandleLedDisplay!=NULL)
				vTaskDelete(xHandleLedDisplay);//should delete task
		}
		else
		{
			if(xHandleLedDisplay==NULL)
				xTaskCreate(rgbDisplay, "rgbDisplay", 4096, NULL, 7, &xHandleLedDisplay);//create a task or timer to do it
		}
		/**
		switch(ledMode)
		{
			case 0://handle control
				//
			break;
			case 1:
				//ledMode=1;

			break;
			case 2://breath

			break;
			case 3://flow led

			break;
			case 4:

			break;
			case 5:
#if(ESPWS2812==1)
//			if(xHandleXDisplay==NULL)
//				xTaskCreate(rainbow, "ws2812 rainbow demo", 4096, NULL, 10, &xHandleXDisplay);
#endif
			break;
			case 6:
#if(ESPWS2812==1)
				if(xHandleXDisplay!=NULL)
				vTaskDelete(xHandleXDisplay);
#endif
			break;

			default:break;
		}
		**/
	}
	item = cJSON_GetObjectItem(root, "temp");
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		setTemp = item->valueint;
		printf("temp %d\n", setTemp);
	}

	item = cJSON_GetObjectItem(root, "humidity");//0~100
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		setHumidity = item->valueint;
		printf("humidity %d\n", setHumidity);
	}
	item = cJSON_GetObjectItem(root, "fan");//on and off or pwm?
	if(item!=NULL&&(item->type == cJSON_Number))//and 手动控制模式
	{
		printf("fan %d\n", item->valueint);
		switch(item->valueint)
		{
			case 0:
				boolValue &= ((~BIT1)&0xFF);//turn off fan
				open_relay_0();
				break;

			case 1:
				boolValue |= ((BIT1)&0x02);//turn on fan
				close_relay_0();
				break;
			default:break;

		}
	}
	item = cJSON_GetObjectItem(root, "pump");
	if(item!=NULL&&(item->type == cJSON_Number)) //when in  手动控制模式
	{
		printf("pump %d\n", item->valueint);
		switch(item->valueint)
		{
			case 0:
				boolValue &= ((~BIT2)&0xFF);//turn off pump
				close_relay_1();
				break;
			case 1:
				boolValue |= ((BIT2)&0x04);//turn on pump
				open_relay_1();
				break;
			default:break;
		}
	}
	item = cJSON_GetObjectItem(root, "sound");
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		printf("sound %d\n", item->valueint);
		switch(item->valueint)
		{
			case 0:
				boolValue &= ((~BIT3)&0xFF);//turn off

				break;
			case 1:
				boolValue |= ((BIT3)&0x08);//turn on

				break;
			default:break;
		}
	}
	item = cJSON_GetObjectItem(root, "live");
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		printf("live %d\n", item->valueint);
		switch(item->valueint)
		{
			case 0:
				boolValue &= ((~BIT4)&0xFF);//turn off
				break;
			case 1:
				boolValue |= ((BIT4)&0x10);//turn on
				break;
			default:break;
		}
	}
	item = cJSON_GetObjectItem(root, "nurseMode");//护理模式
	if(item!=NULL&&(item->type == cJSON_Number))
	{
		nurseMode=item->valueint;
		printf("nurseMode %d\n", nurseMode);
		switch(nurseMode)
		{
			case manual://manual

			break;
			case semiAutomatic:// semi-automatic

			break;
			case automatic://automatic //should run in a task

			break;
			case customized:

			break;
			case aiMode:

			break;
			case 5:

			break;
			default:break;
		}
	}
	/**
	else if (item->type == cJSON_String)
	{

		if(strcmp(item->valuestring,"0")==0)
			nurseMode=manual;
		else if(strcmp(item->valuestring,"1")==0)
			nurseMode=semiAutomatic;
		else if(strcmp(item->valuestring,"2")==0)
			nurseMode=automatic;
		else if(strcmp(item->valuestring,"3")==0)
			nurseMode=customized;
		else if(strcmp(item->valuestring,"4")==0)
			nurseMode=aiMode;
		else
			nurseMode=undefined;
		printf("nurseMode %s==%d\n", item->valuestring, nurseMode);
	}
	 */
//		if(root)
//		{
//			cJSON_Delete(root);
//		}
#if 0
		if(item)
		{
			//printf("item %s\n", cJSON_Print(item));//memory leak
			 if(item->type==cJSON_String)
				 printf("item %s\n", item->valuestring);
			 else if(item->type==cJSON_Number)
				 printf("item %d\n", item->valueint);
			 //free(item);
		}
#endif
//		int ret=send(g_iSock_fd, "{\"status\":\"ok\"}", strlen("{\"status\":\"ok\"}"), 0);
		//if(ret<0)
		{
	    	//socket_deinit();
	    	//printf("Socket-responce-error %d\r\n",errno);
		}
	}
	if(root)
	{
		cJSON_Delete(root);
	}
}





//creak socket and rev task
static void tcp_cli_task(void *pvParameters)
{
	char data_buffer[512];

	int length;
	int err;

#if(ESPWS2812==1)
	pixels = malloc(sizeof(wsRGB_t) * pixel_count);//free(pixels)
	//ws2812_init(WS2812_PIN);
	WS2812B_init(RMT_TX_CHANNEL,WS2812_PIN,pixel_count);
#endif

NEWBEGIN:
	socket_init();//socket init
//	err=send(g_iSock_fd, "\"type\":\"WIFI0\",\"srcMac\":\"A4-E9-75-3D-DC-D0\"", strlen("\"type\":\"WIFI0\",\"srcMac\":\"A4-E9-75-3D-DC-D0\""), 0);
//    if(err<0)
//    {
//    	goto NEWBEGIN;
//    }
    while(1)
	{
    	int s;
    	fd_set rfds;
    	struct timeval tv =
    	{
    			.tv_sec = 2,
				.tv_usec = 0,
    	};

    	FD_ZERO(&rfds);
    	FD_SET(g_iSock_fd, &rfds);

    	s = select((g_iSock_fd + 1), &rfds, NULL, NULL, &tv);

    	if (s < 0)
    	{
    		printf("Select failed: errno %d\r\n", errno);
    	}
    	else if (s == 0)
    	{
    		//printf("Timeout has been reached and nothing has been received\r\n");
    	}
    	else
    	{
        	if (FD_ISSET(g_iSock_fd, &rfds))
			{
				if ((length = recv(g_iSock_fd, data_buffer, sizeof(data_buffer)-1,0)) > 0)
				{
					//data_buffer[length] = '\0';
					//printf("%d bytes were received through: %s", length,data_buffer);
					user_make_thread(data_buffer, length);
					memset(data_buffer,0,length);
				}
				else
				{

					printf("rcv failed: errno %d\r\n", errno);
					vTaskDelay(1000 / portTICK_RATE_MS);
					goto NEWBEGIN;
					//vTaskDelete(NULL);
					//socket_init();
				}
			}
    	}
    	if(nurseMode==automatic)
    	{
#if 1
			if(getTemp>setTempMax)
			{
					//status error TempMax
			}
			else if(getTemp<setTempMin)
			{
					//status error TempMin

			}
			if(setTemp>getTemp)  //设置温度大于实际温度  需要加热 关闭风扇
			{
				//boolValue |= ((BIT1)&0x02);//open heater
				boolValue &= ((~BIT1)&0xFF);//
				close_relay_0();//close cooler/fan
			}
			else
			{
				//boolValue &= ((~BIT1)&0xFF);//turn off//close heater
				boolValue |= ((BIT1)&0x02);//
				open_relay_0();//open cooler/fan
			}

			if(setHumidity>getHumidity)  //设置湿度大于实际湿度 打开水泵
			{
	//open water pump
				boolValue |= ((BIT2)&0x04);//turn on pump
				open_relay_1();
			}
			else
			{
	//close water pump
				boolValue &= ((~BIT2)&0xFF);//turn off pump
				close_relay_1();
			}
			if(getIllum>100)
			{
				brightValue-=10;
			}
			else if (getIllum<100)
			{
				brightValue+=10;
			}
#endif
    	}

	}
}


//create and send
static void tcp_send_task(void *pvParameters)
{
//	uint32_t esp_timer_count =system_get_time();
    //esp_timer_count=system_get_time();
	int ret=0;
	int Num=0;

	//socket_init();

	cJSON * root =  cJSON_CreateObject();//NULL
//    cJSON * item =  cJSON_CreateObject();
//    cJSON * next =  cJSON_CreateObject();

	cJSON_AddItemToObject(root, "auth", cJSON_CreateString("TDP10"));
    cJSON_AddItemToObject(root, "pid", cJSON_CreateNumber(Num));//根节点下添加  或者cJSON_AddNumberToObject(root, "pid",Num);
    Num=1;
    cJSON_AddItemToObject(root, "tid", cJSON_CreateNumber(Num));
    cJSON_AddItemToObject(root, "type", cJSON_CreateString("WIFI0"));//或者  cJSON_AddStringToObject(root, "type", "WIFI0");
	cJSON_AddItemToObject(root, "swVer", cJSON_CreateString("1.0"));
	cJSON_AddItemToObject(root, "hwVer", cJSON_CreateString("1.0"));

   // cJSON_AddItemToObject(root, "Mac", item);//root节点下添加节点MAC
	//char *pStr=strMac;

    cJSON_AddItemToObject(root, "srcMac",cJSON_CreateString(srcMAC));//strMac
	cJSON_AddItemToObject(root, "dstMac",cJSON_CreateString("04-12-56-7E-2A-38"));

    // 打印JSON数据包
    char *out = cJSON_PrintUnformatted(root);//cJSON_Print(root);//
    printf("%s\n", out);
    ret=send(g_iSock_fd, out, strlen(out), 0);//canot use sizeof(cJSON_Print(root))
    //int ret=write(g_iSock_fd, cJSON_Print(root), strlen(cJSON_Print(root)));
    if(ret<0)
    {
    	socket_deinit();
    	printf("Socket send error %d\r\n",errno);
    }
    if(root)
		cJSON_Delete(root); // 释放内存
	if(out)
		free(out);

//	ESP_LOGI(TAG,"get free size of 32BIT heap : %d\n",heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
//	ESP_LOGI(TAG,"get free size of 8BIT heap : %d\n",heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

    free32=xPortGetFreeHeapSizeCaps( MALLOC_CAP_32BIT );////heap_caps_get_largest_free_block(MALLOC_CAP_32BIT);
    free8=xPortGetFreeHeapSizeCaps( MALLOC_CAP_8BIT );//heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    free8start=xPortGetMinimumEverFreeHeapSizeCaps(MALLOC_CAP_8BIT);//heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
    free32start=xPortGetMinimumEverFreeHeapSizeCaps(MALLOC_CAP_32BIT);//heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT);
//    ESP_LOGI(TAG, "Free heap: %u", xPortGetFreeHeapSize());
    ESP_LOGI(TAG, "Free (largest free blocks) 8bit-capable memory : %d, 32-bit capable memory %d\n", free8, free32);
    ESP_LOGI(TAG, "Free (min free size) 8bit-capable memory : %d, 32-bit capable memory %d\n", free8start, free32start);

//    ESP_LOGI(TAG,"get free size of 32BIT heap : %d\n",xPortGetFreeHeapSizeCaps(MALLOC_CAP_32BIT));
    while(1)
    {

		cJSON * root =  cJSON_CreateObject();//NULL
	    //cJSON * item =  cJSON_CreateObject();

		Num=1;
		//cJSON_AddItemToObject(root, "AUTH", cJSON_CreateString("TDP10"));
	    cJSON_AddItemToObject(root, "pid", cJSON_CreateNumber(Num));//根节点下添加  或者cJSON_AddNumberToObject(root, "rc",Num);
	    Num=1;
	    cJSON_AddItemToObject(root, "tid", cJSON_CreateNumber(Num++));
	    cJSON_AddItemToObject(root, "type", cJSON_CreateString("WIFI1"));//或者  cJSON_AddStringToObject(root, "operation", "CALL");
		//cJSON_AddItemToObject(root, "swVer", cJSON_CreateString("1.0"));
		//cJSON_AddItemToObject(root, "hwVER", cJSON_CreateString("1.0"));

	    cJSON_AddItemToObject(root, "srcMac",cJSON_CreateString(srcMAC));//strMac
//    	cJSON_AddItemToObject(root, "dstMac",cJSON_CreateString("04-12-56-7E-2A-38"));

#if(ESPDHT11==1)
	    setDHTPin(DHT_GPIO);
	    if(ReadDHT11(dhtData))  //get temprature
	    {
	    	uint8_t outstr[10];
	    	DHT11_NumToString(dhtData[0],outstr);
	    	//getHumidity=dhtData[2];
	    	printf("Relative Humidity   :%s%%\r\n",outstr);

	    	DHT11_NumToString(dhtData[1],outstr);

	    	DHT11_NumToString(dhtData[2],outstr);
	    	//getTemp=dhtData[2];
	    	printf("Current Temperature :%sC\r\n",outstr);

	    	DHT11_NumToString(dhtData[3],outstr);

	    	getTemp=dhtData[2];
	    	cJSON_AddItemToObject(root, "temp", cJSON_CreateNumber(dhtData[2]));
	    	getHumidity=dhtData[0];
	    	cJSON_AddItemToObject(root, "humidity", cJSON_CreateNumber(dhtData[0]));
	    }
	    else
	    {
	    	printf("--Read DHT11 Error!--\n");
	    }

#endif

#if(ESPDS18B20==1)
		if(ds18b20_init(DS_PIN))
		{
			uint8_t sn[8];
			ds18b20_getROM(sn);
			printf("ROM:");
			for(int i=0;i<8;i++)
			{
				printf("%.2X ",sn[i]);  //ROM:28 D0 CA 5B 06 00 00 D8  //ROM:28 FF 8E 4D A2 16 05 9C
			}
			printf("\n");
			float temp=ds18b20_get_temp();
			if(temp>-55&&temp<125)
			{
				getTemp=temp;
				printf("Temperature: %0.1fC\n",temp);
				cJSON_AddItemToObject(root, "temp", cJSON_CreateNumber((int)temp+((int)(temp*10)%10)*0.1));//保留1位小数  //((float)((int)((temp+0.05)*10)))/10
			}
			else
			{
				printf("Error Temperature: %0.1f\n",temp);
			}
		}
		else
		{
			printf("read Temperature fail\n");
		}
#endif
#if (capHumidity==1)
		if(capacity>0)
		cJSON_AddItemToObject(root, "humidity", cJSON_CreateNumber(capacity));
#endif
#if (ADCENABLE==1)
	uint16_t adc_read=0.;
	// initialize ADC
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1_TEST_CHANNEL, ADC_ATTEN_11db);
	adc_read= ((adc1_get_voltage(ADC1_TEST_CHANNEL)));//3.3V  ~  2^12=4096
	printf("adc %d\n",adc_read);
	//cJSON_AddItemToObject(root, "humidity", cJSON_CreateNumber(adc_read));
	uint8_t rh=(100-((adc_read*100)/4096));
	if((rh>0) && (rh<100))
	{
		getHumidity=rh;
		cJSON_AddItemToObject(root, "humidity", cJSON_CreateNumber(rh));
		printf("rh %d\n",rh);

	}

#endif
#if(ESPBH1750==1)
		if(Init_BH1750(I2C_SDA, I2C_SCL)==0)
		{

			getIllum=Read_BH1750();
			if(getIllum!=NULL)
			{
				printf("--Ambient Light[%d]==%0.2flx\r\n",getIllum,(float)getIllum/1.2);
				cJSON_AddItemToObject(root, "illum", cJSON_CreateNumber(getIllum));
			}

		}
		else
		{
			printf("--Read BH1750 Error!--\n");
		}
#endif

	    Num=28;

	    cJSON_AddItemToObject(root, "waterLevel", cJSON_CreateNumber(90));
	    //cJSON_AddItemToObject(root, "illum", cJSON_CreateNumber(65535));

	    cJSON_AddItemToObject(root, "red", cJSON_CreateNumber(colorR));
	    cJSON_AddItemToObject(root, "green", cJSON_CreateNumber(colorG));
	    cJSON_AddItemToObject(root, "blue", cJSON_CreateNumber(colorB));
	    cJSON_AddItemToObject(root, "bright", cJSON_CreateNumber(brightValue));

	    cJSON_AddItemToObject(root, "led", cJSON_CreateNumber((boolValue>>0)&0x01));
	    cJSON_AddItemToObject(root, "ledMode", cJSON_CreateNumber(ledMode));
	    cJSON_AddItemToObject(root, "nurseMode", cJSON_CreateNumber(nurseMode));
	    cJSON_AddItemToObject(root, "fan", cJSON_CreateNumber((boolValue>>1)&0x01));
	    cJSON_AddItemToObject(root, "pump", cJSON_CreateNumber((boolValue>>2)&0x01));
	    cJSON_AddItemToObject(root, "sound", cJSON_CreateNumber((boolValue>>3)&0x01));
	    cJSON_AddItemToObject(root, "live", cJSON_CreateNumber((boolValue>>4)&0x01));

	    char *out = cJSON_Print(root);
	    printf("%s\n", out);	// 打印JSON数据包
	    ret=send(g_iSock_fd, out, strlen(out), 0);//canot use sizeof(cJSON_Print(root))  //memory leak
		if(root)
		{
			cJSON_Delete(root);
		}
		if(out)
			free(out);
	    if(ret<0)
	    {
	    	//socket_deinit();
	    	printf("--send Error!--\n");
	    	socket_init();
	    }

//		ESP_LOGI(TAG,"get free size of 32BIT heap : %d\n",heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
//		ESP_LOGI(TAG,"get free size of 8BIT heap : %d\n",heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
#if 1
	    free32=xPortGetFreeHeapSizeCaps( MALLOC_CAP_32BIT );////heap_caps_get_largest_free_block(MALLOC_CAP_32BIT);
	    free8=xPortGetFreeHeapSizeCaps( MALLOC_CAP_8BIT );//heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
	    free8start=xPortGetMinimumEverFreeHeapSizeCaps(MALLOC_CAP_8BIT);//heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
	    free32start=xPortGetMinimumEverFreeHeapSizeCaps(MALLOC_CAP_32BIT);//heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT);
	    ESP_LOGI(TAG, "#Free heap: %u", xPortGetFreeHeapSize());
	    ESP_LOGI(TAG, "#Free (largest free blocks) 8bit-capable memory : %d, 32-bit capable memory %d\n", free8, free32);
	    ESP_LOGI(TAG, "#Free (min free size) 8bit-capable memory : %d, 32-bit capable memory %d\n", free8start, free32start);

//	    ESP_LOGI(TAG,"#get free size of 32BIT heap : %d\n",xPortGetFreeHeapSizeCaps(MALLOC_CAP_32BIT));
#endif
	    vTaskDelay(10000 / portTICK_RATE_MS);//10s
    }
}

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static ip4_addr_t s_ip_addr;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            s_ip_addr = event->event_info.got_ip.ip_info.ip;

           // xTaskCreate(&tcp_server_task, "tcp_server_task", 4096, NULL, 5, NULL);//tcp server task
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
             auto-reassociate. */
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        case  SYSTEM_EVENT_AP_STACONNECTED:
        	ESP_LOGI(TAG, "get Connected");

        	break;
        case      SYSTEM_EVENT_AP_STADISCONNECTED:

        	break;

        default:
            break;
    }
    return ESP_OK;
}

static void init_wifi_sta(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    //printf("CONFIG_WIFI_PASSWORD IS %s\n",WIFI_PASSWORD);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    //ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_NONE) );
    ESP_LOGI(TAG, "Connecting to \"%s\"", wifi_config.sta.ssid);
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected");
}

/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/

#define BUF_SIZE 2*1024*1024  //2048KB
//----------------
void spiram_test_task(void *pvParameter)
{
    uint32_t t1, t2, t3, t4;
    uint8_t *buf1;
    uint8_t *buf2;
    uint8_t *pbuf1;
    uint8_t *pbuf2;
    uint8_t testb=0x5A;
    uint32_t idx;
    int pass = 0;
    while (1)  {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        pass++;
        if (pass > 9) break;

        if (pass % 2) testb = 0xA5;
        else testb = 0x5A;

        printf("\n======= PSRAM Test (%u bytes block) pass %d =======\n", BUF_SIZE, pass);

        buf1 = (uint8_t *)0x3f800000;
        buf2 = (uint8_t *)0x3f800000+BUF_SIZE;

        t1 = clock();
        memset(buf1, testb, 2048);//BUF_SIZE
        memset(buf2, testb, BUF_SIZE);
        t1 = clock() - t1;
        printf("--\n");
        for (idx=0; idx<2048;idx++) {
            printf("%02X ", buf1[idx]);
        }
        printf("--\n");
        t2 = clock();
        int res = memcmp(buf1, buf2, BUF_SIZE);
        t2 = clock() - t2;

        pbuf1 = buf1;
        pbuf2 = buf2;
        t4 = clock();
        for (idx=0; idx < BUF_SIZE; idx++) {
            *pbuf1++ = testb;
        }
        for (idx=0; idx < BUF_SIZE; idx++) {
            *pbuf2++ = testb;
        }
        t4 = clock() - t4;

        pbuf1 = buf1;
        pbuf2 = buf1;
        t3 = clock();
        for (idx=0; idx < BUF_SIZE; idx++) {
            if (*pbuf1 != *pbuf2) break;
            pbuf1++;
            pbuf2++;
        }
        t3 = clock() - t3;

        float bs = ((1000.0 / (float)t1 * (float)(BUF_SIZE*2))) / 1048576.0;
        printf("               memset time: %u ms; %f MB/sec\n", t1, bs);
        bs = ((1000.0 / (float)t2 * (float)(BUF_SIZE))) / 1048576.0;
        if (res == 0) printf("               memcmp time: %u ms; %f Mcompares/sec\n", t2, bs);
        else printf("               memcmp time: %u ms; FAILED (%d)\n", t2, res);

        bs = ((1000.0 / (float)t4 * (float)(BUF_SIZE*2))) / 1048576.0;
        printf("   Memory set in loop time: %u ms; %f MB/sec\n", t4, bs);
        bs = ((1000.0 / (float)t3 * (float)(BUF_SIZE))) / 1048576.0;
        printf("  Compare in loop time idx: %u ms; %f Mcompares/sec (%u of %u OK)\n", t4, bs, idx, BUF_SIZE);
        printf("      1st 16 bytes of buf1: ");
        for (idx=0; idx<16;idx++) {
            printf("%02X ", buf1[idx]);
        }
        printf("\n");
        printf("      1st 16 bytes of buf2: ");
        for (idx=0; idx<16;idx++) {
            printf("%02X ", buf2[idx]);
        }
        printf("\n");
    }
}


void app_main()
{
//2048
	//xTaskCreate(&spiram_test_task, "spiram_test_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
	//	xTaskCreate(&spiram_test_task, "spiram_test_task", 2048, NULL, 5, NULL);
	gpioInit();

    currFbPtr = (volatile uint32_t)pvPortMallocCaps(320*240*2, MALLOC_CAP_32BIT);

    ESP_LOGI(TAG,"%s=%p\n",currFbPtr == NULL ? "------currFbPtr is NULL------" : "==========currFbPtr not NULL======",currFbPtr );

    ESP_LOGI(TAG,"get free size of 32BIT heap : %d\n",xPortGetFreeHeapSizeCaps(MALLOC_CAP_32BIT));

    ESP_LOGI(TAG,"Starting nvs_flash_init ...");
    nvs_flash_init();

    //vTaskDelay(3000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "Free heap: %u", xPortGetFreeHeapSize());

#if BTENABLE	//	BTENABLE
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
#endif //enf of BTENABLE

	vTaskDelay(3000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "Wifi Initialized...");
    init_wifi_sta();
    //init_wifi_ap();

    vTaskDelay(2000 / portTICK_RATE_MS);

    //char macFormat[] = "%02X-%02X-%02X-%02X-%02X-%02X";

    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
        esp_wifi_get_mac(ESP_IF_WIFI_STA,wifiMac);

    sprintf(strMac, "%02X-%02X-%02X-%02X-%02X-%02X", wifiMac[0], wifiMac[1], wifiMac[2], wifiMac[3], wifiMac[4], wifiMac[5]);
    printf("srcMAC: %s\n",strMac);


	xTaskCreate(&tcp_cli_task, "tcp_cli_task", 4096, NULL, 9, NULL);//TCP Client create and rcv task
	vTaskDelay(5000 / portTICK_RATE_MS);
	xTaskCreate(&tcp_send_task, "tcp_send_task", 2048, NULL, 6, NULL);//TCP Client Send Task

#if (capHumidity==1)
	//gpio
    gpio_config_t io_conf;

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO0/33 here
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
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 7, NULL);

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
#endif
}
