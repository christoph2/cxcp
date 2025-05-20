#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <sstream>
#include <tuple>
#include <variant>
#include <vector>

extern "C" {
#include "xcp.h"
}

struct Mta {
	uint32_t address;
	uint8_t ext;

	std::string to_string() const {
		std::ostringstream ss;
        ss << "MTA(addr=" << std::hex << address << ", ext=" << (int)ext << ")";
        return ss.str();
	}
};

struct OdtEntry {
	Mta mta;
	std::uint32_t length;

	std::string to_string() const {
        std::ostringstream ss;
	    ss << "ODTEntry(mta=" << mta.to_string() << ", length=" << length << ")";
        return ss.str();
	}
};

struct Odt {
	std::uint16_t num_odt_entries;
	std::uint16_t first_odt_entry;

	std::string to_string() const {
        std::ostringstream ss;
        ss << "ODT(num_odt_entries=" << num_odt_entries << ", first_odt_entry=" << first_odt_entry << ")";
        return ss.str();
    }
};

struct DaqList {
    std::uint16_t num_odts;
    std::uint16_t first_odt;
    std::uint8_t mode;
	std::uint8_t prescaler;
    uint8_t counter;

	std::string to_string() const {
	    std::ostringstream ss;
        ss << "DaqList(num_odts=" << num_odts << ", first_odt=" << first_odt << ", mode=" << (int)mode << ", prescaler=" << (int)prescaler << ", counter=" << (int)counter << ")";
        return ss.str();
	}
};

using DaqEntity = std::variant<std::monostate, OdtEntry, Odt, DaqList>;

auto get_daq_counts() -> std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> {
	std::uint8_t entityCount=0;
	std::uint8_t listCount=0;
	std::uint8_t odtCount=0;
	XcpDaq_GetCounts(&entityCount, &listCount, &odtCount);

	return {entityCount, listCount, odtCount};
}

DaqEntity create_daq_entity(const XcpDaq_EntityType& entity) {
	DaqEntity result{};

	auto disc = static_cast<XcpDaq_EntityKindType>(entity.kind);
	switch (disc) {
		case XCP_ENTITY_DAQ_LIST:
			result = DaqList(entity.entity.daqList.numOdts, entity.entity.daqList.firstOdt,
				entity.entity.daqList.mode, 0, 0
			);
			break;
    	case XCP_ENTITY_ODT:
			result = Odt(entity.entity.odt.numOdtEntries, entity.entity.odt.firstOdtEntry);
			break;
    	case XCP_ENTITY_ODT_ENTRY:
			result = OdtEntry(Mta(entity.entity.odtEntry.mta.address, 0), entity.entity.odtEntry.length);
			break;
	}
	return result;
}

auto get_dynamic_daq_entity(std::uint16_t idx) -> DaqEntity {
	return create_daq_entity(*(XcpDaq_GetDynamicEntity(idx)));
}

auto get_dynamic_daq_entities() -> std::vector<DaqEntity> {
	std::vector<DaqEntity> entities;

    for (auto idx = 0; idx < XcpDaq_GetDynamicDaqEntityCount(); ++idx) {
        entities.push_back(get_dynamic_daq_entity(idx));
	}
	return entities;
}

auto daq_enqueue(const std::string& data) -> bool {
    return XcpDaq_QueueEnqueue(data.size(), reinterpret_cast<const uint8_t*>(data.data()));
}

auto daq_dequeue() -> std::tuple<bool, std::string> {
    bool return_code;
    std::uint16_t length;
    // std::vector<uint8_t> data;
    uint8_t data[XCP_MAX_DTO];

    return_code = XcpDaq_QueueDequeue(&length, &data[0]);
    if (!return_code) {
        return std::make_tuple(return_code, std::string());
    }
    return std::make_tuple(return_code, std::move(std::string(reinterpret_cast<const char*>(data), length)));
}

auto get_daq_queue_var()  -> XcpDaq_QueueType {
    XcpDaq_QueueType queue;
    XcpDaq_QueueGetVar(&queue);
    return queue;
}

namespace py = pybind11;

