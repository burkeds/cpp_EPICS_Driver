#ifndef EPICSPROXY_H
#define EPICSPROXY_H

#include <string>
#include <vector>
#include <any>
#include <stdexcept>
#include <iostream>
// #include <errlog.h>
#include <map>

#include <cadef.h>
#include <db_access.h>
//This is an attempt to redefine SEVCHK so that it prints to the error variable. It doesn't work.
/*
#define SEVCHK(CODE, MSG) \
    do { \
        if ( (CODE) != ECA_NORMAL ) { \
            errlogSevPrintf(errlogMajor, MSG " failed with status %d\n", (CODE)); \
            error += MSG " failed with status " + std::to_string((CODE)) + "\n"; \
            throw std::runtime_error(MSG " failed with status " + std::to_string((CODE))); \
        } \
    } while (0)
*/

namespace epicsproxy {

class EpicsProxy {
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

    void _clear_channel(chid m_chid);

    std::any _read_pv(chid m_chid);
    template<typename TypeValue>
    TypeValue _get(chid m_chid);
    std::string _get_string(chid m_chid);

    void _write_pv(chid m_chid, std::any m_value, std::string m_dataType);
    template<typename TypeValue>
    void _put(chid m_chid, TypeValue value, chtype m_field_type);
    void _put_string(chid m_chid, std::string value, chtype m_field_type);

public:
    //Constructor and destructor
    EpicsProxy(std::string m_deviceName){deviceName = m_deviceName;};
    ~EpicsProxy();

    //Access functions
    std::string get_error() {return error;};
    std::string get_device_name() {return deviceName;};
    std::vector<short> get_allowed_types() {return allowed_types;};

    void connect() {SEVCHK(ca_context_create(ca_enable_preemptive_callback), "ca_context_create");};

    void disconnect() {ca_context_destroy();};
    

    void clear_channel(std::string m_pvName) {
        chid m_chid = get_pv(m_pvName);
        _clear_channel(m_chid);
    };
    void clear_channel(chid m_chid) {_clear_channel(m_chid);};

    chid get_pv(std::string m_pvName);
    std::vector<chid> get_chid_list();
    std::vector<std::string> get_pv_list();

    evid monitor_pv(chid m_chid, void (*callback)(struct event_handler_args args));
    void unmonitor_pv(evid m_eventID);

    std::any read_pv(std::string m_pvName) {
        chid m_chid = get_pv(m_pvName);
        return _read_pv(m_chid);
    };
    std::any read_pv(chid m_chid){return _read_pv(m_chid);};
    
    void write_pv(std::string m_pvName, std::any m_value, std::string m_dataType) {
        chid m_chid = get_pv(m_pvName);
        _write_pv(m_chid, m_value, m_dataType);
    }
    void write_pv(chid m_chid, std::any m_value, std::string m_dataType) {_write_pv(m_chid, m_value, m_dataType);};
};

} // namespace epicsproxy

#endif // EPICSPROXY_H
