#include <cstdint>
#include <chrono>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <link.h>
#endif

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

#if defined(_WIN32)
static Xcp_MemoryMappingResultType ECUSim_PeCoffMemoryMapper(Xcp_MtaType *destination,
                                                              Xcp_MtaType const *source) {
    HMODULE                 module_handle = NULL;
    uint8_t const          *image_base = XCP_NULL;
    IMAGE_DOS_HEADER const *dos_header = XCP_NULL;
    IMAGE_NT_HEADERS const *nt_headers = XCP_NULL;
    uint32_t                rva;

    if (source->ext != 0U) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    rva = static_cast<uint32_t>(source->address);
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR)(uintptr_t)&ECUSim_PeCoffMemoryMapper,
            &module_handle
        )) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    image_base = reinterpret_cast<uint8_t const *>(module_handle);
    dos_header = reinterpret_cast<IMAGE_DOS_HEADER const *>(image_base);
    if ((dos_header == XCP_NULL) || (dos_header->e_magic != IMAGE_DOS_SIGNATURE)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    nt_headers = reinterpret_cast<IMAGE_NT_HEADERS const *>(image_base + dos_header->e_lfanew);
    if ((nt_headers == XCP_NULL) || (nt_headers->Signature != IMAGE_NT_SIGNATURE)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    if (rva >= static_cast<uint32_t>(nt_headers->OptionalHeader.SizeOfImage)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    destination->address = reinterpret_cast<Xcp_PointerSizeType>(image_base + rva);
    destination->ext = source->ext;

    return XCP_MEMORY_MAPPED;
}
#elif defined(__linux__)
typedef struct {
    uintptr_t image_base;
    uintptr_t min_vaddr;
    uintptr_t max_vaddr;
    uint8_t   found;
} ECUSim_ElfImageInfoType;

static int ECUSim_CollectMainElfInfo(struct dl_phdr_info *info, size_t size, void *data) {
    ECUSim_ElfImageInfoType *image_info = static_cast<ECUSim_ElfImageInfoType *>(data);
    uintptr_t                min_vaddr = UINTPTR_MAX;
    uintptr_t                max_vaddr = 0U;
    int                      has_load_segment = 0;
    uint16_t                 segment_idx;

    (void)size;

    if ((info->dlpi_name != XCP_NULL) && (info->dlpi_name[0] != '\0')) {
        return 0;
    }

    for (segment_idx = 0U; segment_idx < static_cast<uint16_t>(info->dlpi_phnum); ++segment_idx) {
        ElfW(Phdr) const *program_header = &info->dlpi_phdr[segment_idx];

        if (program_header->p_type != PT_LOAD) {
            continue;
        }

        if (static_cast<uintptr_t>(program_header->p_vaddr) < min_vaddr) {
            min_vaddr = static_cast<uintptr_t>(program_header->p_vaddr);
        }

        if (static_cast<uintptr_t>(program_header->p_vaddr) + static_cast<uintptr_t>(program_header->p_memsz) > max_vaddr) {
            max_vaddr = static_cast<uintptr_t>(program_header->p_vaddr) + static_cast<uintptr_t>(program_header->p_memsz);
        }

        has_load_segment = 1;
    }

    if (has_load_segment == 0) {
        return 0;
    }

    image_info->image_base = static_cast<uintptr_t>(info->dlpi_addr);
    image_info->min_vaddr = min_vaddr;
    image_info->max_vaddr = max_vaddr;
    image_info->found = 1U;

    return 1;
}

static Xcp_MemoryMappingResultType ECUSim_ElfMemoryMapper(Xcp_MtaType *destination,
                                                           Xcp_MtaType const *source) {
    ECUSim_ElfImageInfoType image_info = { 0U, 0U, 0U, 0U };
    uintptr_t               symbol_address;

    if (source->ext != 0U) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    (void)dl_iterate_phdr(ECUSim_CollectMainElfInfo, &image_info);
    if (image_info.found == 0U) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    symbol_address = static_cast<uintptr_t>(source->address);
    if ((symbol_address < image_info.min_vaddr) || (symbol_address >= image_info.max_vaddr)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    if (symbol_address > (UINTPTR_MAX - image_info.image_base)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    destination->address = static_cast<Xcp_PointerSizeType>(image_info.image_base + symbol_address);
    destination->ext = source->ext;

    return XCP_MEMORY_MAPPED;
}
#endif

Xcp_MemoryMappingResultType Xcp_HookFunction_AddressMapper(Xcp_MtaType *destination,
                                                             Xcp_MtaType const *source) {
    Xcp_MemoryMappingResultType result = FlsEmu_MemoryMapper(destination, source);

    if (result != XCP_MEMORY_NOT_MAPPED) {
        return result;
    }

#if defined(_WIN32)
    return ECUSim_PeCoffMemoryMapper(destination, source);
#elif defined(__linux__)
    return ECUSim_ElfMemoryMapper(destination, source);
#else
    return XCP_MEMORY_NOT_MAPPED;
#endif
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
