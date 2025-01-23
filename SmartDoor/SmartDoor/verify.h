#ifndef VERIFY_H
#define VERIFY_H

// Function to verify the RFID tag
int verify_rfid_tag(const char *tag);

// Function to get the name associated with an RFID tag
const char* get_rfid_name(const char *tag);

#endif // VERIFY_H
