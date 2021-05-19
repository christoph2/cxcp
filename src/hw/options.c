/*
 * BlueParrot XCP
 *
 * (C) 2021 by Christoph Schueler <github.com/Christoph2,
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

#include "xcp.h"

#if defined(_WIN32)

#else
#include <unistd.h>

#if defined(ETHER)
static const char OPTION_STR[] = "htu46p:";
#elif defined(SOCKET_CAN)
static const char OPTION_STR[] = "hi:f";
#endif

#endif


#if defined(_WIN32)
void parse_options(int argc, char ** argv, Xcp_OptionsType * options)
{
    int idx;
    char * arg;
#if !defined(KVASER_CAN)
    options->ipv6 = XCP_FALSE;
    options->tcp = XCP_TRUE;

    if (argc >= 2) {
        for(idx = 1; idx < argc; ++idx) {
            arg = argv[idx];
            if ((arg[0] != '/') && (arg[0] != '-')) {
                continue;
            }
            switch (arg[1]) {
                case '4':
                    options->ipv6 = XCP_FALSE;
                    break;
                case '6':
                    options->ipv6 = XCP_TRUE;
                    break;
                case 'u':
                    options->tcp = XCP_FALSE;
                    break;
                case 't':
                    options->tcp = XCP_TRUE;
                    break;
                case 'h':
                    break;
                default:
                    break;
            }
        }
    }
#endif
}
#else
void parse_options(int argc, char ** argv, Xcp_OptionsType * options)
{
    int opt;
    int res;

#if defined(ETHER)
    int p_assigned = 0;
    int v_assigned = 0;

    Xcp_Options->tcp = XCP_TRUE;
    Xcp_Options->ipv6 = XCP_FALSE;
    Xcp_Options->port = XCP_ETH_DEFAULT_PORT;
#elif defined(SOCKET_CAN)
    int if_assigned = 0;

    Xcp_Options.fd = XCP_FALSE;
#endif

    while ((opt = getopt(argc, argv, OPTION_STR)) != -1) {
        switch (opt) {
            case 'h':
            case '?':
                usage();
                break; /* never reached. */
#if defined(ETHER)
            case 't':
                if (p_assigned) {
                    printf("-t and -u options are mutual exclusive.\n");
                    exit(1);
                }
                p_assigned = XCP_TRUE;
                Xcp_Options->tcp = XCP_TRUE;
                break;
            case 'u':
                if (p_assigned) {
                    printf("-t and -u options are mutual exclusive.\n");
                    exit(1);
                }
                p_assigned = XCP_TRUE;
                Xcp_Options->tcp = XCP_FALSE;
                break;
            case '4':
                if (v_assigned) {
                    printf("-4 and -6 options are mutual exclusive.\n");
                    exit(1);
                }
                v_assigned = XCP_TRUE;
                Xcp_Options->ipv6 = XCP_FALSE;
                break;
            case '6':
                if (v_assigned) {
                    printf("-4 and -6 options are mutual exclusive.\n");
                    exit(1);
                }
                v_assigned = XCP_TRUE;
                Xcp_Options->ipv6 = XCP_TRUE;
                break;
            case 'p':
                Xcp_Options->port = atoi(optarg);
#elif defined(SOCKET_CAN)
            case 'f':
                Xcp_Options->fd = XCP_TRUE;
                break;
            case 'i':
                if_assigned = 1;
                strcpy(Xcp_Options->interface, optarg);
                break;
#endif
        }
    }

#if defined(SOCKET_CAN)
    if (!if_assigned) {
        strcpy(Xcp_Options.interface, DEFAULT_CAN_IF);
    }
#endif

}
#endif
