

#if !defined(__APP_CONFIG_H)
#define __APP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "flsemu.h"

typedef struct {
    uint8_t value;
    bool down;
    uint32_t dummy;
} triangle_type;

extern triangle_type triangle;
extern uint16_t randomValue;

const FlsEmu_ConfigType FlsEmu_Config;

#endif // __APP_CONFIG_H

