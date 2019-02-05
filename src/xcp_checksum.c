/*
 * pySART - Simplified AUTOSAR-Toolkit for Python.
 *
 * (C) 2007-2019 by Christoph Schueler <github.com/Christoph2,
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

 /** CRC calculation based on code by Michael Barr: **/
 /**********************************************************************
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#include "xcp.h"


/*
** Local Types
*/
typedef enum tagXcp_ChecksumJobStateType {
    XCP_CHECKSUM_STATE_IDLE,
    XCP_CHECKSUM_STATE_RUNNING_INITIAL,
    XCP_CHECKSUM_STATE_RUNNING_REMAINING,
    XCP_CHECKSUM_STATE_RUNNING_FINAL
} Xcp_ChecksumJobStateType;


typedef struct tagXcp_ChecksumJobType {
    Xcp_ChecksumJobStateType state;
    Xcp_MtaType mta;
    uint32_t size;
    Xcp_ChecksumType interimChecksum;
} Xcp_ChecksumJobType;


#if XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_11

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_12

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_14

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_44

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16

#define XCP_CRC_NAME            "CRC-16"
#define XCP_CRC_POLYNOMIAL      ((uint16_t)0x8005)
#define XCP_CRC_INITIAL_VALUE   ((uint16_t)0x0000)
#define XCP_CRC_FINAL_XOR_VALUE ((uint16_t)0x0000)
#define REFLECT_DATA            XCP_TRUE
#define REFLECT_REMAINDER       XCP_TRUE
#define CHECK_VALUE             ((uint16_t)0xBB3D)

