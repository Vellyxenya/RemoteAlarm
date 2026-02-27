#ifndef _BOARD_H_
#define _BOARD_H_

#include "esp_err.h"

// This tells the ADF that you aren't using an external codec chip
// that requires a specific driver from the LyraT days.
typedef struct {
    int dummy;
} board_handle_t;

// You can add your I2S pin definitions here later if you want 
// to keep them organized.
#define I2S_BCK_PIN 17
#define I2S_WS_PIN  18
#define I2S_DATA_PIN 16

#endif