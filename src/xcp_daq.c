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

#if defined(_WIN32)
#include <stdio.h>
#endif /* _WIN32 */

#include "xcp.h"
#include "xcp_util.h"


/*
** Local Types.
*/
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

/*
** Local Function-like Macros.
*/
#define XCP_DAQ_MESSAGE_SIZE(msg)   UINT16(((msg->dlc) + sizeof(uint8_t)))


/*
** Local Function Prototypes.
*/
XCP_STATIC XcpDaq_ODTType const * XcpDaq_GetOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber);
void XcpDaq_PrintDAQDetails(void);
XCP_STATIC void XcpDaq_StartStopLists(XcpDaq_ListTransitionType transition);
XCP_STATIC void XcpDaq_InitMessageQueue(void);
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC bool XcpDaq_AllocValidateTransition(XcpDaq_AllocTransitionype transition);
XCP_STATIC XcpDaq_ListIntegerType XcpDaq_GetDynamicListCount(void);
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

/*
** Local Constants.
*/
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC const uint8_t XcpDaq_AllocTransitionTable[5][4] = {
                            /* FREE_DAQ           ALLOC_DAQ             ALLOC_ODT             ALLOC_ODT_ENTRY    */
/* ALLOC_IDLE*/             {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
/* AFTER_FREE_DAQ  */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK) , UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
/* AFTER_ALLOC_DAQ */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK) , UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_ERR) },
/* AFTER_ALLOC_ODT */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_OK)  },
/* AFTER_ALLOC_ODT_ENTRY */ {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK)  },
};
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

/*
** Local Variables.
*/

XCP_STATIC uint8_t XcpDaq_DtoBuffer[XCP_DAQ_DTO_BUFFER_SIZE + 1];
XCP_STATIC XcpDaq_DtoBufferStateType XcpDaq_DtoBufferState;

#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC XcpDaq_AllocStateType XcpDaq_AllocState;
XCP_STATIC XcpDaq_EntityType XcpDaq_Entities[XCP_DAQ_MAX_DYNAMIC_ENTITIES];
XCP_STATIC XCP_DAQ_ENTITY_TYPE XcpDaq_EntityCount = (XCP_DAQ_ENTITY_TYPE)0;
XCP_STATIC XCP_DAQ_ENTITY_TYPE XcpDaq_ListCount = (XCP_DAQ_ENTITY_TYPE)0;
XCP_STATIC XCP_DAQ_ENTITY_TYPE XcpDaq_OdtCount = (XCP_DAQ_ENTITY_TYPE)0;

XCP_STATIC XcpDaq_ListStateType XcpDaq_ListState ;
XCP_STATIC XcpDaq_ListConfigurationType XcpDaq_ListConfiguration;
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT  == XCP_OFF
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
Xcp_ReturnType XcpDaq_Free(void)
{
    Xcp_ReturnType result = ERR_SUCCESS;

    XcpDaq_EntityCount = (XCP_DAQ_ENTITY_TYPE)0;
    XcpDaq_ListCount = (XCP_DAQ_ENTITY_TYPE)0;
    XcpDaq_OdtCount = (XCP_DAQ_ENTITY_TYPE)0;

#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT  == XCP_OFF
    XcpUtl_MemSet(XcpDaq_ListForEvent, UINT8(0), UINT32(sizeof(XcpDaq_ListForEvent[0]) * UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)));
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

    if (XcpDaq_AllocValidateTransition(XCP_CALL_FREE_DAQ)) {
        XcpUtl_MemSet(XcpDaq_Entities, UINT8(0), UINT32(sizeof(XcpDaq_EntityType) * (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES));
        XcpDaq_AllocState = XCP_AFTER_FREE_DAQ;
    } else {
        result = ERR_SEQUENCE;  /* Never touched; function always succeeds. */
    }
    return result;
}

Xcp_ReturnType XcpDaq_Alloc(XcpDaq_ListIntegerType daqCount)
{
    XCP_DAQ_ENTITY_TYPE idx;
    Xcp_ReturnType result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidateTransition(XCP_CALL_ALLOC_DAQ)) {
#if XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR == XCP_ON
        XcpDaq_Init();
#endif /* XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR */
        result = ERR_SEQUENCE;
    } else {
        if ((XcpDaq_EntityCount + daqCount) <= (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_DAQ;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + daqCount); ++idx) {
                XcpDaq_Entities[idx].kind = UINT8(XCP_ENTITY_DAQ_LIST);
                XcpDaq_Entities[idx].entity.daqList.numOdts = (XcpDaq_ODTIntegerType)0;
            }
            XcpDaq_ListCount += (XCP_DAQ_ENTITY_TYPE)daqCount;
            XcpDaq_EntityCount += (XCP_DAQ_ENTITY_TYPE)daqCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
        }
    }
    return result;
}

