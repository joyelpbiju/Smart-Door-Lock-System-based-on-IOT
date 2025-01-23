#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bl_gpio.h>
#include <bl_uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <rfid.h>
#include <verify.h>
#include <time.h>

extern HeapRegion_t xHeapRegions[]; 

#define TIMEOUT_MS 1000
#define MAX_RFID_TAG_LENGTH 40
#define SER_STACK_SIZE 512

char rfid_tag[MAX_RFID_TAG_LENGTH];       // Buffer to store the current RFID tag
char concatenatedRFIDTags[2000];          // Buffer to store all scanned RFID tags
int concatenatedIndex = 0;                // Index for the concatenated buffer
int tag_received = 0;

// Initialize UART for the RFID reader
void init_rfid_uart() {
    // Debug UART initialization for system communication
    bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);  // UART0 at 2 Mbps
    printf("[DEBUG] UART0 initialized with default baud rate: 2,000,000\n");

    // EM-18 configuration (UART1 at 9600 baud)
    uint32_t baudrate = 9600;         // EM-18 baud rate
    uint8_t uart_id = RFID_UART_ID;   // UART1
    uint8_t tx_pin = 3;               // TX connected to GPIO 3
    uint8_t rx_pin = RFID_RX_PIN;     // RX connected to GPIO for EM-18 TX
    uint8_t cts_pin = 255;            // CTS not used
    uint8_t rts_pin = 255;            // RTS not used

    // Initialize UART1 with TX and RX pins
    if (bl_uart_init(uart_id, tx_pin, rx_pin, cts_pin, rts_pin, baudrate) != 0) {
        printf("[ERROR] UART1 initialization failed!\n");
        return;
    }

    printf("[DEBUG] UART1 initialized for EM-18 at baud rate: %lu\n", baudrate);
}

// Read RFID tags via UART
void read_rfid_tag() {
    int index = 0;
    printf("[RFID] Waiting for tag...\r\n");
    TickType_t lastReceivedTime = xTaskGetTickCount();

    static StackType_t ser_stack_pwm[SER_STACK_SIZE];
    static StaticTask_t ser_task_pwm;

    extern void task_ser_pwm(void *pvParameters);

    while (1) {
        // Read one byte from UART1
        int ch = bl_uart_data_recv(RFID_UART_ID);
        if (ch >= 0) {
            printf("%c", ch); // Debug: Print received character

            // Append to buffer if space is available
            if (index < MAX_RFID_TAG_LENGTH - 1) {
                rfid_tag[index++] = (char)ch;
            } else {
                printf("[ERROR] Tag too long. Resetting buffer.\r\n");
                index = 0; // Reset index if tag exceeds buffer length
            }

            // Update the time of the last received character
            lastReceivedTime = xTaskGetTickCount();
        }

        // Check for timeout
        if ((xTaskGetTickCount() - lastReceivedTime) * portTICK_PERIOD_MS > TIMEOUT_MS && index > 0) {
            rfid_tag[index] = '\0'; // Null-terminate the string
            printf("[RFID] Scanned Tag: %s\r\n", rfid_tag);

            // Process the tag (e.g., check validity)
            if (verify_rfid_tag(rfid_tag)) {
                printf("[RFID] Valid tag detected. Activating servo motor...\r\n");
                xTaskCreateStatic(
                    task_ser_pwm,    // Name of the function implementing the task
                    (char*)"ser pwm",// Human-readable name of the task
                    SER_STACK_SIZE,  // Stack size
                    NULL,            // Parameters for the function (not required here)
                    14,              // Task priority
                    ser_stack_pwm,   // Stack to use for the task
                    &ser_task_pwm    // Task handle (optional, for API references)
                );
            } else {
                printf("[RFID] Invalid tag detected. Ignoring...\r\n");
            }

            // Reset buffer for next tag
            index = 0;
        }
    }
}

// FreeRTOS task for RFID handling
void task_rfid(void *pvParameters) {
    printf("[RFID] RFID reader task starting...\r\n");

    // Initialize UART for RFID reader
    init_rfid_uart();

    printf("[RFID] Entering read loop...\r\n");
    // Start reading RFID tags
    read_rfid_tag();

    // Clean up (should never reach here in FreeRTOS)
    vTaskDelete(NULL);
}

// Get the last scanned RFID tag
char* get_rfid_tags() {
    printf("[RFID] Last RFID Tag: %s\r\n", rfid_tag);
    return rfid_tag;
}

// Clear the buffer of concatenated RFID tags
int clear_rfid_tags() {
    printf("[RFID] Cleared all scanned RFID tags.\r\n");
    memset(concatenatedRFIDTags, '\0', sizeof(concatenatedRFIDTags));
    return 0;
}

