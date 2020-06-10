/*
 * BlueParrot XCP
 *
 * (C) 2007-2020 by Christoph Schueler <github.com/Christoph2,
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

#if !defined(__XCP_HW_H)
#define __XCP_HW_H

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
extern "C"
{
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


/*
**  Global Defines.
*/


typedef struct tagXcpHw_OptionsType {
#if defined(KVASER_CAN)
    int dummy;
#elif defined(ETHER)
    bool ipv6;
    bool tcp;
#elif defined(SOCKET_CAN)
    bool fd;
    char interface[64];
#endif
} XcpHw_OptionsType;

void XcpHw_ParseCommandLineOptions(int argc, char **argv, XcpHw_OptionsType * options);
void XcpHw_ErrorMsg(char * const function, int errorCode);

void XcpHw_SignalApplicationState(uint32_t state, uint8_t signal_all);
uint32_t XcpHw_WaitApplicationState(uint32_t mask);
void XcpHw_ResetApplicationState(uint32_t mask);

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
}
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


#endif /* __XCP_HW_H */
