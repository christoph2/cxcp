/*
 * BlueParrot XCP
 *
 * (C) 2007-2018 by Christoph Schueler <github.com/Christoph2,
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


#include "xcp_util.h"
#include "xcp_macros.h"

#if defined(_MSC_VER)
#include <stdio.h>
#endif // _MSC_VER

void XcpUtl_MemCopy(void * dst, void * src, uint32_t len)
{
    uint8_t * pd = (uint8_t *)dst;
    uint8_t * ps = (uint8_t *)src;

    //printf("\tDST: %p SRC: %p LEN: %u\n", dst, src, len);

/*    ASSERT(dst != (void *)NULL); */
/*    ASSERT(pd >= ps + len || ps >= pd + len); */
/*    ASSERT(len != (uint16_t)0); */

    while (len--) {
        *pd++ = *ps++;
    }

}

void XcpUtl_MemSet(void * dest, uint8_t fill_char, uint32_t len)
{
    uint8_t * p = (uint8_t *)dest;

/*    ASSERT(dest != (void *)NULL); */

    while (len--) {
        *p++ = fill_char;
    }
}

bool XcpUtl_MemCmp(void const * lhs, void const * rhs, uint32_t len)
{
    uint8_t const * pl = (uint8_t *)lhs;
    uint8_t const * pr = (uint8_t *)rhs;

    if (len == UINT32(0)) {
        return XCP_FALSE;
    }
    while ((*pl++ == *pr++) && (--len != UINT32(0))) {
    }
    return (bool)(len == UINT32(0));
}

#if XCP_BUILD_TYPE == XCP_DEBUG_BUILD
void XcpUtl_Hexdump(uint8_t const * buf, uint16_t sz)
{
    uint16_t idx;

    for (idx = UINT16(0); idx < sz; ++idx) {
        DBG_PRINT2("%02X ", buf[idx]);
    }
    DBG_PRINT1("\n");
}

void XcpUtl_Itoa(uint32_t value, uint8_t base, uint8_t * buf)
{
    uint32_t  mod;
    uint8_t   pos = (uint8_t)0x00, swap_pos = (uint8_t)0x00;
    uint8_t   ch;

    /* ASSERT(buf != (void *)NULL); */
    if (((int32_t)value) < 0L && base == (uint8_t)10) {
        value      = (uint32_t)((int32_t)value * -1L);
        buf[0]     = '-';
        swap_pos   = 1;
        pos        = 1;
    }

    if (value == 0L) {
        buf[pos++] = '0';
    }

    while (value) {
        mod = value % base;
        if (mod < 10) {
            buf[pos++] = '0' + mod;
        } else {
            buf[pos++] = 'A' + mod - (uint8_t)10;
        }
        value /= base;
    }
    buf[pos--] = '\0';
    while (pos > swap_pos) {
        ch  = buf[swap_pos];
        buf[swap_pos]  = buf[pos];
        buf[pos]       = ch;
        swap_pos++;
        pos--;
    }
}
#endif
