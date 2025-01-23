#ifndef MOTION_H
#define MOTION_H

// PIR sensor pin
#define PIR_SENSOR_PIN 5

// Task for motion detection
void task_pir_sensor(void *pvParameters);

// Utility functions for motion event logging
char* get_motion_events();
int clear_motion_events();

#endif // MOTION_H

