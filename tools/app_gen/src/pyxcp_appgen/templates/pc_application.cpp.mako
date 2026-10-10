#include <cstdint>
#include <chrono>
#include <thread>

extern "C" {
#include "flsemu.h"
#include "xcp.h"
#include "xcp_threads.h"
}

% for segment in pc_segments:
% if segment["type"] == "RAM":
static uint8_t ${segment["c_name"]}_storage[${segment["size"]}] = {};
static FlsEmu_PersistentArrayType ${segment["c_name"]}_backing = {
    XCP_NULL, XCP_NULL, ${segment["c_name"]}_storage, 0
};
% endif
% endfor

extern "C" {
% for segment in pc_segments:
static FlsEmu_SegmentType ${segment["c_name"]} = {
    "${segment["name"]}", ${segment["size"]}U, ${segment["write_size"]}U,
    ${segment["sector_size"]}U, ${segment["page_size"]}U, ${segment["blocks"]}U,
    static_cast<Xcp_PointerSizeType>(0x${format(segment["base_address"], "X")}),
    % if segment["type"] == "RAM":
    &${segment["c_name"]}_backing,
    % else:
    XCP_NULL,
    % endif
    0U, 0U, FLSEMU_${segment["type"]}, ${segment["address_extension"]}U
};
% endfor
static FlsEmu_SegmentType *FlsEmu_Segments[] = {
% for segment in pc_segments:
    &${segment["c_name"]},
% endfor
};
extern const FlsEmu_ConfigType FlsEmu_Config = {
    ${len(pc_segments)}U, FlsEmu_Segments
};

XCP_DAQ_BEGIN_EVENTS
% for event in project["module"].get("events", []):
    XCP_DAQ_DEFINE_EVENT("${event["name"]}", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, ${event.get("interval", 10)}),
% endfor
XCP_DAQ_END_EVENTS

Xcp_MemoryMappingResultType Xcp_HookFunction_AddressMapper(Xcp_MtaType *destination,
                                                             Xcp_MtaType const *source) {
    return FlsEmu_MemoryMapper(destination, source);
}
}

static void initialize_measurements() {
% for measurement in measurements:
    {
        Xcp_MtaType source = {
            ${measurement.get("ecu_address_extension", 0)}U,
            static_cast<Xcp_PointerSizeType>(${measurement["ecu_address"]})
        };
        Xcp_MtaType destination = {};
        if (FlsEmu_MemoryMapper(&destination, &source) == XCP_MEMORY_MAPPED) {
            *reinterpret_cast<volatile ${pc_c_type(measurement)} *>(destination.address) = ${get_initializer(measurement)};
        }
    }
% endfor
}

extern "C" void Application_Initialize(void) {
    initialize_measurements();
}

extern "C" void Application_Run(void) {
    using Clock = std::chrono::steady_clock;
    using namespace std::chrono;

% for event in project["module"].get("events", []):
% if event.get("interval", 10) > 0:
    auto next_event_${event["number"]} = Clock::now();
% endif
% endfor
    while (!XcpThrd_IsShuttingDown()) {
        const auto now = Clock::now();
% for event in project["module"].get("events", []):
% if event.get("interval", 10) > 0:
        if (now >= next_event_${event["number"]}) {
            XcpDaq_TriggerEvent(${event["number"]}U);
            next_event_${event["number"]} += milliseconds(${event.get("interval", 10)});
            if (now > next_event_${event["number"]}) {
                next_event_${event["number"]} = now + milliseconds(${event.get("interval", 10)});
            }
        }
% endif
% endfor
        std::this_thread::sleep_for(milliseconds(1));
    }
}