static const uint16_t CRC_TAB[] = {
    (uint16_t)0x0000, (uint16_t)0x8005, (uint16_t)0x800F, (uint16_t)0x000A, (uint16_t)0x801B, (uint16_t)0x001E, (uint16_t)0x0014, (uint16_t)0x8011,
    (uint16_t)0x8033, (uint16_t)0x0036, (uint16_t)0x003C, (uint16_t)0x8039, (uint16_t)0x0028, (uint16_t)0x802D, (uint16_t)0x8027, (uint16_t)0x0022,
    (uint16_t)0x8063, (uint16_t)0x0066, (uint16_t)0x006C, (uint16_t)0x8069, (uint16_t)0x0078, (uint16_t)0x807D, (uint16_t)0x8077, (uint16_t)0x0072,
    (uint16_t)0x0050, (uint16_t)0x8055, (uint16_t)0x805F, (uint16_t)0x005A, (uint16_t)0x804B, (uint16_t)0x004E, (uint16_t)0x0044, (uint16_t)0x8041,
    (uint16_t)0x80C3, (uint16_t)0x00C6, (uint16_t)0x00CC, (uint16_t)0x80C9, (uint16_t)0x00D8, (uint16_t)0x80DD, (uint16_t)0x80D7, (uint16_t)0x00D2,
    (uint16_t)0x00F0, (uint16_t)0x80F5, (uint16_t)0x80FF, (uint16_t)0x00FA, (uint16_t)0x80EB, (uint16_t)0x00EE, (uint16_t)0x00E4, (uint16_t)0x80E1,
    (uint16_t)0x00A0, (uint16_t)0x80A5, (uint16_t)0x80AF, (uint16_t)0x00AA, (uint16_t)0x80BB, (uint16_t)0x00BE, (uint16_t)0x00B4, (uint16_t)0x80B1,
    (uint16_t)0x8093, (uint16_t)0x0096, (uint16_t)0x009C, (uint16_t)0x8099, (uint16_t)0x0088, (uint16_t)0x808D, (uint16_t)0x8087, (uint16_t)0x0082,
    (uint16_t)0x8183, (uint16_t)0x0186, (uint16_t)0x018C, (uint16_t)0x8189, (uint16_t)0x0198, (uint16_t)0x819D, (uint16_t)0x8197, (uint16_t)0x0192,
    (uint16_t)0x01B0, (uint16_t)0x81B5, (uint16_t)0x81BF, (uint16_t)0x01BA, (uint16_t)0x81AB, (uint16_t)0x01AE, (uint16_t)0x01A4, (uint16_t)0x81A1,
    (uint16_t)0x01E0, (uint16_t)0x81E5, (uint16_t)0x81EF, (uint16_t)0x01EA, (uint16_t)0x81FB, (uint16_t)0x01FE, (uint16_t)0x01F4, (uint16_t)0x81F1,
    (uint16_t)0x81D3, (uint16_t)0x01D6, (uint16_t)0x01DC, (uint16_t)0x81D9, (uint16_t)0x01C8, (uint16_t)0x81CD, (uint16_t)0x81C7, (uint16_t)0x01C2,
    (uint16_t)0x0140, (uint16_t)0x8145, (uint16_t)0x814F, (uint16_t)0x014A, (uint16_t)0x815B, (uint16_t)0x015E, (uint16_t)0x0154, (uint16_t)0x8151,
    (uint16_t)0x8173, (uint16_t)0x0176, (uint16_t)0x017C, (uint16_t)0x8179, (uint16_t)0x0168, (uint16_t)0x816D, (uint16_t)0x8167, (uint16_t)0x0162,
    (uint16_t)0x8123, (uint16_t)0x0126, (uint16_t)0x012C, (uint16_t)0x8129, (uint16_t)0x0138, (uint16_t)0x813D, (uint16_t)0x8137, (uint16_t)0x0132,
    (uint16_t)0x0110, (uint16_t)0x8115, (uint16_t)0x811F, (uint16_t)0x011A, (uint16_t)0x810B, (uint16_t)0x010E, (uint16_t)0x0104, (uint16_t)0x8101,
    (uint16_t)0x8303, (uint16_t)0x0306, (uint16_t)0x030C, (uint16_t)0x8309, (uint16_t)0x0318, (uint16_t)0x831D, (uint16_t)0x8317, (uint16_t)0x0312,
    (uint16_t)0x0330, (uint16_t)0x8335, (uint16_t)0x833F, (uint16_t)0x033A, (uint16_t)0x832B, (uint16_t)0x032E, (uint16_t)0x0324, (uint16_t)0x8321,
    (uint16_t)0x0360, (uint16_t)0x8365, (uint16_t)0x836F, (uint16_t)0x036A, (uint16_t)0x837B, (uint16_t)0x037E, (uint16_t)0x0374, (uint16_t)0x8371,
    (uint16_t)0x8353, (uint16_t)0x0356, (uint16_t)0x035C, (uint16_t)0x8359, (uint16_t)0x0348, (uint16_t)0x834D, (uint16_t)0x8347, (uint16_t)0x0342,
    (uint16_t)0x03C0, (uint16_t)0x83C5, (uint16_t)0x83CF, (uint16_t)0x03CA, (uint16_t)0x83DB, (uint16_t)0x03DE, (uint16_t)0x03D4, (uint16_t)0x83D1,
    (uint16_t)0x83F3, (uint16_t)0x03F6, (uint16_t)0x03FC, (uint16_t)0x83F9, (uint16_t)0x03E8, (uint16_t)0x83ED, (uint16_t)0x83E7, (uint16_t)0x03E2,
    (uint16_t)0x83A3, (uint16_t)0x03A6, (uint16_t)0x03AC, (uint16_t)0x83A9, (uint16_t)0x03B8, (uint16_t)0x83BD, (uint16_t)0x83B7, (uint16_t)0x03B2,
    (uint16_t)0x0390, (uint16_t)0x8395, (uint16_t)0x839F, (uint16_t)0x039A, (uint16_t)0x838B, (uint16_t)0x038E, (uint16_t)0x0384, (uint16_t)0x8381,
    (uint16_t)0x0280, (uint16_t)0x8285, (uint16_t)0x828F, (uint16_t)0x028A, (uint16_t)0x829B, (uint16_t)0x029E, (uint16_t)0x0294, (uint16_t)0x8291,
    (uint16_t)0x82B3, (uint16_t)0x02B6, (uint16_t)0x02BC, (uint16_t)0x82B9, (uint16_t)0x02A8, (uint16_t)0x82AD, (uint16_t)0x82A7, (uint16_t)0x02A2,
    (uint16_t)0x82E3, (uint16_t)0x02E6, (uint16_t)0x02EC, (uint16_t)0x82E9, (uint16_t)0x02F8, (uint16_t)0x82FD, (uint16_t)0x82F7, (uint16_t)0x02F2,
    (uint16_t)0x02D0, (uint16_t)0x82D5, (uint16_t)0x82DF, (uint16_t)0x02DA, (uint16_t)0x82CB, (uint16_t)0x02CE, (uint16_t)0x02C4, (uint16_t)0x82C1,
    (uint16_t)0x8243, (uint16_t)0x0246, (uint16_t)0x024C, (uint16_t)0x8249, (uint16_t)0x0258, (uint16_t)0x825D, (uint16_t)0x8257, (uint16_t)0x0252,
    (uint16_t)0x0270, (uint16_t)0x8275, (uint16_t)0x827F, (uint16_t)0x027A, (uint16_t)0x826B, (uint16_t)0x026E, (uint16_t)0x0264, (uint16_t)0x8261,
    (uint16_t)0x0220, (uint16_t)0x8225, (uint16_t)0x822F, (uint16_t)0x022A, (uint16_t)0x823B, (uint16_t)0x023E, (uint16_t)0x0234, (uint16_t)0x8231,
    (uint16_t)0x8213, (uint16_t)0x0216, (uint16_t)0x021C, (uint16_t)0x8219, (uint16_t)0x0208, (uint16_t)0x820D, (uint16_t)0x8207, (uint16_t)0x0202,
};

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT

