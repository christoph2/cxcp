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


#include <pthread.h>
#include <signal.h>

#include "xcp.h"
#include "terminal.h"
#include "app_config.h"


//////////////

pthread_t threads[NUM_THREADS];
Xcp_OptionsType Xcp_Options = {0};

void parse_options(int argc, char ** argv, Xcp_OptionsType * options);
//////////////

void * XcpTl_Thread(void * param);
void * Xcp_Thread(void * param);

void AppTask(void);

int main(int argc, char **argv)
{
    parse_options(argc, argv, &Xcp_Options);

    FlsEmu_Init(&FlsEmu_Config);

    Xcp_Init();
    Xcp_DisplayInfo();

    pthread_create(&threads[UI_THREAD], NULL, &XcpTerm_Thread, NULL);
    pthread_create(&threads[TL_THREAD], NULL, &XcpTl_Thread, NULL);
    pthread_create(&threads[XCP_THREAD], NULL, &Xcp_Thread, NULL);
    pthread_join(threads[UI_THREAD], NULL);
    pthread_kill(threads[TL_THREAD], SIGINT);
    pthread_kill(threads[XCP_THREAD], SIGINT);

    FlsEmu_DeInit();
    XcpHw_Deinit();
    XcpTl_DeInit();

    return 0;
}

void * Xcp_Thread(void * param)
{
    XCP_UNREFERENCED_PARAMETER(param);
    XCP_FOREVER {
        Xcp_MainFunction();
    }
    return NULL;
}
