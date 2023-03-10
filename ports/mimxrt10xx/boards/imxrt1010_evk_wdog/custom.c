#include "boards.h"
#include "board_api.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "clock_config.h"
#include "tusb.h"
#include "wdog_10xx.h"

void board_gpio_configure(void);


//--------------------------------------------------------------------+
// override weak callbacks
//--------------------------------------------------------------------+
void board_init2(void) {
    const int threshold = 1;
    board_wdog_10xx_check(threshold);
    board_gpio_configure();
}

void board_teardown2(void) {
    board_wdog_10xx_init(10000);
}

void board_gpio_configure(void) {
    // Set up the GPIO Mux for the LED.
    if (LED_PORT == GPIO2) {
        IOMUXC_GPR->GPR26 =(IOMUXC_GPR->GPR26 | (1<<LED_PIN));
    } else if (LED_PORT == GPIO1) {
        IOMUXC_GPR->GPR26 =(IOMUXC_GPR->GPR26 & (~(1<<LED_PIN)));
    } else {
    }
}
