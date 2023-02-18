typedef int  WDOG_Type;
typedef int wdog_config_t;
typedef int flexspi_nor_driver_interface_t;
typedef int rtwdog_driver_interface_t;
typedef struct
{
    void (*WDOG_GetDefaultConfig)(wdog_config_t *config);
    void (*WDOG_Init)(WDOG_Type *base, const wdog_config_t *config);
    void (*WDOG_Deinit)(WDOG_Type *base);
    void (*WDOG_Enable)(WDOG_Type *base);
    void (*WDOG_Disable)(WDOG_Type *base);
    void (*WDOG_EnableInterrupts)(WDOG_Type *base, uint16_t mask);
    uint16_t (*WDOG_GetStatusFlags)(WDOG_Type *base);
    void (*WDOG_ClearInterruptStatus)(WDOG_Type *base, uint16_t mask);
    void (*WDOG_SetTimeoutValue)(WDOG_Type *base, uint16_t timeoutCount);
    void (*WDOG_SetInterrputTimeoutValue)(WDOG_Type *base, uint16_t timeoutCount);
    void (*WDOG_DisablePowerDownEnable)(WDOG_Type *base);
    void (*WDOG_Refresh)(WDOG_Type *base);
} wdog_driver_interface_t;

typedef struct
{
    const uint32_t version;  //!< Bootloader version number
    const char *copyright;   //!< Bootloader Copyright
    void (*runBootloader)(uint32_t *); //!< Function to start the bootloader executing
    const uint32_t *reserved0; //!< Reserved
    const flexspi_nor_driver_interface_t *flexSpiNorDriver; //!< FlexSPI NOR Flash API
    const uint32_t *reserved1; //!< Reserved
    const rtwdog_driver_interface_t *rtwdogDriver;
    const wdog_driver_interface_t *wdogDriver;
    const uint32_t *reserved2;
} bootloader_api_entry_t;
#define g_bootloaderTree (*(bootloader_api_entry_t**)(0x0020001c))

void setup()
{
    Serial.begin(115200);
    while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
    Serial.println("Hello World!");
    Serial.println("Attempting to start bootloader");
    uint32_t arg = (0xEB<<24) | (1<<20) | (1<<16) | (0<<0);
    g_bootloaderTree->runBootloader(&arg);
    Serial.println("failed to start bootloader");
}

void loop()
{

}
extern "C" void usb_isr(void);
extern "C" void usb_isr(void)
{

}