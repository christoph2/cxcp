/*
 * BlueParrot XCP
 *
 * (C) 2007-2024 by Christoph Schueler <github.com/Christoph2,
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
 *
 * TODO: Cleanup DAQ configuration
 *
 */

#if defined(_WIN32)
    #include <stdio.h>
#endif /* _WIN32 */

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_util.h"
/*!!! END-INCLUDE-SECTION !!!*/

/*
** Private Parameters for now.
*/
#define XCP_DAQ_QUEUE_SIZE (4)

/*
** Local Types.
*/
typedef struct {
    uint8_t len;
    uint8_t data[XCP_MAX_DTO];
} XcpDaq_OdtType;

typedef enum tagXcpDaq_AllocResultType {
    DAQ_ALLOC_OK,
    DAQ_ALLOC_ERR
} XcpDaq_AllocResultType;

typedef enum tagXcpDaq_AllocStateType {
    XCP_ALLOC_IDLE,
    XCP_AFTER_FREE_DAQ,
    XCP_AFTER_ALLOC_DAQ,
    XCP_AFTER_ALLOC_ODT,
    XCP_AFTER_ALLOC_ODT_ENTRY
} XcpDaq_AllocStateType;

typedef enum tagXcpDaq_AllocTransitionType {
    XCP_CALL_FREE_DAQ,
    XCP_CALL_ALLOC_DAQ,
    XCP_CALL_ALLOC_ODT,
    XCP_CALL_ALLOC_ODT_ENTRY
} XcpDaq_AllocTransitionype;

typedef enum tagXcpDaq_ListTransitionType {
    DAQ_LIST_TRANSITION_START,
    DAQ_LIST_TRANSITION_STOP
} XcpDaq_ListTransitionType;

#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
typedef struct tagXcpDaq_QueueType {
    uint8_t head;
    uint8_t tail;
    bool    overload;
} XcpDaq_QueueType;
#endif /* XCP_DAQ_ENABLE_QUEUING */

/*
** Local Function-like Macros.
*/
#define XCP_DAQ_MESSAGE_SIZE(msg) UINT16((((msg)->dlc) + sizeof(uint8_t)))

/*
** Local Function Prototypes.
*/
void            XcpDaq_PrintDAQDetails(void);
XCP_STATIC void XcpDaq_StartStopLists(XcpDaq_ListTransitionType transition);
XCP_STATIC void XcpDaq_InitMessageQueue(void);
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC bool                   XcpDaq_AllocValidateTransition(XcpDaq_AllocTransitionype transition);
XCP_STATIC XcpDaq_ListIntegerType XcpDaq_GetDynamicListCount(void);
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
bool XcpDaq_QueueEnqueue(uint16_t len, uint8_t const *data);
#endif /* XCP_DAQ_ENABLE_QUEUING */

/*
** Local Constants.
*/
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC const uint8_t XcpDaq_AllocTransitionTable[5][4] = {
    /* FREE_DAQ           ALLOC_DAQ             ALLOC_ODT ALLOC_ODT_ENTRY */
    /* ALLOC_IDLE*/ { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
    /* AFTER_FREE_DAQ  */
    { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
    /* AFTER_ALLOC_DAQ */
    { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_ERR) },
    /* AFTER_ALLOC_ODT */
    { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_OK)  },
    /* AFTER_ALLOC_ODT_ENTRY */
    { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK)  },
};
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

/*
** Local Variables.
*/

XCP_STATIC XcpDaq_EntityType XcpDaq_Entities[XCP_DAQ_MAX_DYNAMIC_ENTITIES];

#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC XcpDaq_AllocStateType        XcpDaq_AllocState;
XCP_STATIC XcpDaq_ListStateType         XcpDaq_ListState;
XCP_STATIC XcpDaq_ListConfigurationType XcpDaq_ListConfiguration;
XCP_STATIC XCP_DAQ_ENTITY_TYPE          XcpDaq_EntityCount = (XCP_DAQ_ENTITY_TYPE)0;
XCP_STATIC XCP_DAQ_ENTITY_TYPE          XcpDaq_ListCount   = (XCP_DAQ_ENTITY_TYPE)0;
XCP_STATIC XCP_DAQ_ENTITY_TYPE          XcpDaq_OdtCount    = (XCP_DAQ_ENTITY_TYPE)0;
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
XCP_STATIC XcpDaq_QueueType XcpDaq_Queue                         = { 0 };
XCP_STATIC XcpDaq_OdtType   XcpDaq_QueueDTOs[XCP_DAQ_QUEUE_SIZE] = { 0 };
#endif /* XCP_DAQ_ENABLE_QUEUING */

