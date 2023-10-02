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

using namespace epics;

void EpicsProxy::init(std::string deviceName, std::string pvName, ...) {
    //Initialize the EPICS context
    SEVCHK(ca_context_create(ca_enable_preemptive_callback), "Failed to create EPICS context");

    //Create the PVs
    va_list args;
    va_start(args, pvName);
    while (pvName != NULL) {
        pvList.push_back(PV(deviceName, pvName));
        pvName = va_arg(args, char*);
    }
}


EpicsProxy::~EpicsProxy() {
    //Clear all channels
    clear_all_channels();

    //Destroy the EPICS context
    ca_context_destroy();

    //Delete all entries in pvList
    for (auto m_pv : pvList) {
        m_pv.~PV();
    }
    pvList.clear();
}

void EpicsProxy::clear_all_channels() {
    for (auto m_pv : pvList) {
        m_pv.clear_channel();
    }
}

void EpicsProxy::write_pv(std::string m_fieldName, std::any m_value, std::string m_dataType) {
    for (auto m_pv : pvList) {
        if (m_pv.get_name() == m_fieldName) {
            m_pv.write(m_value, m_dataType);
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

std::any EpicsProxy::read_pv(std::string m_fieldName) {
    for (auto m_pv : pvList) {
        if (m_pv.get_name() == m_fieldName) {
            m_pv.read();
            return m_pv.get_value();
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}
