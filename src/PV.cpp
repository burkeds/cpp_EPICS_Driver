#include "PV.h"

PV::PV(std::string m_deviceName, std::string m_fieldName){
    fieldName = m_fieldName;
    deviceName = m_deviceName;
    pvName = deviceName + fieldName;
    _create_channel();
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
TypeValue PV::read() {
    TypeValue value = _get<TypeValue>();
    return value;
}

std::string PV::read_string() {
    std::string value = _get_string();
    return value;
}

template<typename TypeValue>
TypeValue PV::_get() {
    TypeValue pval;
    SEVCHK(ca_get(ca_field_type(channel), channel, &pval), ("Failed to get value from PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str());
    return pval;
}

std::string PV::_get_string() {
    dbr_string_t pValue;
    SEVCHK(ca_get(DBR_STRING, channel, &pValue), ("Failed to get value from PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to get value from PV " + pvName).c_str());
    return std::string(static_cast<const char*>(pValue));
}

template<typename TypeValue>
void PV::_put(TypeValue value) {
        chtype field_type = get_dbr_type(typeid(value).name());
        SEVCHK(ca_put(field_type, channel, &value), ("Failed to put value to PV " + pvName).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to put value to PV " + pvName).c_str());
}

void PV::_put_string(std::string value){
        SEVCHK(ca_put(DBR_STRING, channel, value.c_str()), ("Failed to put value to PV " + std::string(pvName)).c_str());
        SEVCHK(ca_pend_io(5.0), ("Failed to put value to PV " + std::string(pvName)).c_str());
}

void PV::_create_channel(){
    SEVCHK(ca_create_channel(pvName.c_str(), NULL, NULL, 20, &channel), ("Failed to create channel for PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to create channel for PV " + pvName).c_str());
    }

void PV::_clear_channel(){
    SEVCHK(ca_clear_channel(channel), ("Failed to destroy channel for PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to destroy channel for PV " + pvName).c_str());
    }

//Instantiate the template function for allowed types
template double PV::read<double>();
template float PV::read<float>();
template int PV::read<int>();
template short PV::read<short>();
template char PV::read<char>();
template long PV::read<long>();

template void PV::write<double>(double newValue);
template void PV::write<float>(float newValue);
template void PV::write<int>(int newValue);
template void PV::write<short>(short newValue);
template void PV::write<char>(char newValue);
template void PV::write<long>(long newValue);

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
    } else if (type_name == "a") {
        return DBR_ENUM;
    } else if (type_name == "std::string") {
        return DBR_STRING;
    } else {
        throw std::runtime_error("Type " + type_name + " not supported");
    }
}