#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
XCP_STATIC uint8_t XcpDaq_ListForEvent[XCP_DAQ_MAX_EVENT_CHANNEL];
#else
    #error XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT option currently not supported
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

/*
**
** Global Functions.
**
*/

/*
** Dynamic DAQ related functions.
*/
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
Xcp_ReturnType XcpDaq_Free(void) {
    Xcp_ReturnType result = ERR_SUCCESS;

    XcpDaq_EntityCount = (XCP_DAQ_ENTITY_TYPE)0;
    XcpDaq_ListCount   = (XCP_DAQ_ENTITY_TYPE)0;
    XcpDaq_OdtCount    = (XCP_DAQ_ENTITY_TYPE)0;

    #if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
    XcpUtl_MemSet(XcpDaq_ListForEvent, UINT8(0), UINT32(sizeof(XcpDaq_ListForEvent[0]) * UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)));
    #endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

    if (XcpDaq_AllocValidateTransition(XCP_CALL_FREE_DAQ)) {
        XcpUtl_MemSet(
            XcpDaq_Entities, UINT8(0), UINT32(sizeof(XcpDaq_EntityType) * (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES)
        );
        XcpDaq_AllocState = XCP_AFTER_FREE_DAQ;
    } else {
        result = ERR_SEQUENCE; /* Never touched; function always succeeds. */
    }
    return result;
}

Xcp_ReturnType XcpDaq_Alloc(XcpDaq_ListIntegerType daqCount) {
    XCP_DAQ_ENTITY_TYPE idx;
    Xcp_ReturnType      result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidateTransition(XCP_CALL_ALLOC_DAQ)) {
    #if XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR == XCP_ON
        XcpDaq_Init();
    #endif /* XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR */
        result = ERR_SEQUENCE;
    } else {
        if ((XcpDaq_EntityCount + daqCount) <= (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_DAQ;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + daqCount); ++idx) {
                XcpDaq_Entities[idx].kind                   = UINT8(XCP_ENTITY_DAQ_LIST);
                XcpDaq_Entities[idx].entity.daqList.numOdts = (XcpDaq_ODTIntegerType)0;
            }
            XcpDaq_ListCount += daqCount;
            XcpDaq_EntityCount += daqCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
        }
    }
    return result;
}

Xcp_ReturnType XcpDaq_AllocOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtCount) {
    XCP_DAQ_ENTITY_TYPE idx;
    Xcp_ReturnType      result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidateTransition(XCP_CALL_ALLOC_ODT)) {
    #if XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR == XCP_ON
        XcpDaq_Init();
    #endif /* XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR */
        result = ERR_SEQUENCE;
    } else {
        if ((XcpDaq_EntityCount + odtCount) <= (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_ODT;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + odtCount); ++idx) {
                XcpDaq_Entities[idx].kind = UINT8(XCP_ENTITY_ODT);
            }
            XcpDaq_Entities[daqListNumber].entity.daqList.numOdts += odtCount;
            XcpDaq_Entities[daqListNumber].entity.daqList.firstOdt = XcpDaq_EntityCount;
            XcpDaq_OdtCount += odtCount;
            XcpDaq_EntityCount += odtCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
        }
    }
    return result;
}

