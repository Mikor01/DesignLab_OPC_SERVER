#ifndef __EXPANDER_REG_MAP_H__
#define __EXPANDER_REG_MAP_H__

#include "expander.h"


#if !(EXPANDER_SEQUENTIAL_BANK)

    #define IODIR_A         0x00
    #define IPOL_A          0x01
    #define GPINTEN_A       0x02
    #define DEFVAL_A        0x03
    #define INTCON_A        0x04
    #define IOCON           0x05
    #define GPPU_A          0x06
    #define INTF_A          0x07
    #define INTCAP_A        0x08
    #define GPIO_A          0x09
    #define OLAT_A          0x0A


    #define IODIR_B         0x10
    #define IPOL_B          0x11
    #define GPINTEN_B       0x12
    #define DEFVAL_B        0x13
    #define INTCON_B        0x14
    #define IOCON           0x15
    #define GPPU_B          0x16
    #define INTF_B          0x17
    #define INTCAP_B        0x18
    #define GPIO_B          0x19
    #define OLAT_B          0x1A

#else

    #define IODIR           0x00
    #define IPOL            0x02
    #define GPINTEN         0x04
    #define DEFVAL          0x06
    #define INTCON          0x08
    #define IOCON           0x0A
    #define GPPU            0x0C
    #define INTF            0x0E
    #define INTCAP          0x10
    #define GPIO            0x12
    #define OLAT            0x14

    #define IODIR_A         0x00
    #define IPOL_A          0x02
    #define GPINTEN_A       0x04
    #define DEFVAL_A        0x06
    #define INTCON_A        0x08
    #define IOCON           0x0A
    #define GPPU_A          0x0C
    #define INTF_A          0x0E
    #define INTCAP_A        0x10
    #define GPIO_A          0x12
    #define OLAT_A          0x14


    #define IODIR_B         0x01
    #define IPOL_B          0x03
    #define GPINTEN_B       0x05
    #define DEFVAL_B        0x07
    #define INTCON_B        0x09
    #define IOCON           0x0B
    #define GPPU_B          0x0D
    #define INTF_B          0x0F
    #define INTCAP_B        0x11
    #define GPIO_B          0x13
    #define OLAT_B          0x15

#endif

#endif