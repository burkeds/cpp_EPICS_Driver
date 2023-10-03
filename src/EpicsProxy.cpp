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

void EpicsProxy::init(std::string m_deviceName, std::vector<std::string> m_pvNames) {
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

EpicsProxy::~EpicsProxy() {
    std::cout << "Destroying EpicsProxy" << std::endl;
    //Clear all channels
    clear_all_channels();

    

    //Destruct the contents of all pointers in pvList
    for (PV* m_pv : pvList) {
        delete m_pv;
    }
    pvList.clear();

    //Destroy the EPICS context
    ca_context_destroy();
}

void EpicsProxy::clear_all_channels() {
    std::cout << "Clearing all channels" << std::endl;
    for (PV* m_pv : pvList) {
        
        std::cout << "Clearing channel for PV ASK " << m_pv->get_name() << std::endl;
        pend();
        std::cout << "Calling clear_channel" << std::endl;
        //m_pv->clear_channel();
        std::cout << "Finishing clear_channel" << std::endl;
    }
}

void EpicsProxy::read_all() {
    for (PV* m_pv : pvList) {
        m_pv->read();
    }
    pend();
}

void EpicsProxy::write_pv(std::string m_fieldName, std::any m_value, std::string m_dataType) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->write(m_value, m_dataType);
            pend();
            return;
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

void EpicsProxy::read_pv(std::string m_fieldName) {
	    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            m_pv->read();
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}

std::any EpicsProxy::get_pv_value(std::string m_fieldName) {
    for (PV* m_pv : pvList) {
        if (m_pv->get_name() == m_fieldName) {
            return m_pv->get_value();
        }
    }
    throw std::runtime_error("PV " + m_fieldName + " not found");
}
}
