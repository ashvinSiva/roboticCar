#include <stdio.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "ultrasonic.h"

// Defining Motor Pins based on Motor Driver Pins
#define LM1 GPIO_NUM_27
#define LM2 GPIO_NUM_26
#define RM3 GPIO_NUM_13
#define RM4 GPIO_NUM_14

#define TRIG_PIN GPIO_NUM_33
#define ECHO_PIN GPIO_NUM_34
#define MAX_DIST 2000
ultrasonic_sensor_t sensor = { 
        .trigger_pin = TRIG_PIN, 
        .echo_pin = ECHO_PIN
    };

void init_motor() {
    gpio_reset_pin(LM1); 
    gpio_set_direction(LM1, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LM2); 
    gpio_set_direction(LM2, GPIO_MODE_OUTPUT);
    gpio_reset_pin(RM3); 
    gpio_set_direction(RM3, GPIO_MODE_OUTPUT);
    gpio_reset_pin(RM4); 
    gpio_set_direction(RM4, GPIO_MODE_OUTPUT);
}

void drive() {
    gpio_set_level(LM1, 1);
    gpio_set_level(LM2, 0); 
    gpio_set_level(RM3, 1); 
    gpio_set_level(RM4, 0); 
}

void stop() {
    gpio_set_level(LM1, 0);
    gpio_set_level(LM2, 0); 
    gpio_set_level(RM3, 0); 
    gpio_set_level(RM4, 0);
    vTaskDelay( pdMS_TO_TICKS(100) );
}

void reverse() {
    gpio_set_level(LM1, 0);
    gpio_set_level(LM2, 1); 
    gpio_set_level(RM3, 0); 
    gpio_set_level(RM4, 1); 
    vTaskDelay( pdMS_TO_TICKS(1500) );
    stop();
}

void spin_right() {
    gpio_set_level(LM1, 1);
    gpio_set_level(LM2, 0); 
    gpio_set_level(RM3, 0); 
    gpio_set_level(RM4, 1); 
    vTaskDelay( pdMS_TO_TICKS(1000) );
    stop();
}

void spin_left() {
    gpio_set_level(LM1, 0);
    gpio_set_level(LM2, 1); 
    gpio_set_level(RM3, 1); 
    gpio_set_level(RM4, 0); 
    vTaskDelay( pdMS_TO_TICKS(1000) );
    stop(); 
}


void getDistance() {
    
    float distance = 0.0f; 

    esp_err_t result = ultrasonic_measure(&sensor, MAX_DIST, &distance); 

    if (result != ESP_OK) {
        printf("Error %d: \n", result); 
        switch(result) {
            case ESP_ERR_ULTRASONIC_PING: 
                printf("Cannot ping (device is in invalid state)\n");
                break; 
            case ESP_ERR_ULTRASONIC_PING_TIMEOUT: 
                printf("Ping timeout (no device found)\n");
                break; 
            case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT: 
                printf("Echo timeout (i.e. distance is too large)\n"); 
                break; 
            default: 
                printf("%s\n", esp_err_to_name(result));
        }
    } else {
        printf("Distance: %0.2f cm\n", distance*100);
    } 

}

void app_main() {
   
    init_motor();  
    ultrasonic_init(&sensor);

    while(1) {

        getDistance(); 
        vTaskDelay( pdMS_TO_TICKS(1000) );
        
    }
}