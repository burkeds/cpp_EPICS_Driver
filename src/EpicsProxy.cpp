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

using namespace epicsproxy;


//Public functions
EpicsProxy::~EpicsProxy() {
    //Unmonitor all PVs
    unmonitor_all();
    //Clear all channels
    clear_all_channels();

    //Destroy the EPICS context
    disconnect();

    //Delete all chids and eventIDs
    for (auto const& chid : m_chidList) {
        delete chid;
    }
    m_chidList.clear();

    for (auto const& eventID : m_eventIDList) {
        delete eventID;
    }
    m_eventIDList.clear();
}

void EpicsProxy::connect() {
        SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
        SEVCHK(ca_add_exception_event(exceptionCallback, NULL), "ca_add_exception_event");
        };

void EpicsProxy::clear_all_channels() {
    //Get list of all chids and clear each channel
    for (auto const& chid : m_chidList) {
        _clear_channel(chid);
    }
}

chid* EpicsProxy::get_pv(std::string m_pvName) {
    //Search the name of each chid in the chid list for the requested PV name and return the chid if found
    auto it = std::find_if(m_chidList.begin(), m_chidList.end(), [m_pvName](chid* chid) {return std::string(ca_name(*chid)) == m_pvName;});
    if (it != m_chidList.end()) {
        return *it;
    } else {
        error = "PV " + m_pvName + " not found\n";
        throw std::runtime_error("PV " + m_pvName + " not found");
    }
}

chid* EpicsProxy::pv(std::string m_pvName) {

    //Create channel a new chid
    chid* m_chid = new chid;
    
    SEVCHK(ca_create_channel(m_pvName.c_str(), NULL, NULL, 20, m_chid), ("Failed to create channel for PV " + m_pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + m_pvName).c_str());
    
    // Check that the channel was created
    if (m_chid == NULL) {
        error = "Failed to create channel for PV " + m_pvName + "\n";
        delete m_chid;
        throw std::runtime_error("Failed to create channel for PV " + m_pvName);
    }

    //Append to chid list if it does not share a name with another member of the list
    if (std::find_if(m_chidList.begin(), m_chidList.end(), [m_pvName](chid* chid) {return std::string(ca_name(*chid)) == m_pvName;}) == m_chidList.end()) {
        m_chidList.push_back(m_chid);
    } else {
        delete m_chid;
    }
    return m_chidList.back();
}

std::vector<std::string> EpicsProxy::get_pv_list() {
    std::vector<std::string> pvList;
    for (auto const& chid : m_chidList) {
        pvList.push_back(std::string(ca_name(*chid)));
    }
    return pvList;
}

evid* EpicsProxy::monitor_pv(chid* m_chid, void (*callback)(struct event_handler_args args)) {
    //Check that the ca_state is conn or prev_conn
    if (!((ca_state(*m_chid) == cs_conn) || (ca_state(*m_chid) == cs_prev_conn ))) {
        error = "Channel for PV " + std::string(ca_name(*m_chid)) + " is not connected\n";
        throw std::runtime_error("Channel for PV " + std::string(ca_name(*m_chid)) + " is not connected");
    }
    
    evid* pEventID = new evid;
    SEVCHK(ca_create_subscription(ca_field_type(*m_chid), 0, *m_chid, DBE_VALUE, callback, pEventID, pEventID), "ca_create_subscription");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io");

    //Check that the event ID was created
    if (*pEventID == NULL) {
        error = "Failed to create event ID for PV " + std::string(ca_name(*m_chid)) + "\n";
        throw std::runtime_error("Failed to create event ID for PV " + std::string(ca_name(*m_chid)));
    }

    //Append to the eventID list if not already present
    if (std::find(m_eventIDList.begin(), m_eventIDList.end(), pEventID) == m_eventIDList.end()) {
        m_eventIDList.push_back(pEventID);
    } else {
        delete pEventID;
    }
    return m_eventIDList.back();
}

void EpicsProxy::unmonitor_all() {
    for (auto const& eventID : m_eventIDList) {
        _unmonitor(eventID);
    }
}

//Private funtions
void EpicsProxy::_unmonitor(evid* m_eventID) {
    //Check that the event ID is valid
    if (*m_eventID == NULL) {
        error = "Invalid event ID\n";
        throw std::runtime_error("Invalid event ID");
    }
    //Unmonitor the event ID
    SEVCHK(ca_clear_subscription(*m_eventID), "ca_clear_subscription");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io");
}

void EpicsProxy::_clear_channel(chid* m_chid) {
    //Check that the chid is valid
    if (m_chid == NULL) {
        error = "Invalid channel ID\n";
        throw std::runtime_error("Invalid channel ID");
    }

    //Clear the channel
    ca_clear_channel(*m_chid);
    ca_pend_io(5.0);

    //Remove the chid from the chid list
    m_chidList.erase(std::remove(m_chidList.begin(), m_chidList.end(), m_chid), m_chidList.end());
}

std::any EpicsProxy::_read_pv(chid* m_chid) {
    std::any pv_value;
    chtype field_type = ca_field_type(*m_chid);
    const char* m_pvName = ca_name(*m_chid);
    //Check that the field type is supported
    if (std::find(allowed_types.begin(), allowed_types.end(), field_type) == allowed_types.end()) {
        error = "Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l\n";
        throw std::runtime_error("Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l");
    }

    //Check the element count
    if (ca_element_count(*m_chid) != 1) {
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
TypeValue EpicsProxy::_get(chid* m_chid) {
    TypeValue value;
    std::string m_pvName = ca_name(*m_chid);
    short m_field_type = ca_field_type(*m_chid);
    SEVCHK(ca_get(m_field_type, *m_chid, &value), ("Failed to get value from PV " + m_pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + m_pvName).c_str());
    return value;
}

std::string EpicsProxy::_get_string(chid* m_chid) {
    dbr_string_t pValue;
    const char* m_pvName = ca_name(*m_chid);
    SEVCHK(ca_get(DBR_STRING, *m_chid, &pValue), ("Failed to get value from PV " + std::string(m_pvName)).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + std::string(m_pvName)).c_str());
    return std::string(static_cast<const char*>(pValue));
}

void EpicsProxy::_write_pv(chid* m_chid, std::any value, std::string m_dataType) {
    
    //Check that the value is a supported field type
    if (std::find(allowed_types.begin(), allowed_types.end(), ca_field_type(*m_chid)) == allowed_types.end()) {
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
        error = "The value for PV " + std::string(ca_name(*m_chid)) + " is an array\n";
        throw std::runtime_error("The value for PV " + std::string(ca_name(*m_chid)) + " is an array. Arrays are not supported.");
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
void EpicsProxy::_put(chid* mChid, TypeValue value, chtype m_field_type) {
    std::string m_pvName = ca_name(*mChid);
    SEVCHK(ca_put(m_field_type, *mChid, &value), ("Failed to put value to PV " + m_pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + m_pvName).c_str());
}

void EpicsProxy::_put_string(chid* m_chid, std::string value, chtype m_field_type) {
    std::string m_pvName = ca_name(*m_chid);
    SEVCHK(ca_put(m_field_type, *m_chid, value.c_str()), ("Failed to put value to PV " + std::string(m_pvName)).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + std::string(m_pvName)).c_str());
}
