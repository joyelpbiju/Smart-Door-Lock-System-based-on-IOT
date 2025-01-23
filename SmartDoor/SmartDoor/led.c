#include <stdio.h>
#include <bl_gpio.h>
#include <led.h>

void apply_led_state() {
    printf("Applying LED states: R: %d,\r\n",    //G: %d, B: %d
           led_state.state_led_red);
           //led_state.state_led_green,
           //led_state.state_led_blue);

    bl_gpio_output_set(LED_R_PIN, led_state.state_led_red);
    //bl_gpio_output_set(LED_G_PIN, led_state.state_led_green);
    //bl_gpio_output_set(LED_B_PIN, led_state.state_led_blue);
}
