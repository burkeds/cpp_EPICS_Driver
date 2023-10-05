#ifndef EPICSCALLBACKS_H
#define EPICSCALLBACKS_H

#include <cadef.h>
#include <db_access.h>

#include <string>
#include <vector>
#include <iostream>
namespace epics {
void msta_to_nomad_status(EpicsProxy* proxy);
void msta_monitor_callback(struct event_handler_args args);
}

#endif