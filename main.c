
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "xcp_ow.h"


// For demo purpopses, create an extra constant data section whose name is exactly 8 bytes long (the max)
#pragma const_seg(".t_const") // begin allocating const data in a new section whose name is 8 bytes long (the max)
const char const_string1[] = "This string is allocated in a special const data segment named \".t_const\".";
#pragma const_seg() // resume allocating const data in the normal .rdata section



#pragma bss_seg(push, /*stack1,*/ ".arbeitsseite")
int in_der_arbeitsseite;

#pragma bss_seg(pop/*, stack1*/)
int bss_normal;

void pagTest(void);

static Xcp_HwMapFileType a2lFile;

static const char FNAME[] = "f:\\Users\\Public\\Documents\\Vector CANape 15\\Examples\\XCPDemo\\XCPsim.a2l";


void XcpOnCan_Init(void);

#define SIZE    (4096)
uint8_t puffer[SIZE];

void XcpHw_MainFunction(bool * finished);

// TortoiseGit 1.7.6.0

int main()
{
    uint16_t idx;
    bool finished;

    for (idx = 0; idx < SIZE; ++idx) {
        puffer[idx] = '0' + (idx % 10);
        //printf("%c ", puffer[idx]);
    }

    pagTest();

    //XcpOnCan_Init();

    XcpOw_MapFileOpen(FNAME, &a2lFile);

    Xcp_Init();
    DBG_PRINT1("Starting XCP task...\n");
    fflush(stdout);


    for (;;) {
        Xcp_MainFunction();
        XcpTl_MainFunction();
        //Xcp
//#if 0
        XcpHw_MainFunction(&finished);
        if (finished) {
            break;
        }
//#endif
    }

    XcpTl_DeInit();

    XcpOw_MapFileClose(&a2lFile);
    return 0;
}


bool Xcp_HookFunction_GetId(uint8_t idType)
{
    if (idType == 4) {
        Xcp_SetMta(Xcp_GetNonPagedAddress(a2lFile.view.mappingAddress));
        Xcp_Send8(8, 0xff, 0, 0, 0, XCP_LOBYTE(XCP_LOWORD(a2lFile.size)), XCP_HIBYTE(XCP_LOWORD(a2lFile.size)), XCP_LOBYTE(XCP_HIWORD(a2lFile.size)), XCP_HIBYTE(XCP_HIWORD(a2lFile.size)));
        return XCP_TRUE;
    }
    return XCP_FALSE;
}