Xcp_ReturnType XcpDaq_AllocOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtCount)
{
    XCP_DAQ_ENTITY_TYPE idx;
    Xcp_ReturnType result = ERR_SUCCESS;

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
            XcpDaq_OdtCount += (XCP_DAQ_ENTITY_TYPE)odtCount;
            XcpDaq_EntityCount += (XCP_DAQ_ENTITY_TYPE)odtCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
        }

    }
    return result;
}

Xcp_ReturnType XcpDaq_AllocOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntriesCount)
{
    XCP_DAQ_ENTITY_TYPE idx;
    XcpDaq_ODTIntegerType odt;
    Xcp_ReturnType result = ERR_SUCCESS;

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


XCP_STATIC bool XcpDaq_AllocValidateTransition(XcpDaq_AllocTransitionype transition)
{
    /* printf("STATE: %u TRANSITION: %u\n", XcpDaq_AllocState, transition); */
    if (XcpDaq_AllocTransitionTable[XcpDaq_AllocState][transition] == UINT8(DAQ_ALLOC_OK)) {
        return (bool)XCP_TRUE;
    } else {
        return (bool)XCP_FALSE;
    }
}


XCP_STATIC XcpDaq_ListIntegerType XcpDaq_GetDynamicListCount(void)
{
    return (XcpDaq_ListIntegerType)XcpDaq_ListCount;
}
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

void XcpDaq_Init(void)
{
#if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    XcpDaq_ListIntegerType idx;

    XcpDaq_StopAllLists();
    XcpDaq_SetProcessorState(XCP_DAQ_STATE_STOPPED);

    for (idx = (XcpDaq_ListIntegerType)0; idx < XcpDaq_PredefinedListCount; ++idx) {
        XcpDaq_PredefinedListsState[idx].mode = UINT8(0);
#if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
        XcpDaq_PredefinedListsState[idx].prescaler = UINT8(1);
        XcpDaq_PredefinedListsState[idx].counter = UINT8(0);
#endif /* XCP_DAQ_ENABLE_PRESCALER */
    }
#endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */

#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
    XcpDaq_AllocState = XCP_ALLOC_IDLE;
    (void)XcpDaq_Free();
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

    XcpDaq_InitMessageQueue();
}

XcpDaq_ODTEntryType * XcpDaq_GetOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber)
{
    XcpDaq_ODTType const * odt;
    XcpDaq_ODTIntegerType idx;

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
#endif // XCP_DAQ_ENABLE_DYNAMIC_LISTS
}

XcpDaq_ListConfigurationType const * XcpDaq_GetListConfiguration(XcpDaq_ListIntegerType daqListNumber)
{
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON


#endif // XCP_DAQ_ENABLE_DYNAMIC_LISTS

    //printf("XcpDaq_GetListConfiguration(%u)\n", daqListNumber);
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    XcpDaq_DynamicListType * dl = &XcpDaq_Entities[daqListNumber].entity.daqList;

    XcpDaq_ListConfiguration.firstOdt = dl->firstOdt;
    XcpDaq_ListConfiguration.numOdts = dl->numOdts;

    return &XcpDaq_ListConfiguration;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return &XcpDaq_PredefinedLists[daqListNumber];
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    return;
#endif
}

XcpDaq_ListStateType * XcpDaq_GetListState(XcpDaq_ListIntegerType daqListNumber)
{
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
        XcpDaq_DynamicListType * dl = &XcpDaq_Entities[daqListNumber].entity.daqList;

        XcpDaq_ListState.mode = dl->mode;

        return &XcpDaq_ListState;
    } else {
        return &XcpDaq_PredefinedListsState[daqListNumber];
    }
#endif
}

void XcpDaq_SetPointer(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber)
{
    Xcp_StateType * Xcp_State;

    Xcp_State = Xcp_GetState();
    Xcp_State->daqPointer.daqList = daqListNumber;
    Xcp_State->daqPointer.odt = odtNumber;
    Xcp_State->daqPointer.odtEntry = odtEntryNumber;
}

