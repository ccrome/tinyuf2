# i.MX RT watchdog example

This is an example i.MX RT board that has the bootloader start a watchdog to ensure the system can come back from a bad software update.  In a situation where the user doesn't have any access to buttons, a bad software update could easily brick the system in such a way it would be very costly to bring it back to life.  

The operation is
1. Check the reboot cause.  
   1. If the reboot cause is POR or JTAG, set a reboot-count to 0.  
   1. If the reboot cause was a watchdog timeout, increment the reboot-count.
   1. If the reboot count has exceeded a threshold, something is really wrong, so erase the app and stay in DFU mode awaiting a software update.
1. Just before the main application is started, this board uses the board_teardow2() function to start a watchdog timer.  At this point, there is no turning the watchdog off, ever, so the app must service it.  
1. It's the main app's responsibility to:
   1. Once everything is deemed to be operating normally, reset the reboot-count to 0. This implementation uses the `SNVS->LPGPR[2]` register.  So, `SNVS->LPGPR[2] = 0` should do the trick.
   1. Service the watchdog periodically.  This is a periodic call to `WDOG_Refresh(WDOG1)`.

In principle this code should work identically on any i.MX RT board, but has only been tested on the i.MX RT 1010 so far.