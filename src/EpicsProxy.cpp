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
 * channelID_1 = proxy.get_pv("pvName1");
 * channelID_2 = proxy.get_pv("pvName2");
 * std::any value = proxy.read_pv("pvName1");
 * proxy.write_pv("pvName1", 3.14, 'd');
 * proxy.monitor_pv(channelID, my_callback, callback); @note Do not write a new data type to a PV that is being monitored
 * ```
 * @note The allowed data types are: double('t'), float('f'), enum('t'), short('s'), char('h'), string('A40_c'), long('l')
 * @note Arrays are not supported in this version of EpicsProxy
 * @note The EpicsProxy class requires the EPICS library to be installed on the system and linked with the application.
 * @version 1.0.0
 */
#include <vector>
#include <any>
#include <stdexcept>
#include <iostream>
#include <map>
#include <algorithm>
#include <string>

#include "EpicsProxy.h"

class EpicsProxy {

public: 
    EpicsProxy(std::string m_deviceName = "") {
        //Set the device name
        deviceName = m_deviceName;
    }

    ~EpicsProxy() {
        //Get list of all chids and clear each channel
        std::vector<chid> chidList = get_chid_list();
        for (auto const& chid : chidList) {
            _clear_channel(chid);
        }
        // Destroy the context
        disconnect();
    }
    
    std::string get_error() {
        return error;
    }
    
    std::string get_device_name() {
        return deviceName;
    }

    void connect() {
        //Initialize the CA library
        SEVCHK(ca_context_create(ca_enable_preemptive_callback), "ca_context_create");
    }

    void disconnect() {
        // Destroy the context
        ca_context_destroy();
    }

    chid get_pv(std::string m_pvName) {
        chid m_chid;
        SEVCHK(ca_create_channel(m_pvName.c_str(), NULL, NULL, 20, &m_chid), ("Failed to create channel for PV " + m_pvName).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + m_pvName).c_str());

        // Check that the channel was created
        if (m_chid == NULL) {
            error = "Failed to create channel for PV " + m_pvName + "\n";
            throw std::runtime_error("Failed to create channel for PV " + m_pvName);
        }

        // Check that the channel is connected
        if (ca_state(m_chid) != cs_conn) {
            error = "Channel for PV " + m_pvName + " is not connected\n";
            throw std::runtime_error("Channel for PV " + m_pvName + " is not connected");
        }

        // Add m_chid as a key to the map of chids to eventIDs if not already present
        if (m_chidEventIDMap.find(m_chid) == m_chidEventIDMap.end()) {
            m_chidEventIDMap[m_chid] = std::vector<evid>();
        }
        return m_chid;
    }

    std::vector<chid> get_chid_list() {
        //Get the keys of m_chidEventIDMap and return as a vector
        std::vector<chid> chidList;
        std::map<chid, std::vector<evid>>::iterator it;
        for (it = m_chidEventIDMap.begin(); it != m_chidEventIDMap.end(); it++) {
            chidList.push_back(it->first);
        }
        return chidList;
    }

    std::vector<std::string> get_pv_list() {
        std::vector<chid> chidList = get_chid_list();
        std::vector<std::string> pvList;
        for (auto const& chid : chidList) {
            std::string pvName = ca_name(chid);
            if (pvName.find(deviceName) != std::string::npos) {
                pvList.push_back(pvName);
            }
        }
        if (pvList.empty()) {
            error = "No PVs found for device " + deviceName + "\n";
        }
        return pvList;
    }

    void clear_channel(std::string m_pvName) {
        chid m_chid = get_pv(m_pvName);
        _clear_channel(m_chid);
    }  

    std::vector<short> get_allowed_types() {
        return allowed_types;
    }

    evid monitor_pv(chid m_chid, void (*callback)(struct event_handler_args args)) {
        //Check that the channel is connected
        if (ca_state(m_chid) != cs_conn) {
            error = "Channel for PV " + std::string(ca_name(m_chid)) + " is not connected\n";
            throw std::runtime_error("Channel for PV " + std::string(ca_name(m_chid)) + " is not connected");
        }
        
        //Check if the chid is already in the map, if so, return the first event ID in the vector
        if (m_chidEventIDMap.find(m_chid) != m_chidEventIDMap.end()) {
            return m_chidEventIDMap[m_chid][0];
        }
        
        evid m_eventID;
        SEVCHK(ca_create_subscription(ca_field_type(m_chid), 0, m_chid, DBE_VALUE, callback, NULL, &m_eventID), "ca_create_subscription");
        SEVCHK(ca_pend_io(5.0), "ca_pend_io");

        //Check that the event ID was created
        if (m_eventID == NULL) {
            error = "Failed to create event ID for PV " + std::string(ca_name(m_chid)) + "\n";
            throw std::runtime_error("Failed to create event ID for PV " + std::string(ca_name(m_chid)));
        }

        //Append the event ID to the vector if it is not already present
        if (std::find(m_chidEventIDMap[m_chid].begin(), m_chidEventIDMap[m_chid].end(), m_eventID) == m_chidEventIDMap[m_chid].end()) {
            m_chidEventIDMap[m_chid].push_back(m_eventID);
        }
    }

    void unmonitor_pv(evid m_eventID) {
        //Search map for chid associated with event ID
        std::map<chid, std::vector<evid>>::iterator it;
        chid m_chid;
        for (it = m_chidEventIDMap.begin(); it != m_chidEventIDMap.end(); it++) {
            std::vector<evid> eventIDList = it->second;
            if (std::find(eventIDList.begin(), eventIDList.end(), m_eventID) != eventIDList.end()) {
                m_chid = it->first;
                break;
            }
        }
        

        //Destroy the event ID. This will also unmonitor the PV
        SEVCHK(ca_clear_subscription(m_eventID), "ca_clear_subscription");
        SEVCHK(ca_pend_io(5.0), "ca_pend_io");

        //Check that the event ID was destroyed
        if (m_eventID != NULL) {
            std::string m_pvName = ca_name(m_chid);
            error = "Failed to destroy event ID for PV " + std::string(m_pvName) + "\n";
            throw std::runtime_error("Failed to destroy event ID for PV " + std::string(m_pvName));
        }

        //Remove the event ID from the map but keep the chid
        std::vector<evid> eventIDList = m_chidEventIDMap[m_chid];
        eventIDList.erase(std::remove(eventIDList.begin(), eventIDList.end(), m_eventID), eventIDList.end());
        m_chidEventIDMap[m_chid] = eventIDList;
    }

    std::any read_pv(std::string m_pvName) {
        chid m_chid = get_pv(m_pvName);
        return _read_pv(m_chid);
    }
    
    void write_pv(std::string m_pvName, std::any m_value, std::string m_dataType) {
        chid m_chid = get_pv(m_pvName);
        _write_pv(m_chid, m_value, m_dataType);
    }

private:
    std::string deviceName;
    std::map<chid, std::vector<evid>> m_chidEventIDMap;
    std::vector<short> allowed_types = {DBR_DOUBLE,
                                        DBR_FLOAT,
                                        DBR_ENUM,
                                        DBR_SHORT,
                                        DBR_CHAR,
                                        DBR_STRING,
                                        DBR_LONG};
    std::string error;

    void _clear_channel(chid m_chid) {
        //First check to see if there are event IDs associated with the chid
        //If so, unmonitor the event ID
        if (m_chidEventIDMap.find(m_chid) != m_chidEventIDMap.end()) {
            std::vector<evid> eventIDList = m_chidEventIDMap[m_chid];
            for (auto const& eventID : eventIDList) {
                unmonitor_pv(eventID);
            }
        }
        //Clear the channel
        SEVCHK(ca_clear_channel(m_chid), "ca_clear_channel");
        SEVCHK(ca_pend_io(5.0), "ca_pend_io");

        //Check that the channel was cleared
        if (m_chid != NULL) {
            error = "Failed to clear channel for PV " + std::string(ca_name(m_chid)) + "\n";
            throw std::runtime_error("Failed to clear channel for PV " + std::string(ca_name(m_chid)));
        }

        //Remove the chid from the map
        m_chidEventIDMap.erase(m_chid);
    }

    std::any _read_pv(chid m_chid) {
        std::any pv_value;
        chtype field_type = ca_field_type(m_chid);
        const char* m_pvName = ca_name(m_chid);
        //Check that the field type is supported
        if (std::find(allowed_types.begin(), allowed_types.end(), field_type) == allowed_types.end()) {
            error = "Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l\n";
            throw std::runtime_error("Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l");
        }

        //Check the element count
        if (ca_element_count(m_chid) != 1) {
            error = "The element count for PV " + std::string(m_pvName) + " is not 1\n";
            throw std::runtime_error("The element count for PV " + std::string(m_pvName) + " is not 1");
        }        

        if (field_type == DBR_DOUBLE) {
            pv_value = _get<dbr_double_t>(m_chid);
        } else if (field_type == DBR_FLOAT) {
            pv_value = _get<dbr_float_t>(m_chid);
        } else if (field_type == DBR_ENUM) {
            pv_value = _get<dbr_enum_t>(m_chid);
        } else if (field_type == DBR_SHORT) {
            pv_value = _get<dbr_short_t>(m_chid);
        } else if (field_type == DBR_CHAR) {
            pv_value = _get<dbr_char_t>(m_chid);
        } else if (field_type == DBR_STRING) {
            pv_value = _get_string(m_chid);
        } else if (field_type == DBR_LONG) {
            pv_value = _get<dbr_long_t>(m_chid);
        } else {
            throw std::runtime_error("Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l");
        }
        return pv_value;
    }

    template<typename TypeValue>
    TypeValue _get(chid m_chid) {
        TypeValue value;
        std::string m_pvName = ca_name(m_chid);
        short m_field_type = ca_field_type(m_chid);
        SEVCHK(ca_get(m_field_type, m_chid, &value), ("Failed to get value from PV " + m_pvName).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + m_pvName).c_str());
        return value;
    }

    std::string _get_string(chid m_chid) {
        dbr_string_t pValue;
        const char* m_pvName = ca_name(m_chid);
        SEVCHK(ca_get(DBR_STRING, m_chid, &pValue), ("Failed to get value from PV " + std::string(m_pvName)).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + std::string(m_pvName)).c_str());
        return std::string(static_cast<const char*>(pValue));
    }

    void _write_pv(chid m_chid, std::any value, std::string m_dataType) {
        //Check that the value is a supported field type
        if (std::find(allowed_types.begin(), allowed_types.end(), ca_field_type(m_chid)) == allowed_types.end()) {
            error = "Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l\n";
            throw std::runtime_error("Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l");
        }

        //Check that the value is not an array
        if (value.type() == typeid(std::vector<dbr_double_t>) ||
            value.type() == typeid(std::vector<dbr_float_t>) ||
            value.type() == typeid(std::vector<dbr_enum_t>) ||
            value.type() == typeid(std::vector<dbr_short_t>) ||
            value.type() == typeid(std::vector<dbr_char_t>) ||
            value.type() == typeid(std::vector<std::string>) ||
            value.type() == typeid(std::vector<dbr_long_t>)) {
            error = "The value for PV " + std::string(ca_name(m_chid)) + " is an array\n";
            throw std::runtime_error("The value for PV " + std::string(ca_name(m_chid)) + " is an array. Arrays are not supported.");
        }
        
        if (m_dataType == "d") {
            _put<dbr_double_t>(m_chid, std::any_cast<dbr_double_t>(value), DBR_DOUBLE); 
        } else if (m_dataType == "f") {
            _put<dbr_float_t>(m_chid, std::any_cast<dbr_float_t>(value), DBR_FLOAT);
        } else if (m_dataType == "t") {
            _put<dbr_enum_t>(m_chid, std::any_cast<dbr_enum_t>(value), DBR_ENUM);
        } else if (m_dataType == "s") {
            _put<dbr_short_t>(m_chid, std::any_cast<dbr_short_t>(value), DBR_SHORT);
        } else if (m_dataType == "h") {
            _put<dbr_char_t>(m_chid, std::any_cast<dbr_char_t>(value), DBR_CHAR);
        } else if (m_dataType == "A40_c") {
            _put_string(m_chid, std::any_cast<std::string>(value), DBR_STRING);
        } else if (m_dataType == "l") {
            _put<dbr_long_t>(m_chid, std::any_cast<dbr_long_t>(value), DBR_LONG);
        } else {
            throw std::runtime_error("Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l");
        }
    }

    template<typename TypeValue>
    void _put(chid mChid, TypeValue value, chtype m_field_type) {
        std::string m_pvName = ca_name(mChid);
        SEVCHK(ca_put(m_field_type, mChid, &value), ("Failed to put value to PV " + m_pvName).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + m_pvName).c_str());
    }
    
    void _put_string(chid m_chid, std::string value, chtype m_field_type) {
        std::string m_pvName = ca_name(m_chid);
        SEVCHK(ca_put(m_field_type, m_chid, value.c_str()), ("Failed to put value to PV " + std::string(m_pvName)).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + std::string(m_pvName)).c_str());
    }
};
