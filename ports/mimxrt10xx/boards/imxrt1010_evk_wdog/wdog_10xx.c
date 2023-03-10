#include "wdog_10xx.h"
#include "fsl_wdog.h"
#include "tusb.h"
#include "boards.h"
#include "board_api.h"

//--------------------------------------------------------------------+
// Defines
//--------------------------------------------------------------------+
#define WATCHDOG_COUNT_MAGIC (0xab350000)
#define WATCHDOG_COUNT() (WATCHDOG_COUNT_REG & 0x0000FFFF)
#define WATCHDOG_MAGIC_MASK (0xFFFF0000)
#define WATCHDOG_COUNT_RESET() (WATCHDOG_COUNT_REG = WATCHDOG_COUNT_MAGIC)
#define WATCHDOG_COUNT_INC() (WATCHDOG_COUNT_REG++)
#define REBOOT_SRC_WATCHDOG (SRC->SRSR & SRC_SRSR_WDOG_RST_B_MASK)
#define WATCHDOG_COUNT_REG SNVS->LPGPR[2]

void board_wdog_10xx_check(unsigned int threshold) {
    // Start the watchdog
    volatile uint32_t srsr = SRC->SRSR;
    srsr = srsr;
    volatile uint32_t wdog_count = WATCHDOG_COUNT_REG;
    wdog_count = wdog_count;
    if (srsr & (SRC_SRSR_JTAG_RST_B_MASK|SRC_SRSR_JTAG_SW_RST_MASK)) {
        SRC->SRSR = 0xFF;
    }
    // If this is the first reboot from cold initialize the
    // watchdog count.
    if ((WATCHDOG_COUNT_REG & WATCHDOG_MAGIC_MASK) != WATCHDOG_COUNT_MAGIC) {
        WATCHDOG_COUNT_RESET();
    }
    if (REBOOT_SRC_WATCHDOG) {
        TU_LOG1("Reboot source was the watchdog\r\n");
        WATCHDOG_COUNT_INC();
    }
    
    // If there have been more than threshold activations of the
    // watchdog, erase the app and wait for an update.
    if (WATCHDOG_COUNT() > threshold) {
        TU_LOG1("Watchdog count %ld exceeded the threshold %ld\r\n",
                WATCHDOG_COUNT(),
                threshold
            );
        DBL_TAP_REG = DBL_TAP_MAGIC_ERASE_APP;
        WATCHDOG_COUNT_RESET();
    }
}

void board_wdog_10xx_init(float timeout_ms) {
    if (timeout_ms < 500)
        timeout_ms = 500;
    if (timeout_ms >= 127000)
        timeout_ms = 127000;

    uint16_t timeout_value = (int)((timeout_ms * 2 / 1000) + 0.5);
    if (timeout_value > 255)
        // make 100% sure the math above doesn't fail.
        timeout_value = 255;
    wdog_config_t config;
    WDOG_GetDefaultConfig(&config);
    config.timeoutValue = timeout_value;
    config.workMode.enableWait = false;
    config.workMode.enableStop = false;
    config.workMode.enableDebug = true;
    WDOG_Init(WDOG1, &config);
    WDOG_DisablePowerDownEnable(WDOG1);
}

