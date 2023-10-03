#include <iostream>
#include <any>

#include "EpicsProxy.h"

int main() {
    try {
        //List of PVs to create
        std::string pvName = std::string(".VAL");

        //Initialize the EPICS context
        EpicsProxy proxy;
        proxy.init("INSTRUMENT", {pvName});
        
        //Read
        proxy.read_all();

        std::any value = proxy.get_pv_value(pvName);
        std::cout << "Value of PV " << pvName << " is " << std::any_cast<std::string>(value) << std::endl;
   
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}