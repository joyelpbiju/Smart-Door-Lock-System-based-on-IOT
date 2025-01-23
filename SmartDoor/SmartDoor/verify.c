#include <stdio.h>
#include <string.h>
#include <verify.h>

// Define valid RFID tags and their corresponding names
typedef struct {
    const char *tag;
    const char *name;
} RFIDTagName;

static const RFIDTagName valid_tags[] = {
    {"3800B7DCFCAF", "Sudhakar Hanumanth"},
    {"3800B7FC6F1C", "Tobias Tefke"},
    {"3800B7D7540C", "Joshua Porunnedath Biju"}
};

#define NUM_VALID_TAGS (sizeof(valid_tags) / sizeof(valid_tags[0]))

/**
 * Verify the RFID tag.
 * @param tag: Scanned RFID tag
 * @return 1 if valid, 0 otherwise
 */
int verify_rfid_tag(const char *tag) {
    printf("[VERIFY] Verifying RFID Tag: %s\r\n", tag);
    for (int i = 0; i < NUM_VALID_TAGS; i++) {
        if (strcmp(tag, valid_tags[i].tag) == 0) {
            printf("[VERIFY] RFID Tag is valid.\r\n");
            return 1;
        }
    }
    printf("[VERIFY] RFID Tag is invalid.\r\n");
    return 0;
}

/**
 * Get the name associated with an RFID tag.
 * @param tag: Scanned RFID tag
 * @return Name if the tag is valid, NULL otherwise
 */
const char* get_rfid_name(const char *tag) {
    for (int i = 0; i < NUM_VALID_TAGS; i++) {
        if (strcmp(tag, valid_tags[i].tag) == 0) {
            return valid_tags[i].name;
        }
    }
    return NULL; // Tag not found
}
