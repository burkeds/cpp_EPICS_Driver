#ifndef PV_H
#define PV_H

#include <string>
#include <vector>
#include <any>
#include <stdexcept>
#include <iostream>

#include <cadef.h>
#include <db_access.h>

namespace epics {

class EpicsProxy;

class PV {
    private:
    std::string error;
    std::string deviceName;
    std::string fieldName;
    std::string pvName;
    std::vector<evid> monitors;
    chid channel;

    friend class EpicsProxy;
    
    //Create and destroy channel
    void _create_channel();
    void _clear_channel();
    
    //Reading PVs
    template<typename TypeValue>
    TypeValue _get();
    std::string _get_string();

    //Writing PVs
    template<typename TypeValue>
    void _put(TypeValue value);
    void _put_string(std::string value);

    public:
    PV(std::string m_deviceName, std::string m_fieldName);
    ~PV();
    
    std::string get_name() {return fieldName;};
    chtype get_data_type() {return ca_field_type(channel);};
    chid get_channel() {return channel;};
    std::string get_error() {return error;};
    
    //Cleanup
    void clear_channel();

    //Read
    template<typename TypeValue>
    TypeValue read();
    std::string read_string();

    //Write PVs
    template<typename TypeValue>
    void write(TypeValue newValue);
    void write_string(std::string newValue);

    chtype get_dbr_type(std::string type_name);

    void add_monitor(EpicsProxy* proxy, void (*callback)(struct event_handler_args args));
    void remove_monitor();
};
} // namespace epics
#endif