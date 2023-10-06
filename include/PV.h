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
    //void* puser;

    friend class EpicsProxy;
    
    //Create and destroy channel
    void _create_channel(bool pend = true);
    void _clear_channel(bool pend = true);
    
    //Reading PVs
    template<typename TypeValue>
    TypeValue _get(bool pend = true);
    std::string _get_string(bool pend = true);

    //Writing PVs
    template<typename TypeValue>
    void _put(TypeValue value, bool pend = true);
    void _put_string(std::string value, bool pend = true);

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
    TypeValue read(bool pend = true);
    std::string read_string(bool pend = true);

    //Write PVs
    template<typename TypeValue>
    void write(TypeValue newValue, bool pend = true);
    void write_string(std::string newValue, bool pend = true);

    chtype get_dbr_type(std::string type_name);

    void add_monitor(EpicsProxy* proxy, void (*callback)(struct event_handler_args args));
    void remove_monitor();
};
} // namespace epics
#endif