#define XCP_CRC_NAME            "CRC-CCITT"
#define XCP_CRC_POLYNOMIAL      ((uint16_t)0x1021)
#define XCP_CRC_INITIAL_VALUE   ((uint16_t)0xFFFF)
#define XCP_CRC_FINAL_XOR_VALUE ((uint16_t)0x0000)
#define REFLECT_DATA            XCP_FALSE
#define REFLECT_REMAINDER       XCP_FALSE
#define CHECK_VALUE             ((uint16_t)0x29B1)

static const uint16_t CRC_TAB[] = {
    (uint16_t)0x0000, (uint16_t)0x1021, (uint16_t)0x2042, (uint16_t)0x3063, (uint16_t)0x4084, (uint16_t)0x50A5, (uint16_t)0x60C6, (uint16_t)0x70E7,
    (uint16_t)0x8108, (uint16_t)0x9129, (uint16_t)0xA14A, (uint16_t)0xB16B, (uint16_t)0xC18C, (uint16_t)0xD1AD, (uint16_t)0xE1CE, (uint16_t)0xF1EF,
    (uint16_t)0x1231, (uint16_t)0x0210, (uint16_t)0x3273, (uint16_t)0x2252, (uint16_t)0x52B5, (uint16_t)0x4294, (uint16_t)0x72F7, (uint16_t)0x62D6,
    (uint16_t)0x9339, (uint16_t)0x8318, (uint16_t)0xB37B, (uint16_t)0xA35A, (uint16_t)0xD3BD, (uint16_t)0xC39C, (uint16_t)0xF3FF, (uint16_t)0xE3DE,
    (uint16_t)0x2462, (uint16_t)0x3443, (uint16_t)0x0420, (uint16_t)0x1401, (uint16_t)0x64E6, (uint16_t)0x74C7, (uint16_t)0x44A4, (uint16_t)0x5485,
    (uint16_t)0xA56A, (uint16_t)0xB54B, (uint16_t)0x8528, (uint16_t)0x9509, (uint16_t)0xE5EE, (uint16_t)0xF5CF, (uint16_t)0xC5AC, (uint16_t)0xD58D,
    (uint16_t)0x3653, (uint16_t)0x2672, (uint16_t)0x1611, (uint16_t)0x0630, (uint16_t)0x76D7, (uint16_t)0x66F6, (uint16_t)0x5695, (uint16_t)0x46B4,
    (uint16_t)0xB75B, (uint16_t)0xA77A, (uint16_t)0x9719, (uint16_t)0x8738, (uint16_t)0xF7DF, (uint16_t)0xE7FE, (uint16_t)0xD79D, (uint16_t)0xC7BC,
    (uint16_t)0x48C4, (uint16_t)0x58E5, (uint16_t)0x6886, (uint16_t)0x78A7, (uint16_t)0x0840, (uint16_t)0x1861, (uint16_t)0x2802, (uint16_t)0x3823,
    (uint16_t)0xC9CC, (uint16_t)0xD9ED, (uint16_t)0xE98E, (uint16_t)0xF9AF, (uint16_t)0x8948, (uint16_t)0x9969, (uint16_t)0xA90A, (uint16_t)0xB92B,
    (uint16_t)0x5AF5, (uint16_t)0x4AD4, (uint16_t)0x7AB7, (uint16_t)0x6A96, (uint16_t)0x1A71, (uint16_t)0x0A50, (uint16_t)0x3A33, (uint16_t)0x2A12,
    (uint16_t)0xDBFD, (uint16_t)0xCBDC, (uint16_t)0xFBBF, (uint16_t)0xEB9E, (uint16_t)0x9B79, (uint16_t)0x8B58, (uint16_t)0xBB3B, (uint16_t)0xAB1A,
    (uint16_t)0x6CA6, (uint16_t)0x7C87, (uint16_t)0x4CE4, (uint16_t)0x5CC5, (uint16_t)0x2C22, (uint16_t)0x3C03, (uint16_t)0x0C60, (uint16_t)0x1C41,
    (uint16_t)0xEDAE, (uint16_t)0xFD8F, (uint16_t)0xCDEC, (uint16_t)0xDDCD, (uint16_t)0xAD2A, (uint16_t)0xBD0B, (uint16_t)0x8D68, (uint16_t)0x9D49,
    (uint16_t)0x7E97, (uint16_t)0x6EB6, (uint16_t)0x5ED5, (uint16_t)0x4EF4, (uint16_t)0x3E13, (uint16_t)0x2E32, (uint16_t)0x1E51, (uint16_t)0x0E70,
    (uint16_t)0xFF9F, (uint16_t)0xEFBE, (uint16_t)0xDFDD, (uint16_t)0xCFFC, (uint16_t)0xBF1B, (uint16_t)0xAF3A, (uint16_t)0x9F59, (uint16_t)0x8F78,
    (uint16_t)0x9188, (uint16_t)0x81A9, (uint16_t)0xB1CA, (uint16_t)0xA1EB, (uint16_t)0xD10C, (uint16_t)0xC12D, (uint16_t)0xF14E, (uint16_t)0xE16F,
    (uint16_t)0x1080, (uint16_t)0x00A1, (uint16_t)0x30C2, (uint16_t)0x20E3, (uint16_t)0x5004, (uint16_t)0x4025, (uint16_t)0x7046, (uint16_t)0x6067,
    (uint16_t)0x83B9, (uint16_t)0x9398, (uint16_t)0xA3FB, (uint16_t)0xB3DA, (uint16_t)0xC33D, (uint16_t)0xD31C, (uint16_t)0xE37F, (uint16_t)0xF35E,
    (uint16_t)0x02B1, (uint16_t)0x1290, (uint16_t)0x22F3, (uint16_t)0x32D2, (uint16_t)0x4235, (uint16_t)0x5214, (uint16_t)0x6277, (uint16_t)0x7256,
    (uint16_t)0xB5EA, (uint16_t)0xA5CB, (uint16_t)0x95A8, (uint16_t)0x8589, (uint16_t)0xF56E, (uint16_t)0xE54F, (uint16_t)0xD52C, (uint16_t)0xC50D,
    (uint16_t)0x34E2, (uint16_t)0x24C3, (uint16_t)0x14A0, (uint16_t)0x0481, (uint16_t)0x7466, (uint16_t)0x6447, (uint16_t)0x5424, (uint16_t)0x4405,
    (uint16_t)0xA7DB, (uint16_t)0xB7FA, (uint16_t)0x8799, (uint16_t)0x97B8, (uint16_t)0xE75F, (uint16_t)0xF77E, (uint16_t)0xC71D, (uint16_t)0xD73C,
    (uint16_t)0x26D3, (uint16_t)0x36F2, (uint16_t)0x0691, (uint16_t)0x16B0, (uint16_t)0x6657, (uint16_t)0x7676, (uint16_t)0x4615, (uint16_t)0x5634,
    (uint16_t)0xD94C, (uint16_t)0xC96D, (uint16_t)0xF90E, (uint16_t)0xE92F, (uint16_t)0x99C8, (uint16_t)0x89E9, (uint16_t)0xB98A, (uint16_t)0xA9AB,
    (uint16_t)0x5844, (uint16_t)0x4865, (uint16_t)0x7806, (uint16_t)0x6827, (uint16_t)0x18C0, (uint16_t)0x08E1, (uint16_t)0x3882, (uint16_t)0x28A3,
    (uint16_t)0xCB7D, (uint16_t)0xDB5C, (uint16_t)0xEB3F, (uint16_t)0xFB1E, (uint16_t)0x8BF9, (uint16_t)0x9BD8, (uint16_t)0xABBB, (uint16_t)0xBB9A,
    (uint16_t)0x4A75, (uint16_t)0x5A54, (uint16_t)0x6A37, (uint16_t)0x7A16, (uint16_t)0x0AF1, (uint16_t)0x1AD0, (uint16_t)0x2AB3, (uint16_t)0x3A92,
    (uint16_t)0xFD2E, (uint16_t)0xED0F, (uint16_t)0xDD6C, (uint16_t)0xCD4D, (uint16_t)0xBDAA, (uint16_t)0xAD8B, (uint16_t)0x9DE8, (uint16_t)0x8DC9,
    (uint16_t)0x7C26, (uint16_t)0x6C07, (uint16_t)0x5C64, (uint16_t)0x4C45, (uint16_t)0x3CA2, (uint16_t)0x2C83, (uint16_t)0x1CE0, (uint16_t)0x0CC1,
    (uint16_t)0xEF1F, (uint16_t)0xFF3E, (uint16_t)0xCF5D, (uint16_t)0xDF7C, (uint16_t)0xAF9B, (uint16_t)0xBFBA, (uint16_t)0x8FD9, (uint16_t)0x9FF8,
    (uint16_t)0x6E17, (uint16_t)0x7E36, (uint16_t)0x4E55, (uint16_t)0x5E74, (uint16_t)0x2E93, (uint16_t)0x3EB2, (uint16_t)0x0ED1, (uint16_t)0x1EF0,
};

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32

