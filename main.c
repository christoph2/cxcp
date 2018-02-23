
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "xcp.h"
#include "xcp_tl.h"


void sig_handler(int s){
    printf("Caught signal %d\n",s);
    XcpTl_DeInit();
    exit(1);
}


#pragma bss_seg(push, /*stack1,*/ ".arbeitsseite")
int in_der_arbeitsseite;

#pragma bss_seg(pop/*, stack1*/)
int bss_normal;


int main()
{
    Xcp_Init();

    //DBG_PRINT("Starting XCP task...\n");
    DBG_PRINT("Starting XCP task...\n");
    fflush(stdout);
//#if 0
    for (;;) {
        XcpTl_Task();
    }
//#endif
    XcpTl_DeInit();

    return 0;
}

