/**
 * @brief The EpicsProxy class provides a C++ wrapper around the EPICS library for interfacing with EPICS PVs.
 *
 * The EpicsProxy class provides a simple and convenient way to interact with EPICS PVs from C++ code. It encapsulates the low-level details of the EPICS library and provides a high-level interface for reading and writing PV values, monitoring PV changes, and handling PV events.
 *
 * To use the EpicsProxy class, simply create an instance of the class with the name of the device you want to connect to, and then use the public member functions to interact with the PVs. The class takes care of connecting to the EPICS server, creating the necessary channels, and handling the communication with the PVs.
 *
 * Values read from PVs are returned as std::any objects and must be appropriately cast to the desired type. Values written to PVs must be of a valid type. The allowed data types are: double('t'), float('f'), enum('t'), short('s'), char('h'), string('A40_c'), long('l')
 * 
 * Example usage:
 *
 * ```
 * EpicsProxy proxy("my_device"); // Where my_device is a nickname for the device
 * proxy.connect();
 * channelID_1 = pv("pvName1");
 * channelID_2 = pv("pvName2");
 * std::any value = proxy.read_pv("pvName1");
 * proxy.write_pv("pvName1", 3.14, 'd');
 * proxy.monitor_pv(channelID, my_callback, callback); @note Do not write a new data type to a PV that is being monitored
 * ```
 * @note The allowed data types are: double('t'), float('f'), enum('t'), short('s'), char('h'), string('A40_c'), long('l')
 * @note Arrays are not supported in this version of EpicsProxy
 * @note The EpicsProxy class requires the EPICS library to be installed on the system and linked with the application.
 * @version 1.1
 */

#include "EpicsProxy.h"

namespace epics{

void EpicsProxy::init(std::string m_deviceName,
                      std::vector<std::string> m_pvNames,
                      std::string ca_addr_list,
                      std::string ca_auto_addr_list,
                      double ca_conn_tmo,
                      double ca_beacon_period,
                      double ca_repeater_port,
                      double ca_server_port,
                      double ca_max_array_bytes,
                      double ts_min_west) {
    //Configure channel access
    setenv("EPICS_CA_ADDR_LIST", ca_addr_list.c_str(), 1);
    setenv("EPICS_CA_AUTO_ADDR_LIST", ca_auto_addr_list.c_str(), 1);
    setenv("EPICS_CA_CONN_TMO", std::to_string(ca_conn_tmo).c_str(), 1);
    setenv("EPICS_CA_BEACON_PERIOD", std::to_string(ca_beacon_period).c_str(), 1);
    setenv("EPICS_CA_REPEATER_PORT", std::to_string(ca_repeater_port).c_str(), 1);
    setenv("EPICS_CA_SERVER_PORT", std::to_string(ca_server_port).c_str(), 1);
    setenv("EPICS_CA_MAX_ARRAY_BYTES", std::to_string(ca_max_array_bytes).c_str(), 1);
    setenv("EPICS_TS_MIN_WEST", std::to_string(ts_min_west).c_str(), 1);


    //Initialize the EPICS context
    SEVCHK(ca_context_create(ca_enable_preemptive_callback), "Failed to create EPICS context");

    //Set the device name
    deviceName = m_deviceName;

    //Create the PVs
    for (auto m_pvName : m_pvNames) {
        PV* m_pv = new PV(deviceName, m_pvName);
        pvList.push_back(m_pv);
    }
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

//Instantiate the template function for allowed types
    template double EpicsProxy::read_pv<double>(std::string m_fieldName);
    template float EpicsProxy::read_pv<float>(std::string m_fieldName);
    template int EpicsProxy::read_pv<int>(std::string m_fieldName);
    template short EpicsProxy::read_pv<short>(std::string m_fieldName);
    template char EpicsProxy::read_pv<char>(std::string m_fieldName);
    template long EpicsProxy::read_pv<long>(std::string m_fieldName);

    template void EpicsProxy::write_pv<double>(std::string m_fieldName, double m_value);
    template void EpicsProxy::write_pv<float>(std::string m_fieldName, float m_value);
    template void EpicsProxy::write_pv<int>(std::string m_fieldName, int m_value);
    template void EpicsProxy::write_pv<short>(std::string m_fieldName, short m_value);
    template void EpicsProxy::write_pv<char>(std::string m_fieldName, char m_value);
    template void EpicsProxy::write_pv<long>(std::string m_fieldName, long m_value);
    

}
