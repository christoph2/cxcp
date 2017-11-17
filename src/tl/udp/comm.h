

#if !defined(__COMM_H)
#define __COMM_H

#include <stdint.h>

#define XCP_MAX_CTO (0xff)
#define XCP_MAX_DTO (512)

void XcpComm_Init(void);
void XcpComm_DeInit(void);
int XcpComm_FrameAvailable(long sec, long usec);
void XcpComm_RxHandler(void);


#endif // __COMM_H

