#ifndef ECUSIM_XCP_CONFIG_H
#define ECUSIM_XCP_CONFIG_H

/*
 * ECUSim uses the desktop simulator's Ethernet configuration, including its
 * address mapper required by the flash emulator.
 */
#include "../xcpsim/xcp_config.h"

#define ECUSIM_ENABLE_ELF_MAPPER_TRACE XCP_ON

#undef XCP_ENABLE_EXTERN_C_GUARDS
#define XCP_ENABLE_EXTERN_C_GUARDS XCP_ON

#undef XCP_DAQ_MAX_EVENT_CHANNEL
#define XCP_DAQ_MAX_EVENT_CHANNEL (2)

#endif /* ECUSIM_XCP_CONFIG_H */
