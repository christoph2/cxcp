extern "C" {
#include "flsemu.h"
#include "xcp.h"
#include "xcp_threads.h"
}

#include <thread>

extern "C" {
extern const FlsEmu_ConfigType FlsEmu_Config;
void Application_Initialize(void);
void Application_Run(void);
void parse_options(int argc, char **argv, Xcp_OptionsType *options);
Xcp_OptionsType Xcp_Options = {};
}

int main(int argc, char **argv) {
    parse_options(argc, argv, &Xcp_Options);
    FlsEmu_Init(&FlsEmu_Config);
    Application_Initialize();
    Xcp_Init();

    std::thread application_thread(Application_Run);
    XcpThrd_RunThreads();
    application_thread.join();

    FlsEmu_DeInit();
    XcpHw_Deinit();
    XcpTl_DeInit();
    return 0;
}
