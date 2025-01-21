#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwip/apps/fs.h>
#include <lwip/mem.h>
#include "cgi.h"
#include "led.h"
#include "motion.h"
#include "rfid.h"
#include "verify.h"

// Handle HTTP requests for custom pages
int fs_open_custom(struct fs_file *file, const char *name) {
    char *response = NULL;

    if (!strcmp(name, MOTION_ENDPOINT)) {
        // Generate motion detection status page
        response = (char *)calloc(1000, sizeof(char)); // Allocate memory for the response
        strcat(response, "<h1>Motion Detection Status</h1>");
        strcat(response, led_state.state_led_red == LED_ON ? "Motion detected<br>" : "No motion<br>");

        // Append motion event log
        strcat(response, "<h2>Motion Event Log:</h2>");
        const char *event_log = get_motion_events();
        if (strlen(event_log) > 0) {
            strcat(response, event_log);
        } else {
            strcat(response, "No motion events recorded.<br>");
        }

        // Add button to clear the motion log
        strcat(response, "<br><button onclick=\"location.href='/clearmotion.html'\">Clear Log</button>");
    } else if (!strcmp(name, CLEAR_MOTION_ENDPOINT)) {
        // Clear motion events and show confirmation
        clear_motion_events();
        response = (char *)calloc(200, sizeof(char));
        strcat(response, "<h1>Motion Event Log Cleared</h1>");
        strcat(response, "<a href='/motion.html'>Go Back</a>");
    } else if (!strcmp(name, DOOR_STATUS_ENDPOINT)) {
        // Generate door status page
        response = (char *)calloc(128, sizeof(char));
        const char* status = get_door_status();
        strcat(response, "<h1>Door Status</h1>");
        strcat(response, "<p>Current Door Status: ");
        strcat(response, status);
        strcat(response, "</p>");
    } else if (!strcmp(name, RFID_TAGS_ENDPOINT)) {
        // Generate RFID tags page
        response = (char *)calloc(256, sizeof(char));
        const char* lasttag = get_rfid_tags();
        const char* tag_name = get_rfid_name(lasttag);

        strcat(response, "<h1>Last Scanned RFID Tag</h1>");
        strcat(response, "<p>Last RFID Tag: ");
        strcat(response, lasttag ? lasttag : "None");
        strcat(response, "</p>");
        
        strcat(response, "<p>Tag Owner: ");
        strcat(response, tag_name ? tag_name : "Unknown");
        strcat(response, "</p>");
    } else if (!strcmp(name, OPEN_DOOR_ENDPOINT)) {
        // Generate door opening response
        response = (char *)calloc(256, sizeof(char));
        strcat(response, "<h1>Open Door</h1>");

        // Activate the servo motor to open the door
        extern void task_ser_pwm(void *pvParameters);
        static StackType_t ser_stack_pwm[512];
        static StaticTask_t ser_task_pwm;

        printf("[DOOR] Triggering door open operation...\r\n");

        void *task_handle = xTaskCreateStatic(
            task_ser_pwm,         // Servo motor task
            (char *)"Open Door",  // Task name
            512,                  // Stack size
            NULL,                 // Task parameters
            14,                   // Task priority
            ser_stack_pwm,        // Stack memory
            &ser_task_pwm         // Task handle
        );

        if (!task_handle) {
            printf("[ERROR] Failed to create Open Door task\r\n");
            strcat(response, "<p>Failed to open the door. Try again.</p>");
        } else {
            strcat(response, "<p>Door is opening...</p>");
        }

        strcat(response, "<a href='/doorstatus.html'>Check Door Status</a>");
    } else {
        // Unknown endpoint
        return 0;
    }

    int response_size = strlen(response);

    file->pextension = mem_malloc(response_size);
    if (file->pextension != NULL) {
        memcpy(file->pextension, response, response_size);
        file->data = (const char *)file->pextension;
        file->len = response_size;
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
    }
    free(response);
    return file->pextension != NULL;
}

// Free allocated memory for HTTP responses
void fs_close_custom(struct fs_file *file) {
    if (file && file->pextension) {
        mem_free(file->pextension);
        file->pextension = NULL;
    }
}

// Initialize custom files for both motion and RFID
void custom_files_init(void) {
    printf("Initializing custom file module for Motion and RFID projects\n");
}

// Initialize CGI handlers for both motion and RFID
void cgi_init(void) {
    printf("Initializing CGI handlers for Motion and RFID projects\n");
}

