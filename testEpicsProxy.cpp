#include <iostream>
#include <any>

#include "EpicsProxy.h"
#include "callbacks.h"

using namespace epicsproxy;

int main() {
    try {
        EpicsProxy proxy("test");
        proxy.connect();
        proxy.pv("INSTRUMENT");
        proxy.pv("sans:galilMotor[guidehall_galil]:D-.VAL");

        chid* m_chid1 = proxy.get_pv("INSTRUMENT");
        chid* m_chid2 = proxy.get_pv("sans:galilMotor[guidehall_galil]:D-.VAL");
        
        
        //Read the value
        std::any value1 = proxy.read_pv(m_chid1);
        std::cout << "Value: " << std::any_cast<std::string>(value1) << std::endl;
        //Read the value
        std::any value2 = proxy.read_pv(m_chid2);
        std::cout << "Value: " << std::any_cast<double>(value2) << std::endl;

        //Write a new value
        proxy.write_pv(m_chid1, std::any(std::string("Test")), "A40_c");

        //Read the value again
        value1 = proxy.read_pv(m_chid1);
        std::cout << "Value: " << std::any_cast<std::string>(value1) << std::endl;

        proxy.clear_all_channels();
        proxy.disconnect();
   
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}