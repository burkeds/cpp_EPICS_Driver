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
        //Get the chidlist
        //chid* m_chid = proxy.get_pv("INSTRUMENT");
        chid* m_chid = proxy.get_pv("INSTRUMENT");
        
        //Read the value
        std::any value = proxy.read_pv(m_chid);
        std::cout << "Value: " << std::any_cast<std::string>(value) << std::endl;

        //Write a new value
        proxy.write_pv(m_chid, std::any(std::string("Test")), "A40_c");

        //Read the value again
        value = proxy.read_pv(m_chid);
        std::cout << "Value: " << std::any_cast<std::string>(value) << std::endl;

        proxy.clear_all_channels();
        proxy.disconnect();
   
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}