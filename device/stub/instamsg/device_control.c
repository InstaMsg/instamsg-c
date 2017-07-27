#include "../../../common/instamsg/driver/include/control.h"


/*
 * Process the control-command, as per the items filled in the structure ControlCommandParams, defined in control.h
 *
 * For example, if following is the json received from server on the topic "instamsg/clients/#id#/controlaction" :
 *
 * {"data" : "GPIO/DO001","time" : "1501131076", "ttl" : "900", "v" : "1.0", "port" : {"port_name" : "gpio", "port_address" : "", "host_address" : "", "host_port" : ""}}
 *
 * then the mapping will be ::
 *
 *      controlCommandParams->portName      =   "gpio"
 *      controlCommandParams->portAddress   =   ""
 *      controlCommandParams->hostAddress   =   ""
 *      controlCommandParams->hostPort      =   ""
 *      controlCommandParams->command       =   "GPIO/DO001"
 *
 *
 * Note that, when the code lands here, all the pre-processing regarding the json-fields "time", "ttl", "v" etc. has already been done,
 * so device-implementors need not worry about that.
 *
 * Device-implementors just need to decide how to process on #controlCommandParams->command#, after seeing the port-parameters.
 */
void do_process_control_action_command(struct ControlCommandParams *controlCommandParams)
{
}
