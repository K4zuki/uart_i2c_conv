/*
tinyI2C_11U3x.h
*/

static const uint8_t chip_id = ID_LPC11UXX;

#define isI2C1  // unused(always enabled)
I2C dev1( P0_5, P0_4 );


#undef isI2C2
#undef isI2C3
#undef isI2C4

#undef isUART
#define isUSBUART

#include "USBSerial.h"
USBSerial pc;

#define isHWSPI
#undef isSWSPI
SPI _spi(P0_9, P0_8, P0_10); // mosi, miso, sclk
DigitalOut _cs(P0_7); // CS

#define isGPIO0 // unused(always enabled)
DigitalInOut _GPIO00(P0_22);
DigitalInOut _GPIO01(P0_11);
DigitalInOut _GPIO02(P0_12);
DigitalInOut _GPIO03(P0_13);
DigitalInOut _GPIO04(P0_14);
DigitalInOut _GPIO05(P0_15);
DigitalInOut _GPIO06(P0_16);
DigitalInOut _GPIO07(P0_23);

#define isGPIO1
DigitalInOut _GPIO10(P1_15);
DigitalInOut _GPIO11(P0_17);
DigitalInOut _GPIO12(P0_18);
DigitalInOut _GPIO13(P0_19);
DigitalInOut _GPIO14(P1_19);
DigitalInOut _GPIO15(P0_20);
DigitalInOut _GPIO16(P0_2 );
DigitalInOut _GPIO17(P0_21);

