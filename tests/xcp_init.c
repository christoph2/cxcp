
#include "xcp.h"

static Xcp_StateType Xcp_State;

XCP_DAQ_BEGIN_EVENTS
    XCP_DAQ_DEFINE_EVENT("EVT 100ms",
        XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS,
        100
    ),
    XCP_DAQ_DEFINE_EVENT("EVT sporadic",
        XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS,
        0
    ),
    XCP_DAQ_DEFINE_EVENT("EVT 10ms",
        XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS,
        10
    ),
XCP_DAQ_END_EVENTS

void Xcp_Init(void)
{
//    Xcp_ConnectionState = XCP_DISCONNECTED;

 //   XcpHw_Init();
    XcpUtl_MemSet(&Xcp_State, UINT8(0), (uint32_t)sizeof(Xcp_StateType));
    Xcp_State.busy = (bool)XCP_FALSE;

//    Xcp_DefaultResourceProtection();

#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
    Xcp_State.slaveBlockModeState.blockTransferActive = (bool)XCP_FALSE;
    Xcp_State.slaveBlockModeState.remaining = UINT8(0);
#endif  /* XCP_ENABLE_SLAVE_BLOCKMODE */
#if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
    Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_FALSE;
    Xcp_State.masterBlockModeState.remaining = UINT8(0);
#endif /* XCP_ENABLE_MASTER_BLOCKMODE */
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    XcpDaq_Init();
    Xcp_State.daqProcessor.state = XCP_DAQ_STATE_STOPPED;
    Xcp_State.daqPointer.daqList = (XcpDaq_ListIntegerType)0;
    Xcp_State.daqPointer.odt = (XcpDaq_ODTIntegerType)0;
    Xcp_State.daqPointer.odtEntry = (XcpDaq_ODTEntryIntegerType)0;
#endif /* XCP_ENABLE_DAQ_COMMANDS */
#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    Xcp_State.pgmProcessor.state = XCP_PGM_STATE_UNINIT;
#endif /* ENABLE_PGM_COMMANDS */
#if XCP_TRANSPORT_LAYER_COUNTER_SIZE != 0
    Xcp_State.counter = (uint16_t)0;
#endif /* XCP_TRANSPORT_LAYER_COUNTER_SIZE */

#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_State.statistics.crosBusy = UINT32(0);
    Xcp_State.statistics.crosSend = UINT32(0);
    Xcp_State.statistics.ctosReceived = UINT32(0);
#endif /* XCP_ENABLE_STATISTICS */
    //XcpTl_Init();

#if (XCP_ENABLE_BUILD_CHECKSUM == XCP_ON) && (XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON)
    //Xcp_ChecksumInit();
#endif /* XCP_ENABLE_BUILD_CHECKSUM */
}

Xcp_StateType * Xcp_GetState(void)
{
    Xcp_StateType * tState;

    XCP_DAQ_ENTER_CRITICAL();
    tState = &Xcp_State;
    XCP_DAQ_LEAVE_CRITICAL();

    return tState;
}
