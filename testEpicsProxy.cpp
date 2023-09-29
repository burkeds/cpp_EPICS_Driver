#include <iostream>
#include <any>

#include "EpicsProxy.h"

int main() {
    try {
        EpicsProxy proxy;
        proxy.connect();

        //Get PV name from user
        std::string pvName;
        std::cout << "Enter the PV name: ";
        std::cin >> pvName;
        //Get data type from user
        std::string type;
        std::cout << "Enter the data type: ";
        std::cin >> type;

        proxy.get_pv(pvName);
        std::any value = proxy.read_pv(pvName);
        std::cout << "The value of the channel is: " << std::any_cast<std::string>(value) << std::endl;
        
        //Get new value from user
        std::string newValue;
        std::cout << "Enter the new value: ";
        std::cin >> newValue;

        //Get new data type from user
        std::string newType;
        std::cout << "Enter the new data type: ";
        std::cin >> newType;

        //Write new value to PV
        proxy.write_pv(pvName, newValue, newType);

        //Read new value from PV
        value = proxy.read_pv(pvName);
        std::cout << "The new value of the channel is: " << std::any_cast<std::string>(value) << std::endl;

        proxy.disconnect();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}