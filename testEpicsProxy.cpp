#include <iostream>
#include <string>

#include "EpicsProxy.h"

using namespace epics;

int main() {
    try {
        std::string ca_addr_list = getenv("EPICS_CA_ADDR_LIST");
        std::string ca_auto_addr_list = getenv("EPICS_CA_AUTO_ADDR_LIST");
        double ca_conn_tmo = 30.0;
        double ca_beacon_period = 15.0;
        double ca_repeater_port = 5065;
        double ca_server_port = 5064;
        double ca_max_array_bytes = 16384;
        double ts_min_west = 360;

        //List of PVs to create
        std::string pvName = std::string(".VAL");

        //Initialize the EPICS context
        EpicsProxy proxy("name");
        proxy.init("sans:motor[sim_motor]:1-",
                  {pvName},
                  ca_addr_list,
                  ca_auto_addr_list,
                  ca_conn_tmo,
                  ca_beacon_period,
                  ca_repeater_port,
                  ca_server_port,
                  ca_max_array_bytes,
                  ts_min_west);

        //Read the PV
        double val = proxy.read_pv<double>(pvName);
        std::cout << "Value of " << pvName << " is " << val << std::endl;

        //Write the PV
        proxy.write_pv<double>(pvName, 3.14);
        val = proxy.read_pv<double>(pvName);
        std::cout << "Value of " << pvName << " is " << val << std::endl;
   
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}