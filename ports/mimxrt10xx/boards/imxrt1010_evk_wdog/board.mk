MCU = MIMXRT1011
CFLAGS += -DCPU_MIMXRT1011DAE5A

# For flash-jlink target
JLINK_DEVICE = MIMXRT1011DAE5A

# For flash-pyocd target
PYOCD_TARGET = mimxrt1010

SRC_C += \
   $(SDK_DIR)/drivers/wdog01/fsl_wdog.c \
   $(SDK_DIR)/drivers/rtwdog/fsl_rtwdog.c \
   $(BOARD_DIR)/custom.c \
   $(BOARD_DIR)/wdog_10xx.c

# flash using pyocd
flash: flash-pyocd-bin
erase: erase-pyocd
