# cpp_EPICS_Driver

A simple driver to interface the EPICS control system to C++

## Installation

To install the driver, you need to have EPICS Base version 7.0.3 or later installed on your system. You can download EPICS Base from the [EPICS website](https://epics.anl.gov/base/R7-0/3.php).

Once you have EPICS Base installed, you can build the driver by running the following commands:

git clone https://github.com/burkeds/cpp_EPICS_Driver.git
cd cpp_EPICS_Driver $ make


This will build the `testEpicsProxy` executable, which you can run to test the driver.

## Usage

To use the driver in your own C++ code, you need to include the `EpicsProxy.h` header file and link against the `libEpicsProxy.a` library. Here's an example of how to use the driver to read the value of a PV:

```cpp
#include "EpicsProxy.h"
#include <iostream>

int main()
{
    EpicsProxy proxy("my_device"); // Where my_device is a nickname for the device
	proxy.connect();
 	channelID_1 = proxy.get_pv("pvName1");
 	channelID_2 = proxy.get_pv("pvName2");
 	std::any value = proxy.read_pv("pvName1");
 	proxy.write_pv("pvName1", 3.14, 'd');
 	proxy.monitor_pv(channelID, my_callback, callback);
    return 0;
}

License
This project is released under the Unlicense. See the LICENSE file for details.