Xcp_ReturnType XcpDaq_AllocOdtEntry(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntriesCount
) {
    XCP_DAQ_ENTITY_TYPE   idx;
    XcpDaq_ODTIntegerType odt;
    Xcp_ReturnType        result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidateTransition(XCP_CALL_ALLOC_ODT_ENTRY)) {
    #if XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR == XCP_ON
        XcpDaq_Init();
    #endif /* XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR */
        result = ERR_SEQUENCE;
    } else {
        if ((XcpDaq_EntityCount + odtEntriesCount) <= (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_ODT_ENTRY;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + odtEntriesCount); ++idx) {
                XcpDaq_Entities[idx].kind = UINT8(XCP_ENTITY_ODT_ENTRY);
            }
            odt = (XcpDaq_ODTIntegerType)(XcpDaq_Entities[daqListNumber].entity.daqList.firstOdt + UINT16(odtNumber));
            XcpDaq_Entities[odt].entity.odt.firstOdtEntry = XcpDaq_EntityCount;
            XcpDaq_Entities[odt].entity.odt.numOdtEntries = odtEntriesCount;
            XcpDaq_EntityCount += (XCP_DAQ_ENTITY_TYPE)odtEntriesCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
        }
    }
    return result;
}

XCP_STATIC bool XcpDaq_AllocValidateTransition(XcpDaq_AllocTransitionype transition) {
    /* printf("STATE: %u TRANSITION: %u\n", XcpDaq_AllocState, transition); */
    if (XcpDaq_AllocTransitionTable[XcpDaq_AllocState][transition] == UINT8(DAQ_ALLOC_OK)) {
        return (bool)XCP_TRUE;
    } else {
        return (bool)XCP_FALSE;
    }
}

XCP_STATIC XcpDaq_ListIntegerType XcpDaq_GetDynamicListCount(void) {
    return (XcpDaq_ListIntegerType)XcpDaq_ListCount;
}

XCP_DAQ_ENTITY_TYPE XcpDaq_GetDynamicDaqEntityCount(void) {
    return XcpDaq_EntityCount;
}
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

void XcpDaq_Init(void) {
#if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    XcpDaq_ListIntegerType idx = 0;

    XcpDaq_StopAllLists();
    XcpDaq_SetProcessorState(XCP_DAQ_STATE_STOPPED);

    for (idx = (XcpDaq_ListIntegerType)0; idx < XcpDaq_PredefinedListCount; ++idx) {
        XcpDaq_PredefinedListsState[idx].mode = UINT8(0);
    #if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
        XcpDaq_PredefinedListsState[idx].prescaler = UINT8(1);
        XcpDaq_PredefinedListsState[idx].counter   = UINT8(0);
    #endif /* XCP_DAQ_ENABLE_PRESCALER */
    }
#endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */

#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
    XcpDaq_AllocState = XCP_ALLOC_IDLE;
    (void)XcpDaq_Free();
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
    XcpDaq_QueueInit();
#endif
}

XcpDaq_ODTEntryType *XcpDaq_GetOdtEntry(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber
) {
    XcpDaq_ODTType const *odt = XCP_NULL;
    XcpDaq_ODTIntegerType idx = 0;

    /* printf("XcpDaq_GetOdtEntry(()\n"); */

    /* TODO: Range checking. */
    odt = XcpDaq_GetOdt(daqListNumber, odtNumber);
    idx = (XcpDaq_ODTIntegerType)(odt->firstOdtEntry + UINT16(odtEntryNumber));
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return &XcpDaq_Entities[idx].entity.odtEntry;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    return (XcpDaq_ODTEntryType *)&XcpDaq_PredefinedOdtEntries[idx];
    /* Predefined DAQs only */
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
#endif  // XCP_DAQ_ENABLE_DYNAMIC_LISTS
}

XcpDaq_ListConfigurationType const *XcpDaq_GetListConfiguration(XcpDaq_ListIntegerType daqListNumber) {
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON

#endif  // XCP_DAQ_ENABLE_DYNAMIC_LISTS

    // printf("XcpDaq_GetListConfiguration(%u)\n", daqListNumber);
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    XcpDaq_DynamicListType const *dl = &XcpDaq_Entities[daqListNumber].entity.daqList;

    XcpDaq_ListConfiguration.firstOdt = dl->firstOdt;
    XcpDaq_ListConfiguration.numOdts  = dl->numOdts;

    return &XcpDaq_ListConfiguration;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return &XcpDaq_PredefinedLists[daqListNumber];
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    return;
#endif
}

