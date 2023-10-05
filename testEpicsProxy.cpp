#include <iostream>
#include <string>
#include <unistd.h>
#include <bitset>

#include "EpicsProxy.h"
#include "epicsCallbacks.h"

using namespace epics;

int main() {
    try {
        struct caConfig conf;

        conf.ca_addr_list = getenv("EPICS_CA_ADDR_LIST");
        conf.ca_auto_addr_list = getenv("EPICS_CA_AUTO_ADDR_LIST");
        conf.ca_conn_tmo = "30.0";
        conf.ca_beacon_period = "15.0";
        conf.ca_repeater_port = "5065";
        conf.ca_server_port = "5064";
        conf.ca_max_array_bytes = "16384";
        conf.ts_min_west = "360";

        //List of PVs to create
        std::string pvName = std::string(".VAL");
        std::string pvStatus = std::string(".MSTA");
        std::string pvReadback = std::string(".RBV");
        std::string pvStop = std::string(".STOP");
        std::vector<std::string> pvNames = {pvName, pvStatus, pvReadback, pvStop};

        //Initialize the EPICS context
        EpicsProxy proxy("name");
        proxy.init("sans:motor[sim_motor]:2-",
                  pvNames,
                  conf);

        //Add a monitor to the status PV
        proxy.add_monitor(pvStatus, &proxy, &epics::msta_monitor_callback);

        //Read the current position
        double pos = proxy.read_pv<double>(pvReadback);
        std::cout << "Current position: " << pos << std::endl;
        unsigned long stat = proxy.get_current_status();
        std::cout << "Status: " << stat << std::endl;

        //Write a new position
        proxy.write_pv<double>(pvName, 10.0);

        while (pos < 10.0) {
            pos = proxy.read_pv<double>(pvReadback);
            std::cout << "Current position: " << pos << std::endl;
            stat = proxy.get_current_status();
        std::cout << "Status: " << stat << std::endl;
            sleep(1.0);
        }

        //Read the final position
        std::cout << "Final position: " << proxy.read_pv<double>(pvReadback) << std::endl;
        stat = proxy.get_current_status();
        std::cout << "Status: " << stat << std::endl;

   
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}