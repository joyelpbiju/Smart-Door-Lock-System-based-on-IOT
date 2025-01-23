#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <bl_gpio.h>
#include <motion.h>
#include <led.h>

// Buffer to store motion events
#define MAX_EVENTS 2000
static char motion_events[MAX_EVENTS];
static int motion_index = 0;

// Task for motion detection and LED control
void task_pir_sensor(void *pvParameters) {
    printf("PIR sensor task started\r\n");

    // Initialize PIR sensor pin as input
    bl_gpio_enable_input(PIR_SENSOR_PIN, 1, 0);

    while (1) {
        // Read PIR sensor state
        uint8_t pir_state = bl_gpio_input_get_value(PIR_SENSOR_PIN);

        if (pir_state == 1) {
            // Motion detected: Turn on LEDs and log event
            printf("Motion detected\r\n");
            led_state.state_led_red = LED_ON;
            //led_state.state_led_green = LED_ON;
            //led_state.state_led_blue = LED_ON;

            // Log motion event
            if (motion_index < MAX_EVENTS - 2) {
                motion_events[motion_index++] = '1';
                motion_events[motion_index++] = ',';
            }
        } else {
            // No motion: Turn off LEDs and log event
            printf("No motion\r\n");
            led_state.state_led_red = LED_OFF;
            //led_state.state_led_green = LED_OFF;
            //led_state.state_led_blue = LED_OFF;

            // Log no-motion event
            if (motion_index < MAX_EVENTS - 2) {
                motion_events[motion_index++] = '0';
                motion_events[motion_index++] = ',';
            }
        }

        // Null-terminate the motion events buffer
        motion_events[motion_index] = '\0';

        // Apply the LED states
        apply_led_state();

        // Delay for 500ms
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    // Should never reach here
    vTaskDelete(NULL);
}

// Retrieve motion events log
char* get_motion_events() {
    printf("Returning motion event log: %s\r\n", motion_events);
    return motion_events;
}

// Clear motion events log
int clear_motion_events() {
    printf("Clearing motion events log\r\n");
    memset(motion_events, '\0', sizeof(motion_events));
    motion_index = 0;
    return 0; // Success
}

