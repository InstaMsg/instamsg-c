/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/


/*
 * This method returns the minimum-delay achievable via this device.
 */
unsigned int getMinimumDelayPossibleInMicroSeconds()
{
    return 500;
}


/*
 * This method ACTUALLY causes the current-device to go to sleep for the minimum-delay possible.
 */
void minimumDelay()
{
    usleep(500);
}
