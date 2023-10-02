#include "PV.h"

PV::PV(std::string m_deviceName, std::string m_fieldName){
    deviceName = m_deviceName;
    pvName = deviceName + m_fieldName;
    _create_channel();
    dataType = ca_field_type(channel);
}

void PV::write(std::any wval, std::string m_dataType) { 
    //Check that the value is a supported field type
    if (std::find(allowed_types.begin(), allowed_types.end(), ca_field_type(channel)) == allowed_types.end()) {
        error = "Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l\n";
        throw std::runtime_error("Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l");
    }

    //Check that the value is not an array
    if (wval.type() == typeid(std::vector<dbr_double_t>) ||
        wval.type() == typeid(std::vector<dbr_float_t>) ||
        wval.type() == typeid(std::vector<dbr_enum_t>) ||
        wval.type() == typeid(std::vector<dbr_short_t>) ||
        wval.type() == typeid(std::vector<dbr_char_t>) ||
        wval.type() == typeid(std::vector<std::string>) ||
        wval.type() == typeid(std::vector<dbr_long_t>)) {
        error = "The value for PV " + std::string(ca_name(channel)) + " is an array\n";
        throw std::runtime_error("The value for PV " + std::string(ca_name(channel)) + " is an array. Arrays are not supported.");
    }
    if (m_dataType == "d") {
        _put<dbr_double_t>(std::any_cast<dbr_double_t>(value), DBR_DOUBLE); 
    } else if (m_dataType == "f") {
        _put<dbr_float_t>(std::any_cast<dbr_float_t>(value), DBR_FLOAT);
    } else if (m_dataType == "t") {
        _put<dbr_enum_t>(std::any_cast<dbr_enum_t>(value), DBR_ENUM);
    } else if (m_dataType == "s") {
        _put<dbr_short_t>(std::any_cast<dbr_short_t>(value), DBR_SHORT);
    } else if (m_dataType == "h") {
        _put<dbr_char_t>(std::any_cast<dbr_char_t>(value), DBR_CHAR);
    } else if (m_dataType == "A40_c") {
        _put_string(std::any_cast<std::string>(value));
    } else if (m_dataType == "l") {
        _put<dbr_long_t>(std::any_cast<dbr_long_t>(value), DBR_LONG);
    } else {
        throw std::runtime_error("Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l");
    }
}

void PV::_create_channel(){
    SEVCHK(ca_create_channel(pvName.c_str(), NULL, NULL, 20, &channel), ("Failed to create channel for PV " + pvName).c_str());
    SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + pvName).c_str());
    };

void PV::_read() {
    //Check element count
    if (ca_element_count(channel) != 1) {
        error = "The element count for PV " + std::string(pvName) + " is not 1\n";
        throw std::runtime_error("The element count for PV " + std::string(pvName) + " is not 1");
    }
    dataType = ca_field_type(channel);        

    if (dataType == DBR_DOUBLE) {
        value = _get<dbr_double_t>();
    } else if (dataType == DBR_FLOAT) {
        value = _get<dbr_float_t>();
    } else if (dataType == DBR_ENUM) {
        value = _get<dbr_enum_t>();
    } else if (dataType == DBR_SHORT) {
        value = _get<dbr_short_t>();
    } else if (dataType == DBR_CHAR) {
        value = _get<dbr_char_t>();
    } else if (dataType == DBR_STRING) {
        value = _get_string();
    } else if (dataType == DBR_LONG) {
        value = _get<dbr_long_t>();
    } else {
        throw std::runtime_error("Unsupported data type. The supported data types are: d, f, t, s, h, A40_c, l");
    }
}

template<typename TypeValue>
TypeValue PV::_get() {
    TypeValue pval;
    SEVCHK(ca_get(dataType, channel, &pval), ("Failed to get value from PV " + pvName).c_str());
    return pval;
}

std::string PV::_get_string() {
    dbr_string_t pValue;
    SEVCHK(ca_get(DBR_STRING, channel, &pValue), ("Failed to get value from PV " + pvName).c_str());
    return std::string(static_cast<const char*>(pValue));
}