bool XcpDaq_ValidateConfiguration(void)
{
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return (bool)((XcpDaq_EntityCount > (XCP_DAQ_ENTITY_TYPE)0) && (XcpDaq_ListCount > (XCP_DAQ_ENTITY_TYPE)0) &&  (XcpDaq_OdtCount > (XCP_DAQ_ENTITY_TYPE)0));
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return (bool)XCP_TRUE;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
#endif
}

XcpDaq_ListIntegerType XcpDaq_GetListCount(void)
{
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

bool XcpDaq_ValidateList(XcpDaq_ListIntegerType daqListNumber)
{
    XcpDaq_ListConfigurationType const * daqList;
    XcpDaq_ODTType const * odt;
    bool result = (bool)XCP_TRUE;
    XcpDaq_ODTIntegerType numOdts;
    uint8_t idx;

    if (daqListNumber > (XcpDaq_GetListCount() - UINT16(1))) {
        result = (bool)XCP_FALSE;
    } else {
        daqList = XcpDaq_GetListConfiguration(daqListNumber);
        numOdts = daqList->numOdts ;
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

bool XcpDaq_ValidateOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntry)
{
    XcpDaq_ListConfigurationType const * daqList;
    XcpDaq_ODTType const * odt;
    bool result = (bool)XCP_TRUE;

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

void XcpDaq_MainFunction(void)
{
    Xcp_StateType const * Xcp_State;
    XcpDaq_ListIntegerType listCount;

    Xcp_State = Xcp_GetState();
    if (Xcp_State->daqProcessor.state == XCP_DAQ_STATE_RUNNING) {
        listCount = XcpDaq_GetListCount();   /* Check global state for DAQ/STIM running. */
        /* printf("%u Active DAQ list(s).\n", listCount); */
    }
}

XcpDaq_EventType const * XcpDaq_GetEventConfiguration(uint16_t eventChannelNumber)
{
    if (eventChannelNumber >= UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        return (XcpDaq_EventType const *)XCP_NULL;
    }
    return &XcpDaq_Events[eventChannelNumber];
}


void XcpDaq_AddEventChannel(XcpDaq_ListIntegerType daqListNumber, uint16_t eventChannelNumber)
{
#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT  == XCP_OFF
    XcpDaq_ListForEvent[eventChannelNumber] = daqListNumber;
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */
}


/** @brief Triggers acquisition and transmission of DAQ lists.
 *
 *  @param eventChannelNumber   Number of event to trigger.
 */
void XcpDaq_TriggerEvent(uint8_t eventChannelNumber)
{
    Xcp_StateType const * state;
    XcpDaq_ListIntegerType daqListNumber;
    XcpDaq_ODTIntegerType odtIdx;
    XcpDaq_ODTIntegerType pid;
    XcpDaq_ODTEntryIntegerType odtEntryIdx;
    XcpDaq_ODTType const * odt;
    XcpDaq_ODTEntryType * entry;
    XcpDaq_ListConfigurationType const * listConf;

    state = Xcp_GetState();
    if (state->daqProcessor.state != XCP_DAQ_STATE_RUNNING) {
        return;
    }
    if (eventChannelNumber >= UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        return;
    }

#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT  == XCP_OFF
    daqListNumber = XcpDaq_ListForEvent[eventChannelNumber];
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */
    if (!XcpDaq_GetFirstPid(daqListNumber, &pid)) {
        return;
    }

    listConf = XcpDaq_GetListConfiguration(daqListNumber);
    /* TODO: optimize!!! */
    for (odtIdx = (XcpDaq_ODTIntegerType)0; odtIdx < listConf->numOdts; ++odtIdx) {
        odt = XcpDaq_GetOdt(daqListNumber, odtIdx);
        for (odtEntryIdx = (XcpDaq_ODTEntryIntegerType)0; odtEntryIdx < odt->numOdtEntries; ++odtEntryIdx) {
            entry = XcpDaq_GetOdtEntry(daqListNumber, odtIdx, odtEntryIdx);

            // XcpDaq_SamplingBuffer
        }
    }
    return;
}

/** @brief Initialize DAQ message queue.
 *
 *
 */
XCP_STATIC void XcpDaq_InitMessageQueue(void)
{
    XCP_DAQ_ENTER_CRITICAL();
    XcpDaq_DtoBufferState.numEntries = UINT16(0);
    XcpDaq_DtoBufferState.front = UINT16(0);
    XcpDaq_DtoBufferState.back = UINT16(0);
    XcpDaq_DtoBufferState.allocated = UINT16(0);
    XCP_DAQ_LEAVE_CRITICAL();
}

/** @brief Post a message to queue.
 *
 * @param[in] msg
 *
 */
bool XcpDaq_EnqueueMessage(XcpDaq_MessageType const * msg)
{
    uint16_t rhs;
    uint16_t lhs;

    XCP_DAQ_ENTER_CRITICAL();
    if ((XCP_DAQ_MESSAGE_SIZE(msg) + XcpDaq_DtoBufferState.allocated) > UINT16(XCP_DAQ_DTO_BUFFER_SIZE)) {
        /* Overflow. */
        XCP_DAQ_LEAVE_CRITICAL();
        return (bool)XCP_FALSE;
    }

    if ((XcpDaq_DtoBufferState.back + XCP_DAQ_MESSAGE_SIZE(msg)) > UINT16(XCP_DAQ_DTO_BUFFER_SIZE)) {
        /* Wrapping required. */
        lhs = UINT16(XCP_DAQ_DTO_BUFFER_SIZE) - XcpDaq_DtoBufferState.back;
        rhs = XCP_DAQ_MESSAGE_SIZE(msg) - lhs;
        XcpDaq_DtoBuffer[XcpDaq_DtoBufferState.back] = msg->dlc;
        XcpDaq_DtoBufferState.back += UINT16(1);
        --lhs;
        if ((lhs > UINT16(0)) && (lhs)) {
            XcpUtl_MemCopy(&XcpDaq_DtoBuffer + XcpDaq_DtoBufferState.back, (void *)msg->data, lhs);
            XcpDaq_DtoBufferState.back += lhs;
        }
        XcpUtl_MemCopy(&XcpDaq_DtoBuffer, (void *)msg->data, rhs);
        XcpDaq_DtoBufferState.back = rhs;
    } else {
        XcpDaq_DtoBuffer[XcpDaq_DtoBufferState.back] = msg->dlc;
        XcpDaq_DtoBufferState.back += UINT16(1);
        XcpUtl_MemCopy(&XcpDaq_DtoBuffer + XcpDaq_DtoBufferState.back, (void *)msg->data, msg->dlc);
        XcpDaq_DtoBufferState.back = (XcpDaq_DtoBufferState.back + msg->dlc) % UINT16(XCP_DAQ_DTO_BUFFER_SIZE);
    }
    XcpDaq_DtoBufferState.allocated += XCP_DAQ_MESSAGE_SIZE(msg);
    XCP_DAQ_LEAVE_CRITICAL();
    return (bool)XCP_TRUE;
}

/** @brief Take a message from queue.
 *
 * @param[out] msg
 *
 */
bool XcpDaq_DequeueMessage(XcpDaq_MessageType * msg)
{
    uint16_t rhs;
    uint16_t lhs;

    XCP_DAQ_ENTER_CRITICAL();
    if (XcpDaq_DtoBufferState.front == XcpDaq_DtoBufferState.back) {
        XCP_DAQ_LEAVE_CRITICAL();
        return (bool)XCP_FALSE;
    }
    msg->dlc = XcpDaq_DtoBuffer[XcpDaq_DtoBufferState.front];
    if ((XcpDaq_DtoBufferState.front + XCP_DAQ_MESSAGE_SIZE(msg)) > UINT16(XCP_DAQ_DTO_BUFFER_SIZE)) {
        /* Wrapping required. */
        lhs = UINT16(XCP_DAQ_DTO_BUFFER_SIZE) - XcpDaq_DtoBufferState.front;
        rhs = XCP_DAQ_MESSAGE_SIZE(msg) - lhs;
        msg->dlc = XcpDaq_DtoBuffer[XcpDaq_DtoBufferState.front];
        XcpDaq_DtoBufferState.front += UINT16(1);
        --lhs;
        if ((lhs > UINT16(0)) && (lhs)) {
            XcpUtl_MemCopy((void *)msg->data, &XcpDaq_DtoBuffer + XcpDaq_DtoBufferState.front, lhs);
            XcpDaq_DtoBufferState.front += lhs;
        }
        XcpUtl_MemCopy((void *)msg->data, &XcpDaq_DtoBuffer, rhs);
        XcpDaq_DtoBufferState.front = rhs;
    } else {
        msg-> dlc = XcpDaq_DtoBuffer[XcpDaq_DtoBufferState.front];
        XcpDaq_DtoBufferState.front += UINT16(1);
        XcpUtl_MemCopy((void *)msg->data, &XcpDaq_DtoBuffer + XcpDaq_DtoBufferState.front, msg->dlc);
        XcpDaq_DtoBufferState.front = (XcpDaq_DtoBufferState.front + msg->dlc) % UINT16(XCP_DAQ_DTO_BUFFER_SIZE);
    }
    XcpDaq_DtoBufferState.allocated -= XCP_DAQ_MESSAGE_SIZE(msg);
    XCP_DAQ_LEAVE_CRITICAL();
    return (bool)XCP_TRUE;
}

#if 0
bool XcpDaq_MessageQueueEmpty(void)
{
    return (XcpDaq_DtoBufferState.front == XcpDaq_DtoBufferState.back);
}

bool XcpDaq_MessageQueueFull(void)
{
    return (XcpDaq_DtoBufferState.back == (
        (XcpDaq_DtoBufferState.front - 1 + XCP_DAQ_DTO_BUFFER_SIZE) % XCP_DAQ_DTO_BUFFER_SIZE)
    );
}
#endif // 0

/** @brief Copies bytes from a source memory area to a destination memory area,
 *   where both areas may not overlap.
 *  @param[out] dst  The memory area to copy to.
 *  @param[in]  src  The memory area to copy from.
 *  @param[in]  len  The number of bytes to copy
 */
void XcpDaq_CopyMemory(void * dst, void * src, uint32_t len)
{
    XcpUtl_MemCopy(dst, src, len);
}

#if defined(_WIN32)
void XcpDaq_PrintDAQDetails(void)
{
    XcpDaq_ListIntegerType listIdx;
    XcpDaq_ODTIntegerType odtIdx;
    XcpDaq_ODTEntryIntegerType odtEntriyIdx;
    XcpDaq_ListConfigurationType const * listConf;
    XcpDaq_ListStateType * listState;
    XcpDaq_ODTType const * odt;
    XcpDaq_ODTEntryType * entry;
    uint8_t mode;
    uint32_t total;
    XcpDaq_ODTIntegerType firstPid;

    printf("\nDAQ configuration\n");
    printf("-----------------\n");
    for (listIdx = (XcpDaq_ListIntegerType)0; listIdx < XcpDaq_GetListCount(); ++listIdx) {
        listState = XcpDaq_GetListState(listIdx);
        listConf = XcpDaq_GetListConfiguration(listIdx);
        mode = listState->mode;
        total = UINT16(0);
        XcpDaq_GetFirstPid(listIdx, &firstPid);
#if XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_DYNAMIC
        printf("DAQ-List #%d [%s] firstPid: %d mode: ", listIdx, (listIdx < XCP_MIN_DAQ) ? "predefined" : "dynamic",
               firstPid
        );
#elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_STATIC
        printf("DAQ-List #%d [%s] firstPid: %d mode: ", listIdx, (listIdx < XCP_MIN_DAQ) ? "predefined" : "static",
               firstPid
        );
#elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_NONE
    printf("DAQ-List #%d [predefined] firstPid: %d mode: ", listIdx, firstPid);
#endif /* XCP_DAQ_CONFIG_TYPE */

        if (mode & XCP_DAQ_LIST_MODE_DIRECTION) {
            printf("STIM ");
        } else {
            printf("DAQ ");
        }
        if (mode & XCP_DAQ_LIST_MODE_SELECTED) {
            printf("SELECTED ");
        }
        if (mode & XCP_DAQ_LIST_MODE_STARTED) {
            printf("STARTED ");
        }
        if (mode & XCP_DAQ_LIST_MODE_ALTERNATING) {
            printf("ALTERNATING ");
        }
        if (mode & XCP_DAQ_LIST_MODE_PID_OFF) {
            printf("PID_OFF ");
        }
        if (mode & XCP_DAQ_LIST_MODE_TIMESTAMP) {
            printf("TIMESTAMP ");
        }
        printf("\n");
        for (odtIdx = (XcpDaq_ODTIntegerType)0; odtIdx < listConf->numOdts; ++odtIdx) {
            odt = XcpDaq_GetOdt(listIdx, odtIdx);
            printf("    ODT #%d\n", odtIdx);
            for (odtEntriyIdx = (XcpDaq_ODTEntryIntegerType)0; odtEntriyIdx < odt->numOdtEntries; ++odtEntriyIdx) {
                entry = XcpDaq_GetOdtEntry(listIdx, odtIdx, odtEntriyIdx);
                printf("        Entry #%d [0x%08x] %d Byte(s)\n", odtEntriyIdx, entry->mta.address, entry->length);
                total += entry->length;
            }
        }
        printf("                          -------------\n");
        printf("                          %-5d Byte(s)\n", total);
    }
    printf("-------------------------------------------------------------------------------\n");
}

void XcpDaq_Info(void)
{
    Xcp_StateType const * Xcp_State;

    Xcp_State = Xcp_GetState();

    printf("DAQ\n---\n");
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    printf("Processor state       : ");
    switch (Xcp_State->daqProcessor.state) {
        case XCP_DAQ_STATE_UNINIT:
            printf("Uninitialized");
            break;
        case XCP_DAQ_STATE_CONFIG_INVALID:
            printf("Configuration invalid");
            break;
        case XCP_DAQ_STATE_CONFIG_VALID:
            printf("Configuration valid");
            break;
        case XCP_DAQ_STATE_STOPPED:
            printf("Stopped");
            break;
        case XCP_DAQ_STATE_RUNNING:
            printf("Running");
            break;
    }
    printf("\n");
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
    printf("Allocated DAQ entities: %d of %d\n", XcpDaq_EntityCount, XCP_DAQ_MAX_DYNAMIC_ENTITIES);
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#else
    printf("\tfunctionality not supported.\n");
#endif
}
#endif



void XcpDaq_GetProperties(uint8_t * properties)
{
    *properties = UINT8(0);
#if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
    *properties |= XCP_DAQ_PROP_PRESCALER_SUPPORTED;
#endif /*XCP_DAQ_ENABLE_PRESCALER */

#if (XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_NONE) || (XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_STATIC)
    *properties |= UINT8(XCP_DAQ_CONFIG_TYPE_STATIC);
#elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_DYNAMIC
    *properties |= UINT8(XCP_DAQ_CONFIG_TYPE_NONE);
#endif

}


void XcpDaq_SetProcessorState(XcpDaq_ProcessorStateType state)
{
    Xcp_StateType * stateVar;

    stateVar = Xcp_GetState();
    XCP_DAQ_ENTER_CRITICAL();
    stateVar->daqProcessor.state = state;
    XCP_DAQ_LEAVE_CRITICAL();
}


void XcpDaq_StartSelectedLists(void)
{
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_START);
}


void XcpDaq_StopSelectedLists(void)
{
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_STOP);
}


void XcpDaq_StopAllLists(void)
{
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_STOP);
}