XcpDaq_ListStateType *XcpDaq_GetListState(XcpDaq_ListIntegerType daqListNumber) {
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON

#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

    /* printf("XcpDaq_GetListState() number: %u\n", daqListNumber); */
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return &XcpDaq_ListState;

#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return &XcpDaq_PredefinedListsState[daqListNumber];
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    if (daqListNumber >= XcpDaq_PredefinedListCount) {
        XcpDaq_DynamicListType *dl = &XcpDaq_Entities[daqListNumber].entity.daqList;

        XcpDaq_ListState.mode = dl->mode;

        return &XcpDaq_ListState;
    } else {
        return &XcpDaq_PredefinedListsState[daqListNumber];
    }
#endif
}

void XcpDaq_SetPointer(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber
) {
    Xcp_StateType *Xcp_State = XCP_NULL;

    Xcp_State                      = Xcp_GetState();
    Xcp_State->daqPointer.daqList  = daqListNumber;
    Xcp_State->daqPointer.odt      = odtNumber;
    Xcp_State->daqPointer.odtEntry = odtEntryNumber;
}

bool XcpDaq_ValidateConfiguration(void) {
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return (bool)((XcpDaq_EntityCount > (XCP_DAQ_ENTITY_TYPE)0) && (XcpDaq_ListCount > (XCP_DAQ_ENTITY_TYPE)0) &&
                  (XcpDaq_OdtCount > (XCP_DAQ_ENTITY_TYPE)0));
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return (bool)XCP_TRUE;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
#endif
}

XcpDaq_ListIntegerType XcpDaq_GetListCount(void) {
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return XcpDaq_GetDynamicListCount();
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return XcpDaq_PredefinedListCount;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    return XcpDaq_PredefinedListCount + XcpDaq_GetDynamicListCount();
#endif
}

bool XcpDaq_ValidateList(XcpDaq_ListIntegerType daqListNumber) {
    XcpDaq_ListConfigurationType const *daqList = XCP_NULL;
    XcpDaq_ODTType const               *odt     = XCP_NULL;
    bool                                result  = (bool)XCP_TRUE;
    XcpDaq_ODTIntegerType               numOdts = 0;
    uint8_t                             idx     = 0;

    if (daqListNumber > (XcpDaq_GetListCount() - UINT16(1))) {
        result = (bool)XCP_FALSE;
    } else {
        daqList = XcpDaq_GetListConfiguration(daqListNumber);
        numOdts = daqList->numOdts;
        if (numOdts == UINT8(0)) {
            result = (bool)XCP_FALSE;
        } else {
            result = (bool)XCP_FALSE;
            for (idx = UINT8(0); idx < numOdts; ++idx) {
                odt = XcpDaq_GetOdt(daqListNumber, idx);
                if (odt->numOdtEntries != UINT8(0)) {
                    result = (bool)XCP_TRUE;
                    break;
                }
            }
        }
    }
    return result;
}

bool XcpDaq_ValidateOdtEntry(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntry
) {
    XcpDaq_ListConfigurationType const *daqList = XCP_NULL;
    XcpDaq_ODTType const               *odt     = XCP_NULL;
    bool                                result  = (bool)XCP_TRUE;

    if (daqListNumber > (XcpDaq_GetListCount() - UINT16(1))) {
        result = (bool)XCP_FALSE;
    } else {
        daqList = XcpDaq_GetListConfiguration(daqListNumber);
        if (odtNumber > (daqList->numOdts - (XcpDaq_ODTIntegerType)1)) {
            result = (bool)XCP_FALSE;
        } else {
            odt = XcpDaq_GetOdt(daqListNumber, odtNumber);
            if (odtEntry > (odt->numOdtEntries - (XcpDaq_ODTEntryIntegerType)1)) {
                result = (bool)XCP_FALSE;
            }
        }
    }
    return result;
}

XcpDaq_EventType const *XcpDaq_GetEventConfiguration(uint16_t eventChannelNumber) {
    if (eventChannelNumber >= UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        return (XcpDaq_EventType const *)XCP_NULL;
    }
    return &XcpDaq_Events[eventChannelNumber];
}

void XcpDaq_AddEventChannel(XcpDaq_ListIntegerType daqListNumber, uint16_t eventChannelNumber) {
#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
    XcpDaq_ListForEvent[eventChannelNumber] = daqListNumber;
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */
}

