
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

    //setvbuf(stdout, NULL, _IONBF, 0);

#if 0
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
#endif
    //if (signal(SIGINT, sig_handler) == SIG_ERR) {
    //  printf("\ncan't catch SIGINT\n");
    //}

    Xcp_Init();
    //for(;;) {
    //}

    printf("Starting XCP task...\n");
    fflush(stdout);
//#if 0
    for (;;) {
        XcpTl_Task();
    }
//#endif
    XcpTl_DeInit();

    return 0;
}