#define XCP_CRC_NAME            "CRC-32"
#define XCP_CRC_POLYNOMIAL      ((uint32_t)0x04C11DB7)
#define XCP_CRC_INITIAL_VALUE   ((uint32_t)0xFFFFFFFF)
#define XCP_CRC_FINAL_XOR_VALUE ((uint32_t)0xFFFFFFFF)
#define REFLECT_DATA            XCP_TRUE
#define REFLECT_REMAINDER       XCP_TRUE
#define CHECK_VALUE             ((uint32_t)0xCBF43926)

static const uint32_t CRC_TAB[] = {
    (uint32_t)0x00000000, (uint32_t)0x77073096, (uint32_t)0xee0e612c, (uint32_t)0x990951ba, (uint32_t)0x076dc419,
    (uint32_t)0x706af48f, (uint32_t)0xe963a535, (uint32_t)0x9e6495a3, (uint32_t)0x0edb8832, (uint32_t)0x79dcb8a4,
    (uint32_t)0xe0d5e91e, (uint32_t)0x97d2d988, (uint32_t)0x09b64c2b, (uint32_t)0x7eb17cbd, (uint32_t)0xe7b82d07,
    (uint32_t)0x90bf1d91, (uint32_t)0x1db71064, (uint32_t)0x6ab020f2, (uint32_t)0xf3b97148, (uint32_t)0x84be41de,
    (uint32_t)0x1adad47d, (uint32_t)0x6ddde4eb, (uint32_t)0xf4d4b551, (uint32_t)0x83d385c7, (uint32_t)0x136c9856,
    (uint32_t)0x646ba8c0, (uint32_t)0xfd62f97a, (uint32_t)0x8a65c9ec, (uint32_t)0x14015c4f, (uint32_t)0x63066cd9,
    (uint32_t)0xfa0f3d63, (uint32_t)0x8d080df5, (uint32_t)0x3b6e20c8, (uint32_t)0x4c69105e, (uint32_t)0xd56041e4,
    (uint32_t)0xa2677172, (uint32_t)0x3c03e4d1, (uint32_t)0x4b04d447, (uint32_t)0xd20d85fd, (uint32_t)0xa50ab56b,
    (uint32_t)0x35b5a8fa, (uint32_t)0x42b2986c, (uint32_t)0xdbbbc9d6, (uint32_t)0xacbcf940, (uint32_t)0x32d86ce3,
    (uint32_t)0x45df5c75, (uint32_t)0xdcd60dcf, (uint32_t)0xabd13d59, (uint32_t)0x26d930ac, (uint32_t)0x51de003a,
    (uint32_t)0xc8d75180, (uint32_t)0xbfd06116, (uint32_t)0x21b4f4b5, (uint32_t)0x56b3c423, (uint32_t)0xcfba9599,
    (uint32_t)0xb8bda50f, (uint32_t)0x2802b89e, (uint32_t)0x5f058808, (uint32_t)0xc60cd9b2, (uint32_t)0xb10be924,
    (uint32_t)0x2f6f7c87, (uint32_t)0x58684c11, (uint32_t)0xc1611dab, (uint32_t)0xb6662d3d, (uint32_t)0x76dc4190,
    (uint32_t)0x01db7106, (uint32_t)0x98d220bc, (uint32_t)0xefd5102a, (uint32_t)0x71b18589, (uint32_t)0x06b6b51f,
    (uint32_t)0x9fbfe4a5, (uint32_t)0xe8b8d433, (uint32_t)0x7807c9a2, (uint32_t)0x0f00f934, (uint32_t)0x9609a88e,
    (uint32_t)0xe10e9818, (uint32_t)0x7f6a0dbb, (uint32_t)0x086d3d2d, (uint32_t)0x91646c97, (uint32_t)0xe6635c01,
    (uint32_t)0x6b6b51f4, (uint32_t)0x1c6c6162, (uint32_t)0x856530d8, (uint32_t)0xf262004e, (uint32_t)0x6c0695ed,
    (uint32_t)0x1b01a57b, (uint32_t)0x8208f4c1, (uint32_t)0xf50fc457, (uint32_t)0x65b0d9c6, (uint32_t)0x12b7e950,
    (uint32_t)0x8bbeb8ea, (uint32_t)0xfcb9887c, (uint32_t)0x62dd1ddf, (uint32_t)0x15da2d49, (uint32_t)0x8cd37cf3,
    (uint32_t)0xfbd44c65, (uint32_t)0x4db26158, (uint32_t)0x3ab551ce, (uint32_t)0xa3bc0074, (uint32_t)0xd4bb30e2,
    (uint32_t)0x4adfa541, (uint32_t)0x3dd895d7, (uint32_t)0xa4d1c46d, (uint32_t)0xd3d6f4fb, (uint32_t)0x4369e96a,
    (uint32_t)0x346ed9fc, (uint32_t)0xad678846, (uint32_t)0xda60b8d0, (uint32_t)0x44042d73, (uint32_t)0x33031de5,
    (uint32_t)0xaa0a4c5f, (uint32_t)0xdd0d7cc9, (uint32_t)0x5005713c, (uint32_t)0x270241aa, (uint32_t)0xbe0b1010,
    (uint32_t)0xc90c2086, (uint32_t)0x5768b525, (uint32_t)0x206f85b3, (uint32_t)0xb966d409, (uint32_t)0xce61e49f,
    (uint32_t)0x5edef90e, (uint32_t)0x29d9c998, (uint32_t)0xb0d09822, (uint32_t)0xc7d7a8b4, (uint32_t)0x59b33d17,
    (uint32_t)0x2eb40d81, (uint32_t)0xb7bd5c3b, (uint32_t)0xc0ba6cad, (uint32_t)0xedb88320, (uint32_t)0x9abfb3b6,
    (uint32_t)0x03b6e20c, (uint32_t)0x74b1d29a, (uint32_t)0xead54739, (uint32_t)0x9dd277af, (uint32_t)0x04db2615,
    (uint32_t)0x73dc1683, (uint32_t)0xe3630b12, (uint32_t)0x94643b84, (uint32_t)0x0d6d6a3e, (uint32_t)0x7a6a5aa8,
    (uint32_t)0xe40ecf0b, (uint32_t)0x9309ff9d, (uint32_t)0x0a00ae27, (uint32_t)0x7d079eb1, (uint32_t)0xf00f9344,
    (uint32_t)0x8708a3d2, (uint32_t)0x1e01f268, (uint32_t)0x6906c2fe, (uint32_t)0xf762575d, (uint32_t)0x806567cb,
    (uint32_t)0x196c3671, (uint32_t)0x6e6b06e7, (uint32_t)0xfed41b76, (uint32_t)0x89d32be0, (uint32_t)0x10da7a5a,
    (uint32_t)0x67dd4acc, (uint32_t)0xf9b9df6f, (uint32_t)0x8ebeeff9, (uint32_t)0x17b7be43, (uint32_t)0x60b08ed5,
    (uint32_t)0xd6d6a3e8, (uint32_t)0xa1d1937e, (uint32_t)0x38d8c2c4, (uint32_t)0x4fdff252, (uint32_t)0xd1bb67f1,
    (uint32_t)0xa6bc5767, (uint32_t)0x3fb506dd, (uint32_t)0x48b2364b, (uint32_t)0xd80d2bda, (uint32_t)0xaf0a1b4c,
    (uint32_t)0x36034af6, (uint32_t)0x41047a60, (uint32_t)0xdf60efc3, (uint32_t)0xa867df55, (uint32_t)0x316e8eef,
    (uint32_t)0x4669be79, (uint32_t)0xcb61b38c, (uint32_t)0xbc66831a, (uint32_t)0x256fd2a0, (uint32_t)0x5268e236,
    (uint32_t)0xcc0c7795, (uint32_t)0xbb0b4703, (uint32_t)0x220216b9, (uint32_t)0x5505262f, (uint32_t)0xc5ba3bbe,
    (uint32_t)0xb2bd0b28, (uint32_t)0x2bb45a92, (uint32_t)0x5cb36a04, (uint32_t)0xc2d7ffa7, (uint32_t)0xb5d0cf31,
    (uint32_t)0x2cd99e8b, (uint32_t)0x5bdeae1d, (uint32_t)0x9b64c2b0, (uint32_t)0xec63f226, (uint32_t)0x756aa39c,
    (uint32_t)0x026d930a, (uint32_t)0x9c0906a9, (uint32_t)0xeb0e363f, (uint32_t)0x72076785, (uint32_t)0x05005713,
    (uint32_t)0x95bf4a82, (uint32_t)0xe2b87a14, (uint32_t)0x7bb12bae, (uint32_t)0x0cb61b38, (uint32_t)0x92d28e9b,
    (uint32_t)0xe5d5be0d, (uint32_t)0x7cdcefb7, (uint32_t)0x0bdbdf21, (uint32_t)0x86d3d2d4, (uint32_t)0xf1d4e242,
    (uint32_t)0x68ddb3f8, (uint32_t)0x1fda836e, (uint32_t)0x81be16cd, (uint32_t)0xf6b9265b, (uint32_t)0x6fb077e1,
    (uint32_t)0x18b74777, (uint32_t)0x88085ae6, (uint32_t)0xff0f6a70, (uint32_t)0x66063bca, (uint32_t)0x11010b5c,
    (uint32_t)0x8f659eff, (uint32_t)0xf862ae69, (uint32_t)0x616bffd3, (uint32_t)0x166ccf45, (uint32_t)0xa00ae278,
    (uint32_t)0xd70dd2ee, (uint32_t)0x4e048354, (uint32_t)0x3903b3c2, (uint32_t)0xa7672661, (uint32_t)0xd06016f7,
    (uint32_t)0x4969474d, (uint32_t)0x3e6e77db, (uint32_t)0xaed16a4a, (uint32_t)0xd9d65adc, (uint32_t)0x40df0b66,
    (uint32_t)0x37d83bf0, (uint32_t)0xa9bcae53, (uint32_t)0xdebb9ec5, (uint32_t)0x47b2cf7f, (uint32_t)0x30b5ffe9,
    (uint32_t)0xbdbdf21c, (uint32_t)0xcabac28a, (uint32_t)0x53b39330, (uint32_t)0x24b4a3a6, (uint32_t)0xbad03605,
    (uint32_t)0xcdd70693, (uint32_t)0x54de5729, (uint32_t)0x23d967bf, (uint32_t)0xb3667a2e, (uint32_t)0xc4614ab8,
    (uint32_t)0x5d681b02, (uint32_t)0x2a6f2b94, (uint32_t)0xb40bbe37, (uint32_t)0xc30c8ea1, (uint32_t)0x5a05df1b,
    (uint32_t)0x2d02ef8d
};


