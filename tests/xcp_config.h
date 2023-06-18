/*
 * BlueParrot XCP
 *
 * (C) 2007-2021 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

/*
 *  For details on options refer to `documentation
 * <https://github.com/christoph2/cxcp/docs/options.rst>`_
 */

#if !defined(__XCP_CONFIG_H)
#define __XCP_CONFIG_H

#define XCP_TRANSPORT_LAYER XCP_ON_ETHERNET

#define XCP_GET_ID_0 "BlueParrot XCP tests"
#define XCP_GET_ID_1 "Example_Project"

#define XCP_BUILD_TYPE XCP_DEBUG_BUILD

#define XCP_EXTERN_C_GUARDS XCP_OFF

#define XCP_ENABLE_SLAVE_BLOCKMODE XCP_ON
#define XCP_ENABLE_MASTER_BLOCKMODE XCP_ON

#define XCP_ENABLE_STIM XCP_OFF

#define XCP_CHECKSUM_METHOD XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT
#define XCP_CHECKSUM_CHUNKED_CALCULATION XCP_ON
#define XCP_CHECKSUM_CHUNK_SIZE (64)
#define XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE (0) /* 0 ==> unlimited */

#define XCP_BYTE_ORDER XCP_BYTE_ORDER_INTEL
#define XCP_ADDRESS_GRANULARITY XCP_ADDRESS_GRANULARITY_BYTE

#define XCP_MAX_CTO (8)
#define XCP_MAX_DTO (8)

#define XCP_ENABLE_DAQ_COMMANDS XCP_OFF

#define XCP_ENABLE_DOWNLOAD_NEXT XCP_ON
#define XCP_ENABLE_DOWNLOAD_MAX XCP_ON
#define XCP_ENABLE_SHORT_DOWNLOAD XCP_ON

/*
 * **  DAQ Settings.
 * */
#define XCP_DAQ_CONFIG_TYPE XCP_DAQ_CONFIG_TYPE_DYNAMIC
#define XCP_DAQ_DTO_BUFFER_SIZE (40)
#define XCP_DAQ_ENABLE_PREDEFINED_LISTS XCP_OFF
#define XCP_DAQ_TIMESTAMP_UNIT (XCP_DAQ_TIMESTAMP_UNIT_10US)
#define XCP_DAQ_TIMESTAMP_SIZE (XCP_DAQ_TIMESTAMP_SIZE_4)
#define XCP_DAQ_ENABLE_PRESCALER XCP_OFF
#define XCP_DAQ_ENABLE_ADDR_EXT XCP_OFF
#define XCP_DAQ_ENABLE_BIT_OFFSET XCP_OFF
#define XCP_DAQ_ENABLE_PRIORITIZATION XCP_OFF
#define XCP_DAQ_ENABLE_ALTERNATING XCP_OFF
#define XCP_DAQ_ENABLE_CLOCK_ACCESS_ALWAYS XCP_ON
#define XCP_DAQ_ENABLE_QUEUING XCP_ON
#define XCP_DAQ_ENABLE_WRITE_THROUGH XCP_ON
#define XCP_DAQ_MAX_DYNAMIC_ENTITIES (100)
#define XCP_DAQ_MAX_EVENT_CHANNEL (3)
#define XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT XCP_OFF
#define XCP_DAQ_ENABLE_DYNAMIC_LISTS XCP_ON

/*
 * **  Platform Specific Options.
 * */
#define XCP_ENTER_CRITICAL()
#define XCP_LEAVE_CRITICAL()
#define XCP_TL_ENTER_CRITICAL()
#define XCP_TL_LEAVE_CRITICAL()
#define XCP_DAQ_ENTER_CRITICAL()
#define XCP_DAQ_LEAVE_CRITICAL()
#define XCP_STIM_ENTER_CRITICAL()
#define XCP_STIM_LEAVE_CRITICAL()
#define XCP_PGM_ENTER_CRITICAL()
#define XCP_PGM_LEAVE_CRITICAL()
#define XCP_CAL_ENTER_CRITICAL()
#define XCP_CAL_LEAVE_CRITICAL()
#define XCP_PAG_ENTER_CRITICAL()
#define XCP_PAG_LEAVE_CRITICAL()

#define XCP_TRANSPORT_LAYER_LENGTH_SIZE (2)
#define XCP_TRANSPORT_LAYER_COUNTER_SIZE (2)
#define XCP_TRANSPORT_LAYER_CHECKSUM_SIZE (0)

#define XCP_MAIN_FUNCTION_PERIOD (2000)

#endif /* __XCP_CONFIG_H */
