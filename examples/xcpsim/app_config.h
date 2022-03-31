

#if !defined(__APP_CONFIG_H)
#define __APP_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "flsemu.h"

#if defined(_MSC_VER)
#pragma section("calparams", read)
#define CAL_PARAM __declspec(allocate("calparams"))
#else
#define CAL_PARAM __attribute__((section("calparams")))
#endif

extern const FlsEmu_ConfigType FlsEmu_Config;

#endif // __APP_CONFIG_H
