#include "callbacks.h"

void exceptionCallback(struct exception_handler_args args) {
    // Print the error message to the console
    std::cerr << "CA Exception: " << args.chid << " " << args.ctx << " " << args.op << " " << ca_message(args.stat) << std::endl;
}

void monitorCallback(struct event_handler_args args, evid* pEventID) {
    if (args.status != ECA_NORMAL) {
        // Channel disconnected unexpectedly
        SEVCHK(ca_clear_subscription(*pEventID), "ca_clear_subscription");
    } else {
        // PV value changed
        // Do something with the new value
    }
}
