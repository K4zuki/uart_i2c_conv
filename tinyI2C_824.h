/*
tinyI2C_824.h
*/

static const uint8_t chip_id = ID_LPC824;
#define isI2C1  // unused(always enabled)
I2C dev1(I2C_SDA, I2C_SCL);//11,10 hard coded, 220 ohm pull-up

#define isI2C2
#define isI2C3
#define isI2C4

#define isUART
#undef isUSBUART

#define isHWSPI
#undef isSWSPI

#define isGPIO0 // unused(always enabled)
#undef isGPIO1

#if defined(TARGET_SSCI824)
    Serial pc(P0_4, P0_0);
    I2C dev2(P0_16, P0_27);
    I2C dev3(P0_26, P0_25);
    I2C dev4(P0_24, P0_15);
    SPI _spi(P0_6, P0_7, P0_13); // mosi, miso, sclk
    DigitalOut _cs(P0_1); // CS
    DigitalInOut _GPIO00(P0_17);
    DigitalInOut _GPIO01(P0_18);
    DigitalInOut _GPIO02(P0_19);
    DigitalInOut _GPIO03(P0_20);
    DigitalInOut _GPIO04(P0_21);
    DigitalInOut _GPIO05(P0_22);
    DigitalInOut _GPIO06(P0_23);
    DigitalInOut _GPIO07(P0_14);
#elif defined(TARGET_LPC824)
    Serial pc(USBTX,USBRX); // P0_7, P0_18
    I2C dev2(P0_6, P0_14); // 6,14 | A0, A1
    I2C dev3(P0_23, P0_22); // 23,22 | A2, A3
    I2C dev4(P0_21, P0_20); // 21,20 | A4, A5
    SPI _spi(P0_26,P0_25,P0_24); // mosi, miso, sclk, D11, D12, D13
    DigitalOut _cs(P0_15); // CS, D10
    DigitalInOut _GPIO00(P0_19); // D2
    DigitalInOut _GPIO01(P0_12); // D3
    DigitalInOut _GPIO02(P0_18); // D4
    DigitalInOut _GPIO03(P0_28); // D5
    DigitalInOut _GPIO04(P0_16); // D6
    DigitalInOut _GPIO05(P0_17); // D7
    DigitalInOut _GPIO06(P0_13); // D8
    DigitalInOut _GPIO07(P0_27); // D9
#endif