#endif // XCP_CHECKSUM_METHOD

#define WIDTH    ((uint16_t)(8U * sizeof(Xcp_ChecksumType)))
#define TOPBIT   (1 << (WIDTH - 1))

#if (REFLECT_DATA == XCP_TRUE)
#define CRC_REFLECT_DATA(X)         ((uint8_t) reflect((X), 8))
#else
#define CRC_REFLECT_DATA(X)         (X)
#endif

#if (REFLECT_REMAINDER == XCP_TRUE)
#define CRC_REFLECT_REMAINDER(X)    ((Xcp_ChecksumType) reflect((X), WIDTH))
#else
#define CRC_REFLECT_REMAINDER(X)    (X)
#endif

 #if (REFLECT_DATA == XCP_TRUE) || (REFLECT_REMAINDER == XCP_TRUE)
static uint32_t reflect(uint32_t data, uint8_t nBits)
{
        uint32_t  reflection = 0x00000000;
        uint8_t  bit;
        /*
         * Reflect the data about the center bit.
         */
        for (bit = 0; bit < nBits; ++bit) {
                /*
                 * If the LSB bit is set, set the reflection of it.
                 */
                if (data & 0x01) {
                        reflection |= (1 << ((nBits - 1) - bit));
                }
                data = (data >> 1);
        }
        return reflection;
}
#endif  // (REFLECT_DATA == TRUE) || (REFLECT_REMAINDER == TRUE)


