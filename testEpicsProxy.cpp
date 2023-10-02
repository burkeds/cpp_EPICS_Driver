#include <iostream>
#include <any>

#include "EpicsProxy.h"

using namespace epicsproxy;

int main() {
    try {
        //List of PVs to create
        std::string pvName = std::string(".VAL");

        //Initialize the EPICS context
        EpicsProxy proxy;
        proxy.init("test", pvName, NULL);
        
        //Read
        proxy.read();

        proxy.clear_all_channels();
        proxy.disconnect();
   
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}