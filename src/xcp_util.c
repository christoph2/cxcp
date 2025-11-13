/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
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

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp_util.h"

#include "xcp_macros.h"

/*!!! END-INCLUDE-SECTION !!!*/

void XcpUtl_MemCopy(/*@out@*/ void *dst, /*@in@*/ void const *src, uint32_t len) {
    uint8_t       *pd = (uint8_t *)dst;
    uint8_t const *ps = (uint8_t const *)src;

    XCP_ASSERT(dst != XCP_NULL);
    XCP_ASSERT(src != XCP_NULL);

    if (dst == XCP_NULL || src == XCP_NULL || len == 0UL) {
        return;
    }

    for (uint32_t i = 0U; i < len; ++i) {
        pd[i] = ps[i];
    }
}

void XcpUtl_MemSet(/*@out@*/ void *dest, uint8_t fill_char, uint32_t len) {
    uint8_t *p = (uint8_t *)dest;

    XCP_ASSERT(XCP_NULL != dest);
    if (dest == XCP_NULL || len == 0UL) {
        return;
    }

    for (uint32_t i = 0U; i < len; ++i) {
        p[i] = fill_char;
    }
}

bool XcpUtl_MemCmp(/*@in@*/ void const *lhs, /*@in@*/ void const *rhs, uint32_t len) {
    uint8_t const *pl = (uint8_t const *)lhs;
    uint8_t const *pr = (uint8_t const *)rhs;

    XCP_ASSERT(XCP_NULL != lhs);
    XCP_ASSERT(XCP_NULL != rhs);

    if (lhs == XCP_NULL || rhs == XCP_NULL || len == UINT32(0)) {
        return XCP_FALSE;
    }

    while (len && (*pl == *pr)) {
        ++pl;
        ++pr;
        --len;
    }
    return (bool)(len == UINT32(0));
}

uint32_t XcpUtl_StrLen(/*@in@*/ char const *str) {
    uint32_t len = 0U;

    XCP_ASSERT(XCP_NULL != str);
    if (str == XCP_NULL) {
        return 0UL;
    }

    while (str[len] != '\0') {
        ++len;
    }

    return len;
}

uint8_t XcpUtl_SetResetBit8(uint8_t result, uint8_t value, uint8_t flag) {
    if ((value & flag) == flag) {
        result |= flag;
    } else {
        result &= ~flag;
    }
    return result;
}

#if XCP_BUILD_TYPE == XCP_DEBUG_BUILD
void XcpUtl_Hexdump(/*@in@*/ uint8_t const *buf, uint16_t sz) {
    uint16_t idx;

    for (idx = UINT16(0); idx < sz; ++idx) {
        DBG_PRINT("%02X ", buf[idx]);
    }
    DBG_PRINT("\n\r");
}

void XcpUtl_Itoa(uint32_t value, uint8_t base, uint8_t *buf) {
    uint8_t mod;
    uint8_t pos = (uint8_t)0x00, swap_pos = (uint8_t)0x00;
    uint8_t ch;

    /* ASSERT(buf != (void *)NULL); */
    if (((int32_t)value) < 0L && base == (uint8_t)10) {
        value    = (uint32_t)((int32_t)value * -1L);
        buf[0]   = '-';
        swap_pos = 1;
        pos      = 1;
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
        ch            = buf[swap_pos];
        buf[swap_pos] = buf[pos];
        buf[pos]      = ch;
        swap_pos++;
        pos--;
    }
}
#endif