/*
0x04    XCP_ADD_22      Add WORD into a WORD checksum, ignore overflows, blocksize must be modulo 2
0x05    XCP_ADD_24      Add WORD into a DWORD checksum, ignore overflows, blocksize must be modulo 2
0x06    XCP_ADD_44      Add DWORD into  DWORD, ignore overflows, blocksize must be modulo 4
*/

Xcp_ChecksumType Xcp_CalculateChecksum(uint8_t const * ptr, uint32_t length, Xcp_ChecksumType startValue, bool isFirstCall)
{
    Xcp_ChecksumType result;
    uint32_t idx;
#if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT) || \
    (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32)
    uint8_t data;

    if (isFirstCall) {
        result = XCP_CRC_INITIAL_VALUE;
    } else {
        result = startValue;
    }

    for (idx = (uint32_t)0UL; idx < length; ++idx)
    {
        data = CRC_REFLECT_DATA(ptr[idx]) ^ UINT8(result >> (WIDTH - UINT8(8)));
        result = CRC_TAB[data] ^ UINT16(result << 8);
    }
    return CRC_REFLECT_REMAINDER(result) ^ XCP_CRC_FINAL_XOR_VALUE;
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_11) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_12) || \
      (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_14)
    if (isFirstCall) {
        result = (Xcp_ChecksumType)0;
    } else {
        result = startValue;
    }

    for (idx = (uint32_t)0UL; idx < length; ++idx) {
        result += ptr[idx];
    }

    return result;
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24) || \
      (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_44)

