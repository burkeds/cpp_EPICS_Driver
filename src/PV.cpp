/**
 * @file PV.cpp
 * @brief Implementation of the PV class for accessing EPICS process variables.
 */


#include "PV.h"
#include <unistd.h>

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
void PV::write(TypeValue newValue) {
    _put(newValue);
}

void PV::write_string(std::string newValue) {
    _put_string(newValue);
}

template<typename TypeValue>
void PV::write_array(std::vector<TypeValue> newValue) {
    _put_array(newValue);
}

template<typename TypeValue>
TypeValue PV::read() {
    TypeValue value = _get<TypeValue>();
    return value;
}

std::string PV::read_string() {
    std::string value = _get_string();
    return value;
}

template<typename TypeValue>
std::vector<TypeValue> PV::read_array() {
    std::vector<TypeValue> value = _get_array<TypeValue>();
    return value;
}

template<typename TypeValue>
TypeValue PV::_get() {
    TypeValue pval;
    SEVCHK(ca_get(ca_field_type(channel), channel, &pval), ("Failed to get value from PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str())
    return pval;
}

std::string PV::_get_string() {
    dbr_string_t pValue;
    SEVCHK(ca_get(DBR_STRING, channel, &pValue), ("Failed to get value from PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str())
    return std::string(static_cast<const char*>(pValue));
}

template<typename TypeValue>
std::vector<TypeValue> PV::_get_array() {
    long element_count = ca_element_count(channel);
    chtype field_type = ca_field_type(channel);
    TypeValue* array = new TypeValue[element_count];
    std::vector<TypeValue> pval;
    SEVCHK(ca_array_get(field_type, element_count, channel, array), ("Failed to get value from PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str())
    std::size_t size = static_cast<std::size_t>(element_count);
    pval.resize(size);
    std::copy(array, array + size, pval.begin());
    delete[] array;
    return pval;
}

template<typename TypeValue>
void PV::_put(TypeValue value) {
        chtype field_type = get_dbr_type(typeid(value).name());
        SEVCHK(ca_put(field_type, channel, &value), ("Failed to put value to PV " + pvName).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str())
}

void PV::_put_string(std::string value){
        SEVCHK(ca_put(DBR_STRING, channel, value.c_str()), ("Failed to put value to PV " + std::string(pvName)).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str())
}

template<typename TypeValue>
void PV::_put_array(std::vector<TypeValue> value) {
        TypeValue* array = new TypeValue[value.size()];
        std::size_t num_elements = value.size();
        unsigned long count = static_cast<unsigned long>(num_elements);
        TypeValue first_element = value[0];
        chtype field_type = get_dbr_type(typeid(first_element).name());
        std::copy(value.begin(), value.end(), array);
        SEVCHK(ca_array_put(field_type, count, channel, array), ("Failed to put value to PV " + pvName).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str())
        delete[] array;
}

void PV::_create_channel(bool pend){
    SEVCHK(ca_create_channel(pvName.c_str(), NULL, NULL, 20, &channel), ("Failed to create channel for PV " + pvName).c_str());
    if (pend) {
        SEVCHK(ca_pend_io(5.0), ("Failed to create channel for PV " + pvName).c_str());
    }
    //ca_set_puser(channel, puser);
}

void PV::_clear_channel(){
    SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str());
    SEVCHK(ca_clear_channel(channel), ("Failed to destroy channel for PV " + pvName).c_str());
}

//Instantiate the template function for allowed types
template double PV::read<double>();
template float PV::read<float>();
template int PV::read<int>();
template short PV::read<short>();
template char PV::read<char>();
template long PV::read<long>();
template unsigned long PV::read<unsigned long>();

template std::vector<double> PV::read_array<double>();
template std::vector<float> PV::read_array<float>();
template std::vector<int> PV::read_array<int>();
template std::vector<short> PV::read_array<short>();
template std::vector<char> PV::read_array<char>();
template std::vector<long> PV::read_array<long>();
template std::vector<unsigned long> PV::read_array<unsigned long>();

template void PV::write<double>(double newValue);
template void PV::write<float>(float newValue);
template void PV::write<int>(int newValue);
template void PV::write<short>(short newValue);
template void PV::write<char>(char newValue);
template void PV::write<long>(long newValue);
template void PV::write<unsigned long>(unsigned long newValue);

template void PV::write_array<double>(std::vector<double> newValue);
template void PV::write_array<float>(std::vector<float> newValue);
template void PV::write_array<int>(std::vector<int> newValue);
template void PV::write_array<short>(std::vector<short> newValue);
template void PV::write_array<char>(std::vector<char> newValue);
template void PV::write_array<long>(std::vector<long> newValue);
template void PV::write_array<unsigned long>(std::vector<unsigned long> newValue);

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