/** @brief Triggers acquisition and transmission of DAQ lists.
 *
 *  @param eventChannelNumber   Number of event to trigger.
 */
void XcpDaq_TriggerEvent(uint8_t eventChannelNumber) {
    Xcp_StateType const                *state         = XCP_NULL;
    XcpDaq_ListIntegerType              daqListNumber = 0;
    XcpDaq_ODTIntegerType               odtIdx        = 0;
    XcpDaq_ODTIntegerType               pid           = 0;
    XcpDaq_ODTEntryIntegerType          odtEntryIdx   = 0;
    XcpDaq_ODTType const               *odt           = XCP_NULL;
    XcpDaq_ODTEntryType                *entry         = XCP_NULL;
    XcpDaq_ListConfigurationType const *listConf      = XCP_NULL;
    uint16_t                            offset        = UINT16(0);
    uint8_t                             data[k]       = { 0 };
#if XCP_DAQ_ENABLE_TIMESTAMPING == XCP_ON
    XcpDaq_ListStateType *listState        = XCP_NULL;
    uint32_t              timestamp        = UINT32(0);
    bool                  insert_timestamp = XCP_FALSE;

    timestamp = XcpHw_GetTimerCounter();
#endif /* XCP_DAQ_ENABLE_TIMESTAMPING */
    state = Xcp_GetState();
    if (state->daqProcessor.state != XCP_DAQ_STATE_RUNNING) {
        return;
    }
    if (eventChannelNumber >= UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        return;
    }

#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
    daqListNumber = XcpDaq_ListForEvent[eventChannelNumber];
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

    if (!XcpDaq_GetFirstPid(daqListNumber, &pid)) {
        return;
    }
    listConf = XcpDaq_GetListConfiguration(daqListNumber);
#if XCP_DAQ_ENABLE_TIMESTAMPING == XCP_ON
    listState = XcpDaq_GetListState(daqListNumber);
    if ((listState->mode & XCP_DAQ_LIST_MODE_TIMESTAMP) == XCP_DAQ_LIST_MODE_TIMESTAMP) {
        insert_timestamp = XCP_TRUE;
    }
#endif /* XCP_DAQ_ENABLE_TIMESTAMPING */
    for (odtIdx = (XcpDaq_ODTIntegerType)0; odtIdx < listConf->numOdts; ++odtIdx) {
        offset = UINT16(0);
        odt    = XcpDaq_GetOdt(daqListNumber, odtIdx);

        data[0] = pid; /* Absolute ODT number. */
        offset += UINT16(1);
#if XCP_DAQ_ENABLE_TIMESTAMPING == XCP_ON
        if ((odtIdx == (XcpDaq_ODTIntegerType)0) && (insert_timestamp == XCP_TRUE)) {
            XcpDaq_CopyMemory(&data[offset], (void *)&timestamp, XCP_DAQ_TIMESTAMP_SIZE);
            offset += XCP_DAQ_TIMESTAMP_SIZE;
        }
#endif /* XCP_DAQ_ENABLE_TIMESTAMPING */
        for (odtEntryIdx = (XcpDaq_ODTEntryIntegerType)0; odtEntryIdx < odt->numOdtEntries; ++odtEntryIdx) {
            entry = XcpDaq_GetOdtEntry(daqListNumber, odtIdx, odtEntryIdx);
            // printf("\tAddress: 0x%08x Length: %d\n", entry->mta.address,
            // entry->length);
            if (odtEntryIdx == (XcpDaq_ODTEntryIntegerType)0) {
            }
            XCP_ASSERT_LE(entry->length, (unsigned int)(XCP_MAX_DTO - offset));
            XcpDaq_CopyMemory(&data[offset], (void *)entry->mta.address, entry->length);
            offset += entry->length;
        }
        pid++;
        XcpDaq_QueueEnqueue(offset, data);
        //        XcpUtl_Hexdump(data, offset);
    }
    XcpDaq_TransmitDtos();
}

/** @brief Copies bytes from a source memory area to a destination memory area,
 *   where both areas may not overlap.
 *  @param[out] dst  The memory area to copy to.
 *  @param[in]  src  The memory area to copy from.
 *  @param[in]  len  The number of bytes to copy
 */