#if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24)
    uint16_t const * data = (uint16_t const *)ptr;  /* Undefined behaviour -  See note above */

    length >>= 1;
#else
    uint32_t const * data = (uint32_t const *)ptr;  /* Undefined behaviour -  See note above */

    length >>= 2;
#endif  /* XCP_CHECKSUM_METHOD */
    if (isFirstCall) {
        result = (Xcp_ChecksumType)0;
    } else {
        result = startValue;
    }

    for (idx = (uint32_t)0UL; idx < length; ++idx) {
        result += data[idx];
    }

    return result;
#endif /* XCP_CHECKSUM_METHOD */
}


#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON && XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON
static Xcp_ChecksumJobType Xcp_ChecksumJob;

void Xcp_ChecksumInit(void)
{
    Xcp_ChecksumJob.mta.address = UINT32(0ul);
    Xcp_ChecksumJob.mta.ext = UINT8(0);
    Xcp_ChecksumJob.interimChecksum = (Xcp_ChecksumType)0ul;
    Xcp_ChecksumJob.size = UINT32(0ul);
    Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_IDLE;
}

static uint32_t counter = 0ul;

void Xcp_StartChecksumCalculation(uint8_t const * ptr, uint32_t size)
{
    XCP_ENTER_CRITICAL();
    if ((Xcp_ChecksumJob.state != XCP_CHECKSUM_STATE_IDLE) || Xcp_IsBusy()) {
        XCP_LEAVE_CRITICAL();
        return;
    }
    Xcp_SetBusy(XCP_TRUE);
    Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_INITIAL;
    printf("S-Address: %p Size: %u\n", ptr, size);
    Xcp_ChecksumJob.mta.address = (uint32_t)ptr;
    Xcp_ChecksumJob.size = size;
    XCP_LEAVE_CRITICAL();
}

