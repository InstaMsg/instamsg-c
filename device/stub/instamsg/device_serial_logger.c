/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "../driver/include/globals.h"

/*
 * This method MUST connect the underlying medium (even if it means to retry continuously).
 */
void init_serial_logger()
{
}


/*
 * This method writes first "len" bytes from "buffer" onto the serial-logger-interface.
 *
 * This is a blocking function. So, either of the following must hold true ::
 *
 * a)
 * All "len" bytes are written.
 * In this case, SUCCESS must be returned.
 *
 *                      OR
 * b)
 * An error occurred while writing.
 * In this case, FAILURE must be returned immediately.
 */
int serial_logger_write(unsigned char* buffer, int len)
{
#error "Function not implemented."
}


/*
 * This method MUST release the underlying medium (even if it means to retry continuously).
 * But if it is ok to re-connect without releasing the underlying-system-resource, then this can be left empty.
 */
void release_serial_logger()
{
}


