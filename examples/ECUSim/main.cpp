#include <thread>

#include "app_config.h"
#include "application.h"

extern "C" {
#include "xcp.h"
#include "xcp_terminal.h"
#include "xcp_threads.h"
}

Xcp_OptionsType Xcp_Options = { 0 };

extern "C" void parse_options(int argc, char **argv, Xcp_OptionsType *options);

int main(int argc, char **argv) {
    parse_options(argc, argv, &Xcp_Options);
    FlsEmu_Init(&FlsEmu_Config);
    Xcp_Init();
    Xcp_DisplayInfo();

    std::thread application_thread(EcuSim_RunApplication);
    XcpThrd_RunThreads();
    application_thread.join();

    FlsEmu_DeInit();
    XcpHw_Deinit();
    XcpTl_DeInit();
    return 0;
}
