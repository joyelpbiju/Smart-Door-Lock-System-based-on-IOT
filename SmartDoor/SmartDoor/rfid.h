#ifndef RFID_DEFS_H
#define RFID_DEFS_H

#define RFID_UART_ID 1       // Use UART1 for the EM-18
#define RFID_RX_PIN 4      // RX pin (connected to EM-18's TX pin, IO3)
#define RFID_BAUDRATE 9600   // Baud rate for EM-18

// Define the correct RFID tag for verification
#define RFID_TAG "3800B7DCFCAF"  // Predefined RFID tag value

// Function declarations
void init_rfid_uart();
void verify_rfid();
void read_rfid_tag();
void task_rfid(void *pvParameters);
char* get_rfid_tags();
int clear_rfid_tags();

#endif


