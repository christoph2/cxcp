
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

static const char FNAME[] = "C:\\Users\\Public\\Documents\\Vector CANape 15\\Examples\\XCPDemo\\XCPsim.a2l";


void XcpOnCan_Init(void);

int main()
{
    pagTest();

    //XcpOnCan_Init();

    XcpOw_MapFileOpen(FNAME, &a2lFile);

    Xcp_Init();
    DBG_PRINT1("Starting XCP task...\n");
    fflush(stdout);

    for (;;) {
        XcpTl_Task();
    }

    XcpTl_DeInit();

    XcpOw_MapFileClose(&a2lFile);
    return 0;
}


bool Xcp_HookFunction_GetId(uint8_t idType)
{
    if (idType == 4) {
        Xcp_SetMta(Xcp_GetNonPagedAddress(a2lFile.view.mappingAddress));
        Xcp_Send8(8, 0xff, 0, 0, 0, LOBYTE(LOWORD(a2lFile.size)), HIBYTE(LOWORD(a2lFile.size)), LOBYTE(HIWORD(a2lFile.size)), HIBYTE(HIWORD(a2lFile.size)));
        return TRUE;
    }
    return FALSE;
}
