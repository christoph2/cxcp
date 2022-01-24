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

#include <stdlib.h>

#include "xcp.h"

#if defined(_WIN32)

#else
#include <unistd.h>
#include <string.h>

#if defined(TP_ETHER)
static const char OPTION_STR[] = "htu46p:";
#elif defined(TP_CAN)
static const char OPTION_STR[] = "hi:f";
#endif


#endif


#if defined(_WIN32)
void parse_options(int argc, char ** argv, Xcp_OptionsType * options)
{
    int idx;
    char * arg;
#if defined(TP_ETHER)
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
#elif defined(TP_BLUETOOTH)

#else   /* defined(KVASER_CAN)*/

#endif
}
#else

void usage(void)
{
    printf("\nparameter summary: \n");
#if defined(TP_ETHER)
    printf("-h\t  this message.\n");
    printf("-t\t  TCP\t\t  default: TRUE\n");
    printf("-u\t  UDP\t\t  default: FALSE\n");
    printf("-4\t  IPv4\t\t  default: TRUE\n");
    printf("-6\t  IPv6\t\t  default: FALSE\n");
    printf("-p <port> port to listen  default: 5555\n");
#elif defined(TP_CAN)
    printf("-h\tthis message.\n");
    printf("-f\t\tuse CAN-FD\t\tdefault: FALSE\n");
    printf("-i <if-name>\tinterface to use\tdefault: vcan0\n");
#endif
    exit(0);
}


void parse_options(int argc, char ** argv, Xcp_OptionsType * options)
{
    int opt;
    int res;

#if defined(TP_ETHER)
    int p_assigned = 0;
    int v_assigned = 0;

    options->tcp = XCP_TRUE;
    options->ipv6 = XCP_FALSE;
    options->port = XCP_ETH_DEFAULT_PORT;
#elif defined(TP_CAN)
    int if_assigned = 0;
    options->fd = XCP_FALSE;
#endif

    while ((opt = getopt(argc, argv, OPTION_STR)) != -1) {
        switch (opt) {
            case 'h':
            case '?':
                usage();
                break; /* never reached. */
#if defined(TP_ETHER)
            case 't':
                if (p_assigned) {
                    printf("-t and -u options are mutual exclusive.\n");
                    exit(1);
                }
                p_assigned = XCP_TRUE;
                options->tcp = XCP_TRUE;
                break;
            case 'u':
                if (p_assigned) {
                    printf("-t and -u options are mutual exclusive.\n");
                    exit(1);
                }
                p_assigned = XCP_TRUE;
                options->tcp = XCP_FALSE;
                break;
            case '4':
                if (v_assigned) {
                    printf("-4 and -6 options are mutual exclusive.\n");
                    exit(1);
                }
                v_assigned = XCP_TRUE;
                options->ipv6 = XCP_FALSE;
                break;
            case '6':
                if (v_assigned) {
                    printf("-4 and -6 options are mutual exclusive.\n");
                    exit(1);
                }
                v_assigned = XCP_TRUE;
                options->ipv6 = XCP_TRUE;
                break;
            case 'p':
                options->port = atoi(optarg);
#elif defined(TP_CAN)
            case 'f':
                options->fd = XCP_TRUE;
                break;
            case 'i':
                if_assigned = 1;
                strcpy(options->interface, optarg);
                break;
#endif
        }
    }

#if defined(TP_CAN)
    if (!if_assigned) {
        strcpy(options->interface, XCP_SOCKET_CAN_DEFAULT_IF);
    }
#endif

}
#endif