void XcpDaq_CopyMemory(void *dst, void const *src, uint32_t len) {
    XcpUtl_MemCopy(dst, src, len);
}

void XcpDaq_GetProperties(uint8_t *properties) {
    *properties = UINT8(0);
#if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
    *properties |= XCP_DAQ_PROP_PRESCALER_SUPPORTED;
#endif /*XCP_DAQ_ENABLE_PRESCALER */

#if XCP_DAQ_ENABLE_TIMESTAMPING == XCP_ON
    *properties |= XCP_DAQ_PROP_TIMESTAMP_SUPPORTED;
#endif /*XCP_DAQ_ENABLE_TIMESTAMPING */

#if (XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_NONE) || (XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_STATIC)
    *properties |= UINT8(XCP_DAQ_CONFIG_TYPE_STATIC);
#elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_DYNAMIC
    *properties |= UINT8(XCP_DAQ_CONFIG_TYPE_NONE);
#endif
}

void XcpDaq_SetProcessorState(XcpDaq_ProcessorStateType state) {
    Xcp_StateType *stateVar = XCP_NULL;

    stateVar = Xcp_GetState();
    XCP_DAQ_ENTER_CRITICAL();
    stateVar->daqProcessor.state = state;
    XCP_DAQ_LEAVE_CRITICAL();
}

void XcpDaq_StartSelectedLists(void) {
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_START);
}

void XcpDaq_StopSelectedLists(void) {
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_STOP);
}

void XcpDaq_StopAllLists(void) {
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_STOP);
}

void XcpDaq_TransmitDtos(void) {
    uint16_t len;
    uint8_t *dataOut = Xcp_GetDtoOutPtr();
    while (!XcpDaq_QueueEmpty()) {
        XcpDaq_QueueDequeue(&len, dataOut);
        Xcp_SetDtoOutLen(len);
        Xcp_SendDto();
    }
}

/*
** Local Functions.
*/
XcpDaq_ODTType const *XcpDaq_GetOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber) {
    XcpDaq_ListConfigurationType const *dl  = XCP_NULL;
    XcpDaq_ODTIntegerType               idx = 0;

    dl  = XcpDaq_GetListConfiguration(daqListNumber);
    idx = (XcpDaq_ODTIntegerType)dl->firstOdt + odtNumber;
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return &XcpDaq_Entities[idx].entity.odt;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return &XcpDaq_PredefinedOdts[idx];
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
#endif
}

XCP_STATIC void XcpDaq_StartStopLists(XcpDaq_ListTransitionType transition) {
    XcpDaq_ListIntegerType idx   = 0;
    XcpDaq_ListStateType  *entry = XCP_NULL;

    for (idx = (XcpDaq_ListIntegerType)0; idx < XcpDaq_GetListCount(); ++idx) {
        entry = XcpDaq_GetListState(idx);
        if ((entry->mode & XCP_DAQ_LIST_MODE_SELECTED) == XCP_DAQ_LIST_MODE_SELECTED) {
            if (transition == DAQ_LIST_TRANSITION_START) {
                entry->mode |= XCP_DAQ_LIST_MODE_STARTED;
                /* printf("Started DAQ list #%u\n", idx); */
            } else if (transition == DAQ_LIST_TRANSITION_STOP) {
                entry->mode &= UINT8(~XCP_DAQ_LIST_MODE_STARTED);
                /* printf("Stopped DAQ list #%u\n", idx); */
            } else {
                /* Do nothing (to keep MISRA happy). */
            }
            /*
             *  The slave has to reset the SELECTED flag in the mode at
             * GET_DAQ_LIST_MODE as soon as the related START_STOP_SYNCH or
             * SET_REQUEST have been acknowledged.
             */
            entry->mode &= UINT8(~XCP_DAQ_LIST_MODE_SELECTED);
        }
    }
}

