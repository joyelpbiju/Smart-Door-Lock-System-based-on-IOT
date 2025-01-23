// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>

// Input/output
#include <stdio.h>


// PWM library
#include <bl_pwm.h>

// Define SERVO pins
#define SER_R_PIN 17
#define SER_R_PWM_CHANNEL 2

// frequency
#define PWM_FREQUENCY 6400
#define PWM_FREQUENCY_DIVIDER 128
float dc;
/* SERVO task */
void task_ser_pwm(void *pvParameters)
{
    printf("SERVO PWM task started\r\n");

    // Initialize PWM for the servo motor
    bl_pwm_init(SER_R_PWM_CHANNEL, SER_R_PIN, PWM_FREQUENCY);

    // Set frequency to 50Hz: set 128 as divider value -> real frequency = 6400/128 = 50Hz
    PWM_Channel_Set_Div(SER_R_PWM_CHANNEL, PWM_FREQUENCY_DIVIDER);

    // Start PWM operations
    bl_pwm_start(SER_R_PWM_CHANNEL);

    // Initial duty cycle: 5% (corresponds to 0° position)
    dc = 5.0;

    // Set initial position to 5% duty cycle
    printf("[DEBUG] Setting Duty Cycle to %f%% (0° position)\n", dc);
    bl_pwm_set_duty(SER_R_PWM_CHANNEL, dc);

    vTaskDelay(1000 / portTICK_PERIOD_MS); // Small delay for stabilization

    // Move to 7.5% duty cycle (90° position)
    dc = 10;
    printf("[DEBUG] Moving to Duty Cycle: %f%% (90° position)\n", dc);
    bl_pwm_set_duty(SER_R_PWM_CHANNEL, dc);

    // Hold the position for 3 seconds
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    // Move back to 5% duty cycle (0° position)
    dc = 5.0;
    printf("[DEBUG] Returning to Duty Cycle: %f%% (0° position)\n", dc);
    bl_pwm_set_duty(SER_R_PWM_CHANNEL, dc);

    // End of task (optional: keep this task running in a loop if needed)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}

const char* get_door_status(){
    const char* DOOR_STATUS;
    if(dc == 10.0){
        DOOR_STATUS="Door Open";
    }
    else{
        DOOR_STATUS="Door Closed";
    }
    return DOOR_STATUS;
}
