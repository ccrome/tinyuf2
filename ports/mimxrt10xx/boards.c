/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ha Thach for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "board_api.h"

#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_clock.h"
#include "fsl_ocotp.h"
#include "fsl_lpuart.h"
#include "clock_config.h"

#include "tusb.h"


// needed by fsl_flexspi_nor_boot
const uint8_t dcd_data[] = { 0x00 };

void board_init(void)
{
  // Init clock
  BOARD_BootClockRUN();
  //SystemCoreClockUpdate();

  // Enable IOCON clock
  CLOCK_EnableClock(kCLOCK_Iomuxc);

  board_timer_stop();

#ifdef LED_PIN
  IOMUXC_SetPinMux( LED_PINMUX, 0U);
  IOMUXC_SetPinConfig( LED_PINMUX, 0x10B0U);

  gpio_pin_config_t led_config = { kGPIO_DigitalOutput, 0, kGPIO_NoIntmode };
  GPIO_PinInit(LED_PORT, LED_PIN, &led_config);
#endif

#if NEOPIXEL_NUMBER
  IOMUXC_SetPinMux( NEOPIXEL_PINMUX, 0U);
  IOMUXC_SetPinConfig( NEOPIXEL_PINMUX, 0x10B0U);

  gpio_pin_config_t neopixel_config = { kGPIO_DigitalOutput, 0, kGPIO_NoIntmode };
  GPIO_PinInit(NEOPIXEL_PORT, NEOPIXEL_PIN, &neopixel_config);
#endif

#ifdef BUTTON_PIN
  IOMUXC_SetPinMux( BUTTON_PINMUX, 0U);
  gpio_pin_config_t button_config = { kGPIO_DigitalInput, 0, kGPIO_IntRisingEdge, };
  GPIO_PinInit(BUTTON_PORT, BUTTON_PIN, &button_config);
#endif

#if defined(UART_DEV) && CFG_TUSB_DEBUG
  // Enable UART when debug log is on
  IOMUXC_SetPinMux( UART_TX_PINMUX, 0U);
  IOMUXC_SetPinMux( UART_RX_PINMUX, 0U);
  IOMUXC_SetPinConfig( UART_TX_PINMUX, 0x10B0u);
  IOMUXC_SetPinConfig( UART_RX_PINMUX, 0x10B0u);

  lpuart_config_t uart_config;
  LPUART_GetDefaultConfig(&uart_config);
  uart_config.baudRate_Bps = BOARD_UART_BAUDRATE;
  uart_config.enableTx = true;
  uart_config.enableRx = true;
  LPUART_Init(UART_DEV, &uart_config, (CLOCK_GetPllFreq(kCLOCK_PllUsb1) / 6U) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U));
#endif
}

void board_dfu_init(void)
{
  // Clock
  CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
  CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);

#ifdef USBPHY1
  USBPHY_Type* usb_phy = USBPHY1;
#else
  USBPHY_Type* usb_phy = USBPHY;
#endif

  // Enable PHY support for Low speed device + LS via FS Hub
  usb_phy->CTRL |= USBPHY_CTRL_SET_ENUTMILEVEL2_MASK | USBPHY_CTRL_SET_ENUTMILEVEL3_MASK;

  // Enable all power for normal operation
  usb_phy->PWD = 0;

  // TX Timing
  uint32_t phytx = usb_phy->TX;
  phytx &= ~(USBPHY_TX_D_CAL_MASK | USBPHY_TX_TXCAL45DM_MASK | USBPHY_TX_TXCAL45DP_MASK);
  phytx |= USBPHY_TX_D_CAL(0x0C) | USBPHY_TX_TXCAL45DP(0x06) | USBPHY_TX_TXCAL45DM(0x06);
  usb_phy->TX = phytx;

  // USB1
//  CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
//  CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
}

uint8_t board_usb_get_serial(uint8_t serial_id[16])
{
  OCOTP_Init(OCOTP, CLOCK_GetFreq(kCLOCK_IpgClk));

  // Reads shadow registers 0x01 - 0x04 (Configuration and Manufacturing Info)
  // into 8 bit wide destination, avoiding punning.
  for (int i = 0; i < 4; ++i) {
    uint32_t wr = OCOTP_ReadFuseShadowRegister(OCOTP, i + 1);
    for (int j = 0; j < 4; j++) {
      serial_id[i*4+j] = wr & 0xff;
      wr >>= 8;
    }
  }
  OCOTP_Deinit(OCOTP);

  return 16;
}

void board_dfu_complete(void)
{
  NVIC_SystemReset();
}

bool board_app_valid(void)
{
  volatile uint32_t const * app_vector = (volatile uint32_t const*) BOARD_FLASH_APP_START;

  // 1st word is stack pointer (should be in SRAM region)

  // 2nd word is App entry point (reset)
  if (app_vector[1] < BOARD_FLASH_APP_START || app_vector[1] > BOARD_FLASH_APP_START + BOARD_FLASH_SIZE) {
    return false;
  }

  return true;
}

