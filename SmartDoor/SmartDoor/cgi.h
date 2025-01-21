#ifndef MOTION_CGI_H
#define MOTION_CGI_H

// Endpoints
// Motion Detection Endpoints
#define MOTION_ENDPOINT "/motion.html"
#define CLEAR_MOTION_ENDPOINT "/clearmotion.html"

// RFID Endpoints
#define DOOR_STATUS_ENDPOINT "/doorstatus.html"
#define RFID_TAGS_ENDPOINT "/rfidtags.html"

// Door Control Endpoint
#define OPEN_DOOR_ENDPOINT "/opendoor.html"

// External variables for RFID and Servo
extern char *get_rfid_tags();
extern const char* get_door_status();

// Initialization functions
void custom_files_init(void);
void cgi_init(void);

#endif // MOTION_CGI_H
