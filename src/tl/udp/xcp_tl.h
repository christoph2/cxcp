

#if !defined(__COMM_H)
#define __COMM_H

#include <stdint.h>
#include <stdbool.h>

#define XCP_MAX_CTO (0xff)
#define XCP_MAX_DTO (512)

#define XCP_TRANSPORT_LAYER_VERSION_MAJOR       (1)
#define XCP_TRANSPORT_LAYER_VERSION_RELEASE     (0)

typedef struct tagXcp_PDUType {
    uint8_t len;
    //uint8_t data[XCP_MAX_CTO - 1];
    uint8_t * data;
} Xcp_PDUType;

void XcpComm_Init(void);
void XcpComm_DeInit(void);
int XcpComm_FrameAvailable(long sec, long usec);
void XcpComm_RxHandler(void);

void XcpTl_Task(void);

void XcpTl_SendPdu(void);

uint8_t * XcpComm_GetOutPduPtr(void);
void XcpTl_SetPduOutLen(uint16_t len);
void XcpTl_Send8(uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);

void XcpTl_SaveConnection(void);
void XcpTl_ReleaseConnection(void);
bool XcpTl_VerifyConnection(void);


#endif // __COMM_H