PYBIND11_MODULE(cxcp, m) {
	m.def("Xcp_Init", &Xcp_Init);
    m.def("XcpDaq_Init", &XcpDaq_Init);
	m.def("XcpDaq_Free", &XcpDaq_Free);
	m.def("XcpDaq_Alloc", &XcpDaq_Alloc);
	m.def("XcpDaq_AllocOdt", &XcpDaq_AllocOdt);
	m.def("XcpDaq_AllocOdtEntry", &XcpDaq_AllocOdtEntry);
	m.def("XcpDaq_QueueInit", &XcpDaq_QueueInit);
//#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
	m.def("XcpDaq_QueueGetVar", &XcpDaq_QueueGetVar);
    m.def("daq_enqueue", &daq_enqueue);
    m.def("daq_dequeue", &daq_dequeue);
//#endif

#if 0
	m.def("", &);
	m.def("", &);
#endif
	m.def("Xcp_GetConnectionState", &Xcp_GetConnectionState);
    m.def("Xcp_CalculateChecksum", &Xcp_CalculateChecksum);	// , py::return_value_policy::move
	m.def("XcpDaq_GetDynamicEntity", &XcpDaq_GetDynamicEntity);
	m.def("get_daq_counts", &get_daq_counts);
	m.def("XcpDaq_TotalDynamicEntityCount", &XcpDaq_TotalDynamicEntityCount);
	m.def("XcpDaq_GetDynamicDaqEntityCount", &XcpDaq_GetDynamicDaqEntityCount);
	m.def("get_dynamic_daq_entities", &get_dynamic_daq_entities);
	m.def("get_dynamic_daq_entity", &get_dynamic_daq_entity);
    m.def("get_daq_queue_var", &get_daq_queue_var);

	py::class_<Mta>(m, "Mta")
		// .def(py::init<int>())
		.def_readonly("address", &Mta::address)
		.def_readonly("ext", &Mta::ext)
		.def("__repr__", &Mta::to_string)
	;

	py::class_<OdtEntry>(m, "OdtEntry")
		.def_readonly("mta", &OdtEntry::mta)
		.def_readonly("length", &OdtEntry::length)
		.def("__repr__", &OdtEntry::to_string)
	;

	py::class_<Odt>(m, "Odt")
		.def_readonly("num_odt_entries", &Odt::num_odt_entries)
		.def_readonly("first_odt_entry", &Odt::first_odt_entry)
		.def("__repr__", &Odt::to_string)
	;

	py::class_<DaqList>(m, "DaqList")
		.def_readonly("num_odts", &DaqList::num_odts)
		.def_readonly("num_odts", &DaqList::num_odts)
		.def_readonly("prescaler", &DaqList::prescaler)
		.def_readonly("counter", &DaqList::counter)
		.def("__repr__", &DaqList::to_string)
	;

	py::class_<tagXcpDaq_ListConfigurationType>(m, "XcpDaq_ListConfigurationType")
		.def_readonly("numOdts", &tagXcpDaq_ListConfigurationType::numOdts)
        .def_readonly("firstOdt", &tagXcpDaq_ListConfigurationType::firstOdt)
	;

	py::class_<tagXcpDaq_ListStateType>(m, "XcpDaq_ListStateType")
		.def_readonly("mode", &tagXcpDaq_ListStateType::mode)
	;

    py::class_<XcpDaq_QueueType>(m, "XcpDaq_QueueType")
		.def_readonly("head", &XcpDaq_QueueType::head)
        .def_readonly("tail", &XcpDaq_QueueType::tail)
        .def_readonly("overload", &XcpDaq_QueueType::overload)
	;


#if 0
    typedef struct tagXcpDaq_EntityType {
        /*Xcp_DaqEntityKindType*/ uint8_t kind;
        union {
            XcpDaq_ODTEntryType    odtEntry;
            XcpDaq_ODTType         odt;
            XcpDaq_DynamicListType daqList;
        } entity;
    } XcpDaq_EntityType;

    typedef struct tagXcpDaq_EventType {
        uint8_t const * const name;
        uint8_t               nameLen;
        uint8_t               properties;
        uint8_t               timeunit;
        uint8_t               cycle;
        /* unit8_t priority; */
    } XcpDaq_EventType;
#endif

	pybind11::enum_<tagXcpDaq_DirectionType>(m, "XcpDaq_DirectionType")
        .value("XCP_DIRECTION_NONE", tagXcpDaq_DirectionType::XCP_DIRECTION_NONE)
        .value("XCP_DIRECTION_DAQ", tagXcpDaq_DirectionType::XCP_DIRECTION_DAQ)
        .value("XCP_DIRECTION_STIM", tagXcpDaq_DirectionType::XCP_DIRECTION_STIM)
        .value("XCP_DIRECTION_DAQ_STIM", tagXcpDaq_DirectionType::XCP_DIRECTION_DAQ_STIM)
		.export_values()
	;

	pybind11::enum_<tagXcp_ConnectionStateType>(m, "Xcp_ConnectionStateType")
		.value("XCP_DISCONNECTED", tagXcp_ConnectionStateType::XCP_DISCONNECTED)
		.value("XCP_CONNECTED", tagXcp_ConnectionStateType::XCP_CONNECTED)
		.export_values()
	;
	
	pybind11::enum_<tagXcpDaq_EntityKindType>(m, "XcpDaq_EntityKindType")
    	.value("XCP_ENTITY_UNUSED", tagXcpDaq_EntityKindType::XCP_ENTITY_UNUSED)
    	.value("XCP_ENTITY_DAQ_LIST", tagXcpDaq_EntityKindType::XCP_ENTITY_DAQ_LIST)
    	.value("XCP_ENTITY_ODT", tagXcpDaq_EntityKindType::XCP_ENTITY_ODT)
    	.value("XCP_ENTITY_ODT_ENTRY", tagXcpDaq_EntityKindType::XCP_ENTITY_ODT_ENTRY)
		.export_values()
	;	


	pybind11::enum_<tagXcp_ReturnType>(m, "Xcp_ReturnType")
    .value("ERR_CMD_SYNCH", tagXcp_ReturnType::ERR_CMD_SYNCH)
    .value("ERR_CMD_BUSY", tagXcp_ReturnType::ERR_CMD_BUSY)
    .value("ERR_DAQ_ACTIVE", tagXcp_ReturnType::ERR_DAQ_ACTIVE)
    .value("ERR_PGM_ACTIVE", tagXcp_ReturnType::ERR_PGM_ACTIVE)
    .value("ERR_CMD_UNKNOWN", tagXcp_ReturnType::ERR_CMD_UNKNOWN)
    .value("ERR_CMD_SYNTAX", tagXcp_ReturnType::ERR_CMD_SYNTAX)
    .value("ERR_OUT_OF_RANGE", tagXcp_ReturnType::ERR_OUT_OF_RANGE)
    .value("ERR_WRITE_PROTECTED", tagXcp_ReturnType::ERR_WRITE_PROTECTED)
    .value("ERR_ACCESS_DENIED", tagXcp_ReturnType::ERR_ACCESS_DENIED)
    .value("ERR_ACCESS_LOCKED", tagXcp_ReturnType::ERR_ACCESS_LOCKED)
    .value("ERR_PAGE_NOT_VALID", tagXcp_ReturnType::ERR_PAGE_NOT_VALID)
    .value("ERR_MODE_NOT_VALID", tagXcp_ReturnType::ERR_MODE_NOT_VALID)
    .value("ERR_SEGMENT_NOT_VALID", tagXcp_ReturnType::ERR_SEGMENT_NOT_VALID)
    .value("ERR_SEQUENCE", tagXcp_ReturnType::ERR_SEQUENCE)
    .value("ERR_DAQ_CONFIG", tagXcp_ReturnType::ERR_DAQ_CONFIG)
    .value("ERR_MEMORY_OVERFLOW", tagXcp_ReturnType::ERR_MEMORY_OVERFLOW)
    .value("ERR_GENERIC", tagXcp_ReturnType::ERR_GENERIC)
    .value("ERR_VERIFY", tagXcp_ReturnType::ERR_VERIFY)
    .value("ERR_RESOURCE_TEMPORARY_NOT_ACCESSIBLE", tagXcp_ReturnType::ERR_RESOURCE_TEMPORARY_NOT_ACCESSIBLE)
    .value("ERR_SUCCESS", tagXcp_ReturnType::ERR_SUCCESS)
	.export_values()
	;

 }
