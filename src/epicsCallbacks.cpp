/**
 * @file epicsCallbacks.cpp
 * @brief Implementation of the callback functions for EPICS process variables.
 */

#include "epicsCallbacks.h"
#include <cadef.h>

/*
This callback is called whenever the MSTA PV changes.
It takes the motor status (MSTA) value and translates it into
status values expected by the NOMAD GUI (McMasterAxisDef.h). 
It then writes this to the currentStatus member of EpicsProxy using the EpicsProxy
method set_current_status.
A void* to EpicsProxy is passed to the callback as the first argument.
*/

/*
Status values expected by the NOMAD GUI (McMasterAxisDef.h):
static const int32 ERROR_STATUS = 0x1; //! Error/OK
static const int32 RUNNING_STATUS = 0x2; //! Axis Running/Idle
static const int32 HIGH_HARDSTOP = 0x4; //! Limit switch 1 active/inactive (limit switch set to plus)
static const int32 LOW_HARDSTOP = 0x8; //! Limit switch 2 active/inactive (limit switch set to minus)
static const int32 ACHIEVED_STATUS = 0x10; //! Position achieved. Is reset using start and stop
static const int32 NOT_SYNCHRONIZED_STATUS = 0x20; //! Axis is not synchronized
static const int32 USER_STOP_STATUS = 0x40; //! “User Stop Bit”: is activated when the axis is idle, as either a STOP occurred in the main computer or a limit switch was activated
static const int32 JOB_ERROR_STATUS = 0x80; //!
*/

/*
The motor status is of type ULONG, a 32 bit unsigned integer.
Status values of .MSTA from the epics motor record :
 	The motor status as received from the hardware.  The MSTA bits are defined as follows:

        1.  DIRECTION: last raw direction; (0:Negative, 1:Positive)
        2.  DONE: motion is complete.
        3.  PLUS_LS: plus limit switch has been hit.
        4.  HOMELS: state of the home limit switch.
        5.  Unused
        6.  POSITION: closed-loop position control is enabled.
        7.  SLIP_STALL: Slip/Stall detected (eg. fatal following error)
        8.  HOME: if at home position.
        9.  PRESENT: encoder is present.
        10. PROBLEM: driver stopped polling, or hardware problem
        11. MOVING: non-zero velocity present.
        12. GAIN_SUPPORT: motor supports closed-loop position control.
        13. COMM_ERR: Controller communication error.
        14. MINUS_LS: minus limit switch has been hit.
        15. HOMED: the motor has been homed.

    The record is put into MAJOR STATE alarm if either SLIP_STALL or PROBLEM bits are detected. If HLSV is set, then the record is put into HIGH alarm if either a high soft limit or hard limit (PLUS_LS) has been reached. Similary for the low limits. 

*/

/*
Status translation:
Bit       MSTA        :   NOMAD 
============================================================ 
0         Direction   :   N/A
1         Done        :   ACHIEVED_STATUS
2         Plus_LS     :   HIGH_HARDSTOP
3         HomeLS      :   N/A
4         Unused      :   N/A
5         Position    :   N/A
6         Slip_Stall  :   ERROR_STATUS
7         Home        :   ACHIEVED_STATUS
8         Present     :   N/A
9         Problem     :   ERROR_STATUS
10        Moving      :   RUNNING_STATUS
11        Gain_Support:   N/A
12        Comm_Err    :   ERROR_STATUS
13        Minus_LS    :   LOW_HARDSTOP
14        Homed       :   ACHIEVED_STATUS

*/
namespace epics {

void msta_to_nomad_status(EpicsProxy* proxy, double d_msta) {
    //Translate the MSTA value into status values expected by the NOMAD GUI (McMasterAxisDef.h)
    //This can be called to initialize the currentStatus member of EpicsProxy
    unsigned long msta = static_cast<unsigned long>(d_msta);
    unsigned long NOMAD_STATUS = 0;
        if (msta & (1 << 1)) {
        NOMAD_STATUS |= 0x10; // ACHIEVED_STATUS
    } else if (msta & (1 << 2)) {
        NOMAD_STATUS |= 0x4; // HIGH_HARDSTOP
    } else if (msta & (1 << 6)) {
        NOMAD_STATUS |= 0x2; // RUNNING_STATUS
    } else if (msta & (1 << 7)) {
        NOMAD_STATUS |= 0x10; // ACHIEVED_STATUS
    } else if (msta & (1 << 9)) {
        NOMAD_STATUS |= 0x1; // ERROR_STATUS
    } else if (msta & (1 << 10)) {
        NOMAD_STATUS |= 0x2; // RUNNING_STATUS
    } else if (msta & (1 << 12)) {
        NOMAD_STATUS |= 0x1; // ERROR_STATUS
    } else if (msta & (1 << 13)) {
        NOMAD_STATUS |= 0x8; // LOW_HARDSTOP
    } else if (msta & (1 << 14)) {
        NOMAD_STATUS |= 0x10; // ACHIEVED_STATUS
    } else {
        NOMAD_STATUS |= 0x1; // ERROR_STATUS
    }

    //Set the currentStatus member of EpicsProxy to NOMAD_STATUS
    proxy->set_current_status(NOMAD_STATUS);

}

void msta_monitor_callback(struct event_handler_args args) {
    //Translate the MSTA value into status values expected by the NOMAD GUI (McMasterAxisDef.h)
    //The MSTA value is a 32-bit unsigned integer. The status values are defined in McMasterAxisDef.h
    //The status values are defined in McMasterAxisDef.h
    
    //Get the value of MSTA as a 32-bit unsigned integer
    double d_msta = *(double*)args.dbr;
    unsigned long msta = static_cast<unsigned long>(d_msta);
    unsigned long NOMAD_STATUS = 0;
        if (msta & (1 << 1)) {
        NOMAD_STATUS |= 0x10; // ACHIEVED_STATUS
    } else if (msta & (1 << 2)) {
        NOMAD_STATUS |= 0x4; // HIGH_HARDSTOP
    } else if (msta & (1 << 6)) {
        NOMAD_STATUS |= 0x2; // RUNNING_STATUS
    } else if (msta & (1 << 7)) {
        NOMAD_STATUS |= 0x10; // ACHIEVED_STATUS
    } else if (msta & (1 << 9)) {
        NOMAD_STATUS |= 0x1; // ERROR_STATUS
    } else if (msta & (1 << 10)) {
        NOMAD_STATUS |= 0x2; // RUNNING_STATUS
    } else if (msta & (1 << 12)) {
        NOMAD_STATUS |= 0x1; // ERROR_STATUS
    } else if (msta & (1 << 13)) {
        NOMAD_STATUS |= 0x8; // LOW_HARDSTOP
    } else if (msta & (1 << 14)) {
        NOMAD_STATUS |= 0x10; // ACHIEVED_STATUS
    } else {
        NOMAD_STATUS |= 0x1; // ERROR_STATUS
    }

    //An EpicsProxy* called proxy is in the user argument of the callback

    //Set the currentStatus member of EpicsProxy to NOMAD_STATUS
    ((EpicsProxy*)args.usr)->set_current_status(NOMAD_STATUS);
    
}
}