void board_app_jump(void)
{
  // Create the function call to the user application.
  // Static variables are needed since changed the stack pointer out from under the compiler
  // we need to ensure the values we are using are not stored on the previous stack
  static uint32_t stack_pointer;
  static uint32_t app_entry;

  uint32_t const * app_vector = (uint32_t const*) BOARD_FLASH_APP_START;
  stack_pointer = app_vector[0];
  app_entry = app_vector[1];

  // TODO protect bootloader region

  /* switch exception handlers to the application */
  SCB->VTOR = (uint32_t) BOARD_FLASH_APP_START;

  // Set stack pointer
  __set_MSP(stack_pointer);
  __set_PSP(stack_pointer);

  // Jump to Application Entry
  asm("bx %0" ::"r"(app_entry));
}

//--------------------------------------------------------------------+
// Timer
//--------------------------------------------------------------------+

void board_timer_start(uint32_t ms)
{
  // due to highspeed SystemCoreClock = 600 mhz, max interval of 24 bit systick is only 27 ms
  const uint32_t tick = (SystemCoreClock/1000) * ms;
  SysTick_Config( tick );
}

void board_timer_stop(void)
{
  SysTick->CTRL = 0;
}

void SysTick_Handler(void)
{
  board_timer_handler();
}

//--------------------------------------------------------------------+
// LED / RGB
//--------------------------------------------------------------------+

void board_led_write(uint32_t state)
{
  GPIO_PinWrite(LED_PORT, LED_PIN, state ? LED_STATE_ON : (1-LED_STATE_ON));
}

#if NEOPIXEL_NUMBER
#define MAGIC_800_INT   900000  // ~1.11 us -> 1.2  field
#define MAGIC_800_T0H  2800000  // ~0.36 us -> 0.44 field
#define MAGIC_800_T1H  1350000  // ~0.74 us -> 0.84 field

static inline uint8_t apply_percentage(uint8_t brightness)
{
  return (uint8_t) ((brightness*NEOPIXEL_BRIGHTNESS) >> 8);
}

void board_rgb_write(uint8_t const rgb[])
{
  // neopixel color order is GRB
  uint8_t const pixels[3] = { apply_percentage(rgb[1]), apply_percentage(rgb[0]), apply_percentage(rgb[2]) };
  uint32_t const numBytes = 3;

  uint8_t const *p = pixels, *end = p + numBytes;
  uint8_t pix = *p++, mask = 0x80;
  uint32_t start = 0;
  uint32_t cyc = 0;

  //assumes 800_000Hz frequency
  //Theoretical values here are 800_000 -> 1.25us, 2500000->0.4us, 1250000->0.8us
  //TODO: try to get dynamic weighting working again
  uint32_t const sys_freq = SystemCoreClock;
  uint32_t const interval = sys_freq/MAGIC_800_INT;
  uint32_t const t0       = sys_freq/MAGIC_800_T0H;
  uint32_t const t1       = sys_freq/MAGIC_800_T1H;

  __disable_irq();

  // Enable DWT in debug core. Useable when interrupts disabled, as opposed to Systick->VAL
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;

  for(;;) {
    cyc = (pix & mask) ? t1 : t0;
    start = DWT->CYCCNT;

    GPIO_PinWrite(NEOPIXEL_PORT, NEOPIXEL_PIN, 1);
    while((DWT->CYCCNT - start) < cyc);

    GPIO_PinWrite(NEOPIXEL_PORT, NEOPIXEL_PIN, 0);
    while((DWT->CYCCNT - start) < interval);

    if(!(mask >>= 1)) {
      if(p >= end) break;
      pix       = *p++;
      mask = 0x80;
    }
  }

  __enable_irq();
}

#else

void board_rgb_write(uint8_t const rgb[])
{
  (void) rgb;
}

#endif

#if 0
uint32_t board_button_read(void)
{
  // active low
  return BUTTON_STATE_ACTIVE == GPIO_PinRead(BUTTON_PORT, BUTTON_PIN);
}
#endif

//--------------------------------------------------------------------+
// UART
//--------------------------------------------------------------------+

int board_uart_write(void const * buf, int len)
{
#if defined(UART_DEV) && CFG_TUSB_DEBUG
  LPUART_WriteBlocking(UART_DEV, (uint8_t*)buf, len);
  return len;
#else
  (void) buf; (void) len;
  return 0;
#endif
}

//--------------------------------------------------------------------+
// USB Interrupt Handler
//--------------------------------------------------------------------+
void USB_OTG1_IRQHandler(void)
{
  #if CFG_TUSB_RHPORT0_MODE & OPT_MODE_HOST
    tuh_int_handler(0);
  #endif

  #if CFG_TUSB_RHPORT0_MODE & OPT_MODE_DEVICE
    tud_int_handler(0);
  #endif
}

void USB_OTG2_IRQHandler(void)
{
  #if CFG_TUSB_RHPORT1_MODE & OPT_MODE_HOST
    tuh_int_handler(1);
  #endif

  #if CFG_TUSB_RHPORT1_MODE & OPT_MODE_DEVICE
    tud_int_handler(1);
  #endif
}
