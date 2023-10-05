#ifndef EPICSPROXY_H
#define EPICSPROXY_H

#include <string>
#include <vector>
#include <any>
#include <stdexcept>
#include <iostream>

#include <cadef.h>
#include <db_access.h>
#include <cstdarg>

#include "PV.h"
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

namespace epics {

struct caConfig {
    const char* ca_addr_list;
    const char* ca_auto_addr_list;
    const char* ca_conn_tmo;
    const char* ca_beacon_period;
    const char* ca_repeater_port;
    const char* ca_server_port;
    const char* ca_max_array_bytes;
    const char* ts_min_west;
};

class EpicsProxy {
    //Class Variables
private:
    std::string error;
    std::string deviceName;
    std::vector<PV*> pvList;
    std::string statusPV;
    unsigned long currentStatus = 0x1;
    std::string axisName;
    std::vector<short> allowed_types = {DBR_DOUBLE,
                                        DBR_FLOAT,
                                        DBR_ENUM,
                                        DBR_SHORT,
                                        DBR_CHAR,
                                        DBR_STRING,
                                        DBR_LONG};

public:
    //Constructor and destructor
    EpicsProxy(std::string name);
    ~EpicsProxy();

    void init(std::string m_deviceName,
              std::vector<std::string> m_pvNames,
              caConfig m_caConfig);
    
    void set_status_pv(std::string m_statusPV) {statusPV = m_statusPV;};
    void set_current_status(unsigned long m_currentStatus) {currentStatus = m_currentStatus;};
    unsigned long get_current_status() {return currentStatus;}

    // Create PVs
    PV create_PV(std::string m_partialName) {return PV(deviceName, m_partialName);};

    //Access functions
    std::string get_device_name() {return deviceName;};
    std::string get_axis_name() {return axisName;};
    std::vector<short> get_allowed_types() {return allowed_types;};

    void add_monitor(std::string m_fieldName, EpicsProxy* proxy, void (*callback)(struct event_handler_args args));
    void remove_monitor(std::string m_fieldName);

    //Read and write functions
    template<typename TypeValue>
    void write_pv(std::string m_fieldName, TypeValue m_value);
    
    void write_pv_string(std::string m_fieldName, std::string m_value);

    template<typename TypeValue>
    TypeValue read_pv(std::string m_fieldName);
    
    std::string read_pv_string(std::string m_fieldName);
};
}
#endif