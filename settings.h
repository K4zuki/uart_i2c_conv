/*
settings.h
*/

// #define isI2C1  // unused(always enabled)
// #define isGPIO0 // unused(always enabled)

enum chipID_e {
    ID_LPC824 = '0',
    ID_LPC1768 = '1',
    ID_LPC11UXX = '2',
};

#if defined(TARGET_SSCI824) || defined(TARGET_LPC824)
#warning "tinyI2C_824.h"
#include "tinyI2C_824.h"

#elif defined(TARGET_LPC11U35_501) || defined(TARGET_LPC11U35_401)

#warning "import USBDevice library"
#include "tinyI2C_11U3x.h"

#elif defined(TARGET_LPC1768)

#warning "tinyI2C_1768.h"
#include "tinyI2C_1768.h"

#endif
