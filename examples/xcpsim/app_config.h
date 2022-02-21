

#if !defined(__APP_CONFIG_H)
#define __APP_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "flsemu.h"

#define CAL_PARAM __attribute__((section("calparams")))

const FlsEmu_ConfigType FlsEmu_Config;

#endif  // __APP_CONFIG_H
