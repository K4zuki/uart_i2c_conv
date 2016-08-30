/*
tinyI2C_1768.h
*/

static const uint8_t chip_id = ID_LPC1768;

#define isI2C1  // unused(always enabled)
I2C dev1( p9, p10 );

#define isI2C2
I2C dev2(p28, p27);
#undef isI2C3
#undef isI2C4

#define isUART
#undef isUSBUART
Serial pc(USBTX,USBRX);

#define isHWSPI
#undef isSWSPI
SPI _spi(p5, p6, p7); // mosi, miso, sclk
DigitalOut _cs(p8); // CS

#define isGPIO0 // unused(always enabled)
DigitalInOut _GPIO00(p11);
DigitalInOut _GPIO01(p12);
DigitalInOut _GPIO02(p13);
DigitalInOut _GPIO03(p14);
DigitalInOut _GPIO04(p15);
DigitalInOut _GPIO05(p16);
DigitalInOut _GPIO06(p17);
DigitalInOut _GPIO07(p18);

#define isGPIO1
DigitalInOut _GPIO10(p19);
DigitalInOut _GPIO11(p20);
DigitalInOut _GPIO12(p21);
DigitalInOut _GPIO13(p22);
DigitalInOut _GPIO14(p23);
DigitalInOut _GPIO15(p24);
DigitalInOut _GPIO16(p25);
DigitalInOut _GPIO17(p26);

