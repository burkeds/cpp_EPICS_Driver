#ifndef EPICSPROXY_H
#define EPICSPROXY_H

#include <string>
#include <vector>
#include <any>
#include <stdexcept>
#include <iostream>
// #include <errlog.h>

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
public:
    EpicsProxy(std::string m_deviceName = "");
    ~EpicsProxy();
    std::string get_error();
    std::string get_device_name();

    void connect();    
    void disconnect();

    chid get_pv(std::string m_pvName);
    std::vector<chid> get_chid_list();
    std::vector<std::string> get_pv_list();
    
    void clear_channel(std::string m_pvName);

    std::vector<short> get_allowed_types();

    void monitor_pv(chid m_chid, void (*callback)(struct event_handler_args args));
    void unmonitor_pv(evid m_eventID);

    std::any read_pv(std::string m_pvName);
    std::any read_pv(chid m_chid){
        return _read_pv(m_chid);
        };
    
    void write_pv(std::string m_pvName, std::any m_value, std::string m_dataType);
    void write_pv(chid m_chid, std::any m_value, std::string m_dataType) {
        _write_pv(m_chid, m_value, m_dataType);
        };

private:
    void _clear_channel(chid m_chid);

    std::any _read_pv(chid m_chid);
    template<typename TypeValue>
    TypeValue _get(chid m_chid);
    std::string _get_string(chid m_chid);

    void _write_pv(chid m_chid, std::any m_value, std::string m_dataType);
    template<typename TypeValue>
    void _put(chid m_chid, TypeValue value, chtype m_field_type);
    void _put_string(chid m_chid, std::string value, chtype m_field_type);
};

} // namespace epicsproxy

#endif // EPICSPROXY_H