/** @brief Do lengthy checksum/CRC calculations in the background.
 *
 *
 */
void Xcp_ChecksumMainFunction(void)
{
    if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_IDLE) {
        return;
    } else if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_RUNNING_INITIAL) {
         Xcp_ChecksumJob.interimChecksum = Xcp_CalculateChecksum(
            (uint8_t const *)Xcp_ChecksumJob.mta.address, XCP_CHECKSUM_CHUNK_SIZE, (Xcp_ChecksumType)0, XCP_TRUE
         );
        Xcp_ChecksumJob.size -= XCP_CHECKSUM_CHUNK_SIZE;
        Xcp_ChecksumJob.mta.address += XCP_CHECKSUM_CHUNK_SIZE;
         Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_REMAINING;
    } else if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_RUNNING_REMAINING) {
        Xcp_ChecksumJob.interimChecksum = Xcp_CalculateChecksum(
            (uint8_t const *)Xcp_ChecksumJob.mta.address, XCP_CHECKSUM_CHUNK_SIZE, Xcp_ChecksumJob.interimChecksum, XCP_FALSE
        );
        //printf("N-Address: %x Size: %u CS: %x\n", Xcp_ChecksumJob.mta.address, Xcp_ChecksumJob.size, Xcp_ChecksumJob.interimChecksum);
        Xcp_ChecksumJob.size -= XCP_CHECKSUM_CHUNK_SIZE;
        Xcp_ChecksumJob.mta.address += XCP_CHECKSUM_CHUNK_SIZE;
        if (Xcp_ChecksumJob.size <= XCP_CHECKSUM_CHUNK_SIZE) {
            Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_FINAL;
        }
    }  else if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_RUNNING_FINAL) {
        //printf("F-Address: %x Size: %u CS: %x\n", Xcp_ChecksumJob.mta.address, Xcp_ChecksumJob.size, Xcp_ChecksumJob.interimChecksum);
        Xcp_ChecksumJob.interimChecksum = Xcp_CalculateChecksum(
            (uint8_t const *)Xcp_ChecksumJob.mta.address, Xcp_ChecksumJob.size, Xcp_ChecksumJob.interimChecksum, XCP_FALSE
        );
        //printf("FINAL-VALUE %x\n", Xcp_ChecksumJob.interimChecksum);
        Xcp_SetBusy(XCP_FALSE);
        Xcp_SendChecksumPositiveResponse(Xcp_ChecksumJob.interimChecksum);
        Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_IDLE;
    }
}
#endif // XCP_ENABLE_BUILD_CHECKSUM
