/**
 * @brief The EpicsProxy class provides a C++ wrapper around the EPICS library for interfacing with EPICS PVs.
 *
 * The EpicsProxy class provides a simple and convenient way to interact with EPICS PVs from C++ code. 
 * It encapsulates the low-level details of the EPICS library and provides a high-level interface for 
 * reading and writing PV values, monitoring PV changes, and handling PV events.
 *
 * To use the EpicsProxy class, simply create an instance of the class with the name of the device you
 *  want to connect to, and then use the public member functions to interact with the PVs. The class
 *  takes care of connecting to the EPICS server, creating the necessary channels, and handling the
 *  communication with the PVs.
 *
 * Values read from PVs are returned as std::any objects and must be appropriately cast to the desired
 *  type. Values written to PVs must be of a valid type. The allowed data types are: double('t'),
 *  float('f'), enum('t'), short('s'), char('h'), string('A40_c'), long('l')
 * 
 * @note The allowed data types are: double, float, enum, short, char, string('A40_c'), long, and unsigned long
 * @note Arrays are not supported in this version of EpicsProxy
 * @note The EpicsProxy class requires the EPICS library to be installed on the system and linked with the application.
 * @note Contact Devin Burke (dburke1215@gmail.com) for questions or comments.
 * @note https://github.com/burkeds/cpp_EPICS_Driver
 * @version 1.2 
 * @author Devin Burke
 */

#include "EpicsProxy.h"

namespace epics{

void EpicsProxy::init(std::string m_deviceName,
                      std::vector<std::string> m_pvNames,
                      caConfig m_caConfig) {
    //Configure channel access
    setenv("EPICS_CA_ADDR_LIST", m_caConfig.ca_addr_list, 1);
    setenv("EPICS_CA_AUTO_ADDR_LIST", m_caConfig.ca_auto_addr_list, 1);
    setenv("EPICS_CA_CONN_TMO", m_caConfig.ca_conn_tmo, 1);
    setenv("EPICS_CA_BEACON_PERIOD", m_caConfig.ca_beacon_period, 1);
    setenv("EPICS_CA_REPEATER_PORT", m_caConfig.ca_repeater_port, 1);
    setenv("EPICS_CA_SERVER_PORT", m_caConfig.ca_server_port, 1);
    setenv("EPICS_CA_MAX_ARRAY_BYTES", m_caConfig.ca_max_array_bytes, 1);
    setenv("EPICS_TS_MIN_WEST", m_caConfig.ts_min_west, 1);

    //Initialize the EPICS context and assign a pointer to the context
    SEVCHK(ca_context_create(ca_enable_preemptive_callback), "Failed to create EPICS context");
    context = ca_current_context();
    //Set the device name
    deviceName = m_deviceName;

    //Create the PVs
    for (auto m_pvName : m_pvNames) {
        PV* m_pv = new PV(deviceName, m_pvName);
        pvList.push_back(m_pv);
    }
    ca_pend_io(5.0);
}
EpicsProxy::EpicsProxy(std::string name) {
    //Set the device name
    axisName = name;
}

EpicsProxy::~EpicsProxy() {
    //Destruct the contents of all pointers in pvList
    for (PV* m_pv : pvList) {
        delete m_pv;
    }
    pvList.clear();

    //Destroy the EPICS context
    ca_context_destroy();
}

void EpicsProxy::add_monitor(std::string m_fieldName, EpicsProxy* proxy, void (*callback)(struct event_handler_args args)) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->add_monitor(proxy, callback);
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

void EpicsProxy::remove_monitor(std::string m_fieldName) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->remove_monitor();
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}


template<typename TypeValue>
void EpicsProxy::write_pv(std::string m_fieldName, TypeValue m_value, bool pend) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->write<TypeValue>(m_value, pend);
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

void EpicsProxy::write_pv_string(std::string m_fieldName, std::string m_value, bool pend) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->write_string(m_value, pend);
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

template<typename TypeValue>
TypeValue EpicsProxy::read_pv(std::string m_fieldName, bool pend) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            return m_pv->read<TypeValue>(pend);
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

std::string EpicsProxy::read_pv_string(std::string m_fieldName, bool pend) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            return m_pv->read_string(pend);
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

//Instantiate the template function for allowed types
    template double EpicsProxy::read_pv<double>(std::string m_fieldName, bool pend);
    template float EpicsProxy::read_pv<float>(std::string m_fieldName, bool pend);
    template int EpicsProxy::read_pv<int>(std::string m_fieldName, bool pend);
    template short EpicsProxy::read_pv<short>(std::string m_fieldName, bool pend);
    template char EpicsProxy::read_pv<char>(std::string m_fieldName, bool pend);
    template long EpicsProxy::read_pv<long>(std::string m_fieldName, bool pend);
    template unsigned long EpicsProxy::read_pv<unsigned long>(std::string m_fieldName, bool pend);

    template void EpicsProxy::write_pv<double>(std::string m_fieldName, double m_value, bool pend);
    template void EpicsProxy::write_pv<float>(std::string m_fieldName, float m_value, bool pend);
    template void EpicsProxy::write_pv<int>(std::string m_fieldName, int m_value, bool pend);
    template void EpicsProxy::write_pv<short>(std::string m_fieldName, short m_value, bool pend);
    template void EpicsProxy::write_pv<char>(std::string m_fieldName, char m_value, bool pend);
    template void EpicsProxy::write_pv<long>(std::string m_fieldName, long m_value, bool pend);
    template void EpicsProxy::write_pv<unsigned long>(std::string m_fieldName, unsigned long m_value, bool pend);
}
