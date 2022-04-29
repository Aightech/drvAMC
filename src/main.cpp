#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>

#include "drvAMC.hpp"

#define PI 3.14159265


int
main(int argc, char **argv)
{

  AMC::Driver drv = AMC::Driver();
    drv.writeAccess();
    drv.enableBridge();

    drv.writeIndex<uint32_t>(0x38, 0x00, 10000);
    drv.writeIndex<int32_t>(0x45, 0x00, -5000);


    drv.enableBridge(false);
    drv.writeAccess(false);
    return 0;
}
