#ifndef __WDOG_10XX_H__
#define __WDOG_10XX_H__

//--------------------------------------------------------------------+
// Function Declarations
//--------------------------------------------------------------------+

// watchdog_check:
// This function checks to see what to do with the watchdog situation
// call it from the board_init_post() function
// threshold:
//       This is the number of consecutive watchdog reboots that are
//       tolerated by the system before erasing the app and staying in
//       bootloader mode.
//       Set to 0 to erase the app if the watchdog ever triggers.
//       Set to 1 to erase the app if more than 1 watchdog event
//       triggered without the app resetting the watchdog count by
//       setting WATCHDOG_COUNT_REG = WATCHDOG_COUNT_MAGIC.
//
void board_wdog_10xx_check(unsigned int threshold);

// wdog_10xx_init:
// call this function to start the watchdog.  This should be call *just*
// prior to starting the application, so call it from
// board_teardown_post()
//
// timeout_ms:  the time to set the watchdog for.
//              on i.MX RT devices, the valid range is 500 to 127000
// 
// So, the application must do 2 things:
//    1) feed the dog at a rate higher than timeout_ms
//    2) set WATCHDOG_COUNT_REG = WATCHDOG_COUNT_MAGIC
void board_wdog_10xx_init(float timeout_ms);
#endif
