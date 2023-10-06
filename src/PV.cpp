/**
 * @file PV.cpp
 * @brief Implementation of the PV class for accessing EPICS process variables.
 */


#include "PV.h"
#include <unistd.h>
#include <limits.h>
#include <boost/asio/ip/host_name.hpp>

namespace epics {

PV::PV(std::string m_deviceName, std::string m_fieldName){
    fieldName = m_fieldName;
    deviceName = m_deviceName;
    pvName = deviceName + fieldName;
    _create_channel(false);
}

PV::~PV(){
        remove_monitor();
        clear_channel();
}

void PV::clear_channel(){
    _clear_channel();
}

template<typename TypeValue>
void PV::write(TypeValue newValue, bool pend) {
    _put(newValue, pend);
}

void PV::write_string(std::string newValue, bool pend) {
    _put_string(newValue, pend);
}

template<typename TypeValue>
TypeValue PV::read(bool pend) {
    TypeValue value = _get<TypeValue>(pend);
    return value;
}

std::string PV::read_string(bool pend) {
    std::string value = _get_string(pend);
    return value;
}

template<typename TypeValue>
TypeValue PV::_get(bool pend) {
    TypeValue pval;
    //If the field name is .MSTA
    SEVCHK(ca_get(ca_field_type(channel), channel, &pval), ("Failed to get value from PV " + pvName).c_str());
    if (pend) {    
        SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str());
    }
    return pval;
}

std::string PV::_get_string(bool pend) {
    dbr_string_t pValue;
    SEVCHK(ca_get(DBR_STRING, channel, &pValue), ("Failed to get value from PV " + pvName).c_str());
    if (pend) {    
        SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str());
    }
    return std::string(static_cast<const char*>(pValue));
}

template<typename TypeValue>
void PV::_put(TypeValue value, bool pend) {
        chtype field_type = get_dbr_type(typeid(value).name());
        SEVCHK(ca_put(field_type, channel, &value), ("Failed to put value to PV " + pvName).c_str());
        if (pend) {    
        SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str());
    }
}

void PV::_put_string(std::string value, bool pend){
        SEVCHK(ca_put(DBR_STRING, channel, value.c_str()), ("Failed to put value to PV " + std::string(pvName)).c_str());
        if (pend) {    
        SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str());
    }
}

void PV::_create_channel(bool pend){
    SEVCHK(ca_create_channel(pvName.c_str(), NULL, NULL, 20, &channel), ("Failed to create channel for PV " + pvName).c_str());
    if (pend) {
        SEVCHK(ca_pend_io(5.0), ("Failed to create channel for PV " + pvName).c_str());
    }
    //ca_set_puser(channel, puser);
}

void PV::_clear_channel(bool pend){
    SEVCHK(ca_clear_channel(channel), ("Failed to destroy channel for PV " + pvName).c_str());
    if (pend) {    
        SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str());
    }
}

//Instantiate the template function for allowed types
template double PV::read<double>(bool pend);
template float PV::read<float>(bool pend);
template int PV::read<int>(bool pend);
template short PV::read<short>(bool pend);
template char PV::read<char>(bool pend);
template long PV::read<long>(bool pend);
template unsigned long PV::read<unsigned long>(bool pend);

template void PV::write<double>(double newValue, bool pend);
template void PV::write<float>(float newValue, bool pend);
template void PV::write<int>(int newValue, bool pend);
template void PV::write<short>(short newValue, bool pend);
template void PV::write<char>(char newValue, bool pend);
template void PV::write<long>(long newValue, bool pend);
template void PV::write<unsigned long>(unsigned long newValue, bool pend);

// Take a type name from typeid(type).name() and return the corresponding DBR_ type
chtype PV::get_dbr_type(std::string type_name) {
    if (type_name == "d") {
        return DBR_DOUBLE;
    } else if (type_name == "f") {
        return DBR_FLOAT;
    } else if (type_name == "i") {
        return DBR_INT;
    } else if (type_name == "s") {
        return DBR_SHORT;
    } else if (type_name == "c") {
        return DBR_CHAR;
    } else if (type_name == "l") {
        return DBR_LONG;
    } else if (type_name == "m") {
        return DBR_LONG;
    } else if (type_name == "a") {
        return DBR_ENUM;
    } else if (type_name == "std::string") {
        return DBR_STRING;
    } else {
        throw std::runtime_error("Type " + type_name + " not supported");
    }
}

// Add a monitor for the PV and add the event id to the list of monitors
void PV::add_monitor(EpicsProxy* proxy, void (*callback)(struct event_handler_args args)) {
    evid monitor;
    SEVCHK(ca_add_masked_array_event(ca_field_type(channel), 1, channel, callback, proxy, 0.0, 0.0, 0.0, &monitor, DBE_VALUE), ("Failed to add monitor for PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to add monitor for PV " + pvName).c_str());
    monitors.push_back(monitor);
}

void PV::remove_monitor() {
    for (auto monitor : monitors) {
        SEVCHK(ca_clear_event(monitor), ("Failed to remove monitor for PV " + pvName).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to remove monitor for PV " + pvName).c_str());
    }
    monitors.clear();
}
}