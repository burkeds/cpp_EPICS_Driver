#ifndef EPICSCALLBACKS_H
#define EPICSCALLBACKS_H

#include <cadef.h>
#include <db_access.h>

#include <string>
#include <vector>
#include <iostream>

#include "EpicsProxy.h"
namespace epics {
void msta_to_nomad_status(EpicsProxy* proxy, double d_msta);
void msta_monitor_callback(struct event_handler_args args);
}

#endif