#ifndef PV_H
#define PV_H

#include <string>
#include <vector>
#include <any>
#include <stdexcept>
#include <iostream>

#include <cadef.h>
#include <db_access.h>

class PV {
    private:
    std::string error;
    std::string deviceName;
    std::string fieldName;
    std::string pvName;
    std::any value;
    chid channel;
    std::vector<evid*> eventIDs;
    std::string status;
    std::vector<short> allowed_types = {DBR_DOUBLE,
                                        DBR_FLOAT,
                                        DBR_ENUM,
                                        DBR_SHORT,
                                        DBR_CHAR,
                                        DBR_STRING,
                                        DBR_LONG};
    //Initialize the channel
    void _create_channel(){
        SEVCHK(ca_create_channel(pvName.c_str(), NULL, NULL, 20, &channel), ("Failed to create channel for PV " + pvName).c_str());
        };

    //Destroy the channel
    void _clear_channel(){
        //Check if channel is valid
        if (channel == NULL) {
            return;
        }
        std::cout << "Destroying channel for PV _ " << pvName << std::endl;
        SEVCHK(ca_clear_channel(channel), ("Failed to destroy channel for PV " + pvName).c_str());
        };
    
    //Reading PVs
    void _read();
    template<typename TypeValue>
    TypeValue _get();
    std::string _get_string();

    //Writing PVs
    template<typename TypeValue>
    void _put(TypeValue value, chtype m_field_type) {
        SEVCHK(ca_put(m_field_type, channel, &value), ("Failed to put value to PV " + pvName).c_str());
        pend();
        }

    void _put_string(std::string value){
        SEVCHK(ca_put(DBR_STRING, channel, value.c_str()), ("Failed to put value to PV " + std::string(pvName)).c_str());
        pend();
        };

    public:
    PV(std::string m_deviceName, std::string m_fieldName);
    ~PV(){clear_channel();};
    
    std::string get_name() {return fieldName;};
    chtype get_data_type() {return ca_field_type(channel);};
    std::any get_value() {return value;};
    chid get_channel() {return channel;};
    std::vector<short> get_allowed_types() {return allowed_types;};
    std::string get_error() {return error;};

    // Pend and flush IO buffer
    void pend() {SEVCHK(ca_pend_io(5.0), ("Failed to pend IO for PV " + pvName).c_str());};
    void flush() {ca_flush_io();};
    
    //Cleanup
    void clear_channel() {_clear_channel();};
    //Read
    void read() {_read();};
    //Write PVs
    void write(std::any m_value, std::string m_dataType);
};
;
#endif // PV_H