#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include <bl_dma.h>
#include <bl_gpio.h>
#include <bl_irq.h>
#include <bl_sec.h>
#include <bl_sys_time.h>
#include <bl_uart.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <hal_hwtimer.h>

#include "rfid.h"
#include <blog.h>
#include <lwip/tcpip.h>
#include <motion.h>
#include <led.h>

extern void task_rfid(void *pvParameters);

extern uint8_t _heap_start;
extern uint8_t _heap_size;
extern uint8_t _heap_wifi_start;
extern uint8_t _heap_wifi_size;

static HeapRegion_t xHeapRegions[] = {
    {&_heap_start, (unsigned int)&_heap_size},
    {&_heap_wifi_start, (unsigned int)&_heap_wifi_size},
    {NULL, 0},
    {NULL, 0}
};

/* Task stack sizes */
#define RFID_STACK_SIZE 1024
#define WIFI_STACK_SIZE 1024
#define PIR_STACK_SIZE 512
#define HTTPD_STACK_SIZE 512

/* Main function, execution starts here */
void bfl_main(void)
{
    bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);  // Default baud rate 2,000,000
    printf("[DEBUG] UART0 initialized with default baud rate: 2,000,000\n");

    vPortDefineHeapRegions(xHeapRegions);

    // Initialize system components
    blog_init();
    bl_irq_init();
    bl_sec_init();
    bl_dma_init();
    hal_boot2_init();
    hal_board_cfg(0);

    // Initialize LEDs
    printf("[SYSTEM] Initializing LEDs...\r\n");
    bl_gpio_enable_output(LED_R_PIN, 0, 0);

    // Explicitly set initial LED states
    led_state.state_led_red = LED_OFF;
    led_state.state_led_green = LED_OFF;
    led_state.state_led_blue = LED_OFF;
    apply_led_state();
    printf("[SYSTEM] LED initialization complete.\r\n");

    // Task creation
    static StackType_t pir_stack[PIR_STACK_SIZE];
    static StaticTask_t pir_task;

    static StackType_t rfid_stack[RFID_STACK_SIZE];
    static StaticTask_t rfid_task;

    static StackType_t httpd_stack[HTTPD_STACK_SIZE];
    static StaticTask_t httpd_task;

    static StackType_t wifi_stack[WIFI_STACK_SIZE];
    static StaticTask_t wifi_task;

    /* Start PIR Sensor Task */
    printf("[SYSTEM] Starting PIR sensor task\r\n");
    if (xTaskCreateStatic(
            task_pir_sensor,  // Task function
            "pir_sensor",     // Task name
            PIR_STACK_SIZE,   // Stack size
            NULL,             // Task parameters
            10,               // Task priority
            pir_stack,        // Stack memory
            &pir_task         // Task handle
        ) == NULL)
    {
        printf("[ERROR] Failed to create PIR sensor task\r\n");
    }

    /* Start RFID Task */
    printf("[SYSTEM] Starting RFID task\r\n");
    if (xTaskCreateStatic(
            task_rfid,        // Task function
            "RFID",           // Task name
            RFID_STACK_SIZE,  // Stack size
            NULL,             // Task parameters
            8,                // Task priority
            rfid_stack,       // Stack memory
            &rfid_task        // Task handle
        ) == NULL)
    {
        printf("[ERROR] Failed to create RFID task\r\n");
    }

    // Create HTTP server task
    printf("[SYSTEM] Starting HTTP server task\r\n");
    extern void task_httpd(void *pvParameters);
    xTaskCreateStatic(
        task_httpd,       // Task function
        "httpd",          // Task name
        HTTPD_STACK_SIZE, // Stack size
        NULL,             // Task parameters
        11,               // Task priority
        httpd_stack,      // Stack memory
        &httpd_task       // Task handle
    );

    // Create Wi-Fi task
    printf("[SYSTEM] Starting Wi-Fi task\r\n");
    extern void task_wifi(void *pvParameters);
    xTaskCreateStatic(
        task_wifi,        // Task function
        "wifi",           // Task name
        WIFI_STACK_SIZE,  // Stack size
        NULL,             // Task parameters
        15,               // Task priority
        wifi_stack,       // Stack memory
        &wifi_task        // Task handle
    );

    // Start TCP/IP stack
    printf("[SYSTEM] Starting TCP/IP stack\r\n");
    tcpip_init(NULL, NULL);

    /* Start Scheduler */
    printf("[SYSTEM] Starting scheduler\r\n");
    vTaskStartScheduler();

    /* If the scheduler exits, it indicates an error */
    printf("[ERROR] Scheduler exited unexpectedly\r\n");
    for (;;);
}
