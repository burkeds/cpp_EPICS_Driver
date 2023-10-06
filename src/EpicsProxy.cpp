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
 * @note The EpicsProxy class requires the EPICS library to be installed on the system and linked with the application.
 * @note Contact Devin Burke (dburke1215@gmail.com) for questions or comments.
 * @note https://github.com/burkeds/cpp_EPICS_Driver
 * @version 1.3 
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

    caContext_ptr = new caContext();
    //Set the device name
    deviceName = m_deviceName;

    //Create the PVs
    for (auto m_pvName : m_pvNames) {
        PV* m_pv = new PV(deviceName, m_pvName);
        pvList.push_back(m_pv);
    }
    SEVCHK(ca_pend_io(5.0), "Failed to create PVs");
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
    destroy_context();
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
void EpicsProxy::write_pv(std::string m_fieldName, TypeValue m_value) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->write<TypeValue>(m_value);
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

void EpicsProxy::write_pv_string(std::string m_fieldName, std::string m_value) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->write_string(m_value);
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

template<typename TypeValue>
void EpicsProxy::write_pv_array(std::string m_fieldName, std::vector<TypeValue> m_value) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->write_array<TypeValue>(m_value);
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

template<typename TypeValue>
TypeValue EpicsProxy::read_pv(std::string m_fieldName) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            return m_pv->read<TypeValue>();
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

std::string EpicsProxy::read_pv_string(std::string m_fieldName) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            return m_pv->read_string();
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

template<typename TypeValue>
std::vector<TypeValue> EpicsProxy::read_pv_array(std::string m_fieldName) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            return m_pv->read_array<TypeValue>();
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

//Instantiate the template function for allowed types
    template double EpicsProxy::read_pv<double>(std::string m_fieldName);
    template float EpicsProxy::read_pv<float>(std::string m_fieldName);
    template int EpicsProxy::read_pv<int>(std::string m_fieldName);
    template short EpicsProxy::read_pv<short>(std::string m_fieldName);
    template char EpicsProxy::read_pv<char>(std::string m_fieldName);
    template long EpicsProxy::read_pv<long>(std::string m_fieldName);
    template unsigned long EpicsProxy::read_pv<unsigned long>(std::string m_fieldName);

    template std::vector<double> EpicsProxy::read_pv_array<double>(std::string m_fieldName);
    template std::vector<float> EpicsProxy::read_pv_array<float>(std::string m_fieldName);
    template std::vector<int> EpicsProxy::read_pv_array<int>(std::string m_fieldName);
    template std::vector<short> EpicsProxy::read_pv_array<short>(std::string m_fieldName);
    template std::vector<char> EpicsProxy::read_pv_array<char>(std::string m_fieldName);
    template std::vector<long> EpicsProxy::read_pv_array<long>(std::string m_fieldName);
    template std::vector<unsigned long> EpicsProxy::read_pv_array<unsigned long>(std::string m_fieldName);

    template void EpicsProxy::write_pv<double>(std::string m_fieldName, double m_value);
    template void EpicsProxy::write_pv<float>(std::string m_fieldName, float m_value);
    template void EpicsProxy::write_pv<int>(std::string m_fieldName, int m_value);
    template void EpicsProxy::write_pv<short>(std::string m_fieldName, short m_value);
    template void EpicsProxy::write_pv<char>(std::string m_fieldName, char m_value);
    template void EpicsProxy::write_pv<long>(std::string m_fieldName, long m_value);
    template void EpicsProxy::write_pv<unsigned long>(std::string m_fieldName, unsigned long m_value);

    template void EpicsProxy::write_pv_array<double>(std::string m_fieldName, std::vector<double> m_value);
    template void EpicsProxy::write_pv_array<float>(std::string m_fieldName, std::vector<float> m_value);
    template void EpicsProxy::write_pv_array<int>(std::string m_fieldName, std::vector<int> m_value);
    template void EpicsProxy::write_pv_array<short>(std::string m_fieldName, std::vector<short> m_value);
    template void EpicsProxy::write_pv_array<char>(std::string m_fieldName, std::vector<char> m_value);
    template void EpicsProxy::write_pv_array<long>(std::string m_fieldName, std::vector<long> m_value);
    template void EpicsProxy::write_pv_array<unsigned long>(std::string m_fieldName, std::vector<unsigned long> m_value);
}
