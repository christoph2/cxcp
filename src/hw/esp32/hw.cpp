/*
 * BlueParrot XCP
 *
 * (C) 2026 by Christoph Schueler <github.com/Christoph2,
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

#include <stdint.h>

#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"
#include "xcp_threads.h"
/*!!! END-INCLUDE-SECTION !!!*/

typedef struct tagHwStateType {
    uint32_t StartingTime;
} HwStateType;

static const char *TAG = "xcp-esp32-hw";

static HwStateType       HwState                        = { 0 };
static SemaphoreHandle_t XcpHw_Locks[XCP_HW_LOCK_COUNT] = { 0 };
static volatile bool     XcpHw_Initialized              = false;
static volatile bool     XcpThrd_ShuttingDown           = false;
static TaskHandle_t      XcpHw_XcpTask                  = NULL;
static TaskHandle_t      XcpHw_TlTask                   = NULL;
static TaskHandle_t      XcpHw_TermTask                 = NULL;
static TaskHandle_t      XcpHw_WdtTask                  = NULL;

static void XcpHw_InitLocks(void);

static void XcpHw_DeinitLocks(void);

static void XcpHw_TaskTrampoline(void *param, void *(*entry)(void *));

static void XcpHw_XcpTaskEntry(void *param);

static void XcpHw_TlTaskEntry(void *param);

static void XcpHw_TermTaskEntry(void *param);

static void XcpHw_WatchdogTaskEntry(void *param);

static void XcpHw_TaskTrampoline(void *param, void *(*entry)(void *)) {
    (void)entry(param);
    vTaskDelete(NULL);
}

static void XcpHw_XcpTaskEntry(void *param) {
    XcpHw_TaskTrampoline(param, Xcp_Thread);
}

static void XcpHw_TlTaskEntry(void *param) {
    XcpHw_TaskTrampoline(param, XcpTl_Thread);
}

static void XcpHw_TermTaskEntry(void *param) {
    XcpHw_TaskTrampoline(param, XcpTerm_Thread);
}

static void XcpHw_WatchdogTaskEntry(void *param) {
    XCP_UNREFERENCED_PARAMETER(param);

    if (esp_task_wdt_add(NULL) != ESP_OK) {
        vTaskDelete(NULL);
        return;
    }

    for (;;) {
        (void)esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" {
    void XcpHw_ParseCommandLineOptions(int argc, char **argv, Xcp_OptionsType *options) {
        (void)argc;
        (void)argv;
        (void)options;
    }

    void XcpHw_Init(void) {
        if (XcpHw_Initialized) {
            return;
        }
        XcpHw_InitLocks();
        HwState.StartingTime = (uint32_t)(esp_timer_get_time() / 1000LL);
        XcpHw_Initialized    = true;
    }

    void XcpHw_Deinit(void) {
        if (!XcpHw_Initialized) {
            return;
        }
        XcpHw_DeinitLocks();
        XcpHw_Initialized = false;
    }

    uint32_t XcpHw_GetTimerCounter(void) {
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
        return (uint32_t)esp_timer_get_time();
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
        return (uint32_t)(esp_timer_get_time() / 1000LL);
#else
    #error Timestamp-unit not supported.
#endif
    }

    uint32_t XcpHw_GetTimerCounterMS(void) {
        return (uint32_t)(esp_timer_get_time() / 1000LL);
    }

    void XcpHw_AcquireLock(uint8_t lockIdx) {
        if (lockIdx >= XCP_HW_LOCK_COUNT) {
            return;
        }
        if (XcpHw_Locks[lockIdx] != NULL) {
            xSemaphoreTakeRecursive(XcpHw_Locks[lockIdx], portMAX_DELAY);
        }
    }

    void XcpHw_ReleaseLock(uint8_t lockIdx) {
        if (lockIdx >= XCP_HW_LOCK_COUNT) {
            return;
        }
        if (XcpHw_Locks[lockIdx] != NULL) {
            xSemaphoreGiveRecursive(XcpHw_Locks[lockIdx]);
        }
    }

    void XcpHw_ErrorMsg(char * const function, int errorCode) {
        ESP_LOGE(TAG, "%s failed with %d", function, errorCode);
    }

    void XcpHw_Sleep(uint64_t usec) {
        TickType_t ticks = pdMS_TO_TICKS((usec + 999ULL) / 1000ULL);

        if (ticks == 0U) {
            ticks = 1U;
        }
        vTaskDelay(ticks);
    }

    void XcpThrd_EnableAsyncCancellation(void) {
    }

    bool XcpThrd_IsShuttingDown(void) {
        return XcpThrd_ShuttingDown;
    }

    void XcpThrd_ShutDown(void) {
        XcpThrd_ShuttingDown = true;
    }

    void XcpThrd_Exit(void) {
        vTaskDelete(NULL);
    }

    void *Xcp_Thread(void *param) {
        (void)param;

        while (!XcpThrd_IsShuttingDown()) {
            Xcp_MainFunction();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        return NULL;
    }

    void *XcpTerm_Thread(void *param) {
        (void)param;

        return NULL;
    }

    void XcpThrd_RunThreads(void) {
        BaseType_t ok;

        XcpThrd_ShuttingDown = false;

        ok = xTaskCreatePinnedToCore(
            XcpHw_XcpTaskEntry, "xcp-core", CONFIG_EXAMPLE_XCP_TASK_STACK_SIZE, NULL, CONFIG_EXAMPLE_XCP_TASK_PRIORITY,
            &XcpHw_XcpTask, 0
        );
        if (ok != pdPASS) {
            ESP_LOGE(TAG, "Failed to create XCP task");
        }

        ok = xTaskCreatePinnedToCore(
            XcpHw_TlTaskEntry, "xcp-tl", CONFIG_EXAMPLE_XCP_TL_TASK_STACK_SIZE, NULL, CONFIG_EXAMPLE_XCP_TL_TASK_PRIORITY,
            &XcpHw_TlTask, 1
        );
        if (ok != pdPASS) {
            ESP_LOGE(TAG, "Failed to create TL task");
        }

        ok = xTaskCreatePinnedToCore(
            XcpHw_TermTaskEntry, "xcp-term", CONFIG_EXAMPLE_XCP_TERM_TASK_STACK_SIZE, NULL, CONFIG_EXAMPLE_XCP_TERM_TASK_PRIORITY,
            &XcpHw_TermTask, 0
        );
        if (ok != pdPASS) {
            ESP_LOGW(TAG, "Failed to create terminal task");
        }

        ok = xTaskCreatePinnedToCore(XcpHw_WatchdogTaskEntry, "xcp-wdt", 2048, NULL, 10, &XcpHw_WdtTask, 0);
        if (ok != pdPASS) {
            ESP_LOGW(TAG, "Failed to create watchdog task");
        }
    }
} /* extern "C" */

static void XcpHw_InitLocks(void) {
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        XcpHw_Locks[idx] = xSemaphoreCreateRecursiveMutex();
        if (XcpHw_Locks[idx] == NULL) {
            ESP_LOGE(TAG, "Failed to create lock %u", idx);
        }
    }
}

static void XcpHw_DeinitLocks(void) {
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        if (XcpHw_Locks[idx] != NULL) {
            vSemaphoreDelete(XcpHw_Locks[idx]);
            XcpHw_Locks[idx] = NULL;
        }
    }
}
