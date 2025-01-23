#ifndef LED_DEFS_H
#define LED_DEFS_H

/* LED pins */
#define LED_R_PIN 17
//#define LED_G_PIN 14
//#define LED_B_PIN 11

#define LED_ON 1
#define LED_OFF 0

/* Data structure for LED states */
struct led_state {
    uint8_t state_led_red;
    uint8_t state_led_green;
    uint8_t state_led_blue;
} led_state;

/* Function to apply the current LED state */
void apply_led_state();

#endif // LED_DEFS_H
