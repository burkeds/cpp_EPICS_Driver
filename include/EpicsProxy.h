#ifndef EPICSPROXY_H
#define EPICSPROXY_H

#include <string>
#include <vector>
#include <any>
#include <stdexcept>
#include <iostream>

#include <cadef.h>
#include <db_access.h>

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

class EpicsProxy {
    //Class Variables
    private:
    std::string error;
    std::string deviceName;
    std::vector<PV> pvList;

public:
    //Constructor and destructor
    EpicsProxy();
    ~EpicsProxy();

    void init(std::string deviceName, std::string pvName, ...);

    // Create PVs
    PV create_PV(std::string m_partialName) {return PV(deviceName, m_partialName);};

    //Cleanup
    void clear_all_channels();

    //Access functions
    std::string get_device_name() {return deviceName;};

    //Read
    void read_all() {for (auto &pv : pvList) {pv.read();}};
    std::any read_pv(std::string m_fieldName);
    void write_pv(std::string m_fieldName, std::any m_value, std::string m_dataType);
};

} // namespace epics

#endif // EPICSPROXY_H
