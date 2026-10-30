#include <stdio.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

// Defining Motor Pins based on Motor Driver Pins
#define LM1 GPIO_NUM_27
#define LM2 GPIO_NUM_26
#define RM3 GPIO_NUM_13
#define RM4 GPIO_NUM_14

#define TRIG_PIN GPIO_NUM_36
#define ECHO_PIN GPIO_NUM_33

#define MAX_TIMEOUT_US 30000

#define OBSTACLE_DISTANCE_CM 10.0f


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

void init_USS() {

    // configure trigger pin as output on ESP32
    gpio_config_t config_trig = {
        .intr_type = GPIO_INTR_DISABLE, 
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << TRIG_PIN), 
        .pull_down_en = GPIO_PULLDOWN_DISABLE, 
        .pull_up_en = GPIO_PULLUP_DISABLE
    }; 

    gpio_config(&config_trig);

    // configure echo pin as input on ESP32
    gpio_config_t config_echo = {
        .intr_type = GPIO_INTR_DISABLE, 
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << ECHO_PIN), 
        .pull_down_en = GPIO_PULLDOWN_DISABLE, 
        .pull_up_en = GPIO_PULLUP_DISABLE
    };

    gpio_config(&config_echo);

    // Ensure trigger pin initially starts LOW
    gpio_set_level(TRIG_PIN, 0);
}

float getDistance() {
    
    // Initiate Trigger Pin to HIGH
    gpio_set_level(TRIG_PIN, 1); 
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN, 0); 

    // Wait for echo pin to go HIGH
    int64_t start_time = esp_timer_get_time(); 
    while(gpio_get_level(ECHO_PIN) == 0) {
        if ((esp_timer_get_time() - start_time) > MAX_TIMEOUT_US) {
            return -1.0f; 
        }
    }

    // Measure how long ECHO_PIN stays HIGH
    int64_t start_echo = esp_timer_get_time(); 
    while(gpio_get_level(ECHO_PIN) == 1) {
        if ((esp_timer_get_time() - start_echo) > MAX_TIMEOUT_US) {
            return -1.0f; 
        }
    }
    int64_t end_echo = esp_timer_get_time(); 

    int64_t delta_time = end_echo - start_echo; 
    return (delta_time*0.0343f)/2.0f;
}

void app_main() {
   
    init_motor(); 
    init_USS();

    while(1) {
        float distance = getDistance(); 

        while (distance > OBSTACLE_DISTANCE_CM) {
            drive(); 
            distance = getDistance();
            printf("Distance: %.2f cm\n", distance);
            vTaskDelay( pdMS_TO_TICKS(100) );
        }
        stop(); 
        reverse(); 
        spin_left(); 
        
        float distance_left = getDistance(); 
        if (distance_left > OBSTACLE_DISTANCE_CM) {
            continue;
        } else {
            spin_right(); 
            spin_right(); 
        }
        drive(); 
        vTaskDelay( pdMS_TO_TICKS(100) );
    }
}