/*
** Local Functions.
*/
XCP_STATIC XcpDaq_ODTType const * XcpDaq_GetOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber)
{
    XcpDaq_ListConfigurationType const * dl;
    XcpDaq_ODTIntegerType idx;

    dl = XcpDaq_GetListConfiguration(daqListNumber);
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

XCP_STATIC void XcpDaq_StartStopLists(XcpDaq_ListTransitionType transition)
{
    XcpDaq_ListIntegerType idx;
    XcpDaq_ListStateType * entry;

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
            *  The slave has to reset the SELECTED flag in the mode at GET_DAQ_LIST_MODE
            *  as soon as the related START_STOP_SYNCH or SET_REQUEST have been acknowledged.
            */
            entry->mode &= UINT8(~XCP_DAQ_LIST_MODE_SELECTED);
        }
    }
}


bool XcpDaq_GetFirstPid(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType * firstPID)
{
    XcpDaq_ListIntegerType listIdx;
    XcpDaq_ListConfigurationType const * daqList;
    bool result = (bool)XCP_TRUE;
    XcpDaq_ODTIntegerType tmp = (XcpDaq_ODTIntegerType)0;

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
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
void XcpDaq_GetCounts(XCP_DAQ_ENTITY_TYPE * entityCount, XCP_DAQ_ENTITY_TYPE * listCount, XCP_DAQ_ENTITY_TYPE * odtCount)
{
    *entityCount = XcpDaq_EntityCount;
    *listCount = XcpDaq_ListCount;
    *odtCount = XcpDaq_OdtCount;
}

uint16_t XcpDaq_TotalDynamicEntityCount(void)
{
    return UINT16(XCP_DAQ_MAX_DYNAMIC_ENTITIES);
}

XcpDaq_EntityType * XcpDaq_GetDynamicEntities(void)
{
    return &XcpDaq_Entities[0];
}

XcpDaq_EntityType * XcpDaq_GetDynamicEntity(uint16_t num)
{
    return &XcpDaq_Entities[num];
}

uint8_t * XcpDaq_GetDtoBuffer(void)
{
    return &XcpDaq_DtoBuffer[0];
}

#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#endif // XCP_BUILD_TYPE