bool XcpDaq_GetFirstPid(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType *firstPID) {
    XcpDaq_ListIntegerType              listIdx = 0;
    XcpDaq_ListConfigurationType const *daqList = XCP_NULL;
    bool                                result  = (bool)XCP_TRUE;
    XcpDaq_ODTIntegerType               tmp     = (XcpDaq_ODTIntegerType)0;

    if (daqListNumber > (XcpDaq_GetListCount() - UINT16(1))) {
        result = (bool)XCP_FALSE;
    } else {
        for (listIdx = UINT16(0); listIdx < daqListNumber; ++listIdx) {
            daqList = XcpDaq_GetListConfiguration(listIdx);
            tmp += daqList->numOdts;
        }
    }
    *firstPID = tmp;
    return result;
}

/*
**  Debugging / Testing interface.
*/
#if XCP_BUILD_TYPE == XCP_DEBUG_BUILD

void XcpDaq_GetCounts(XCP_DAQ_ENTITY_TYPE *entityCount, XCP_DAQ_ENTITY_TYPE *listCount, XCP_DAQ_ENTITY_TYPE *odtCount) {
    *entityCount = XcpDaq_EntityCount;
    *listCount   = XcpDaq_ListCount;
    *odtCount    = XcpDaq_OdtCount;
}

uint16_t XcpDaq_TotalDynamicEntityCount(void) {
    return UINT16(XCP_DAQ_MAX_DYNAMIC_ENTITIES);
}

XcpDaq_EntityType *XcpDaq_GetDynamicEntities(void) {
    return &XcpDaq_Entities[0];
}

XcpDaq_EntityType *XcpDaq_GetDynamicEntity(uint16_t num) {
    return &XcpDaq_Entities[num];
}

void XcpDaq_QueueGetVar(XcpDaq_QueueType *var) {
    XcpUtl_MemCopy(var, &XcpDaq_Queue, sizeof(XcpDaq_QueueType));
}

#endif /* XCP_BUILD_TYPE */

// #if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON

#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
void XcpDaq_QueueInit(void) {
    uint8_t idx;

    XcpDaq_Queue.head = XcpDaq_Queue.tail = UINT8(0);
    XcpDaq_Queue.overload                 = (bool)XCP_FALSE;
    for (idx = UINT8(0); idx < UINT8(XCP_DAQ_QUEUE_SIZE); ++idx) {
        XcpUtl_ZeroMem(&XcpDaq_QueueDTOs[idx], sizeof(Xcp_PduType));
    }
}

XCP_STATIC bool XcpDaq_QueueFull(void) {
    return ((XcpDaq_Queue.head + UINT8(1)) % UINT8(XCP_DAQ_QUEUE_SIZE + 1)) == XcpDaq_Queue.tail;
}

bool XcpDaq_QueueEmpty(void) {
    return XcpDaq_Queue.head == XcpDaq_Queue.tail;
}

bool XcpDaq_QueueEnqueue(uint16_t len, uint8_t const *data) {
    if (XcpDaq_QueueFull()) {
        XcpDaq_Queue.overload = (bool)XCP_TRUE;
        return (bool)XCP_FALSE;
    }

    XcpDaq_QueueDTOs[XcpDaq_Queue.head].len = len;

    XCP_ASSERT_LE(len, XCP_MAX_DTO);
    XcpUtl_MemCopy(XcpDaq_QueueDTOs[XcpDaq_Queue.head].data, data, len);
    XcpDaq_Queue.head = (XcpDaq_Queue.head + UINT8(1)) % UINT8(XCP_DAQ_QUEUE_SIZE + 1);
    return (bool)XCP_TRUE;
}

bool XcpDaq_QueueDequeue(uint16_t *len, uint8_t *data) {
    uint16_t dto_len;

    if (XcpDaq_QueueEmpty()) {
        return (bool)XCP_FALSE;
    }
    dto_len = XcpDaq_QueueDTOs[XcpDaq_Queue.tail].len;
    XCP_ASSERT_LE(dto_len, XCP_MAX_DTO);
    *len = dto_len;
    XcpUtl_MemCopy(data, XcpDaq_QueueDTOs[XcpDaq_Queue.tail].data, dto_len);
    XcpDaq_Queue.tail = (XcpDaq_Queue.tail + UINT8(1)) % UINT8(XCP_DAQ_QUEUE_SIZE + 1);
    return (bool)XCP_TRUE;
}

// #endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */
#endif /* XCP_DAQ_ENABLE_QUEUING */
