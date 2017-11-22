
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "xcp.h"
#include "comm.h"


void sig_handler(int s){
    printf("Caught signal %d\n",s);
    XcpComm_DeInit();
    exit(1);
}

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

    XcpComm_Init();
    //for(;;) {
    //}

    printf("Starting XCP task...\n");
    fflush(stdout);
//#if 0
    for (;;) {
        XcpComm_Task();
    }
//#endif
    XcpComm_DeInit();

    return 0;
}

