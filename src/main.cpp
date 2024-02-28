#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>

#include "drvAMC.hpp"

#define PI 3.14159265

int main(int argc, char **argv)
{

    // AMC::Driver_setting_com drv = AMC::Driver_setting_com();
    //   drv.writeAccess();
    //   drv.enableBridge();

    //   drv.writeIndex<uint32_t>(0x38, 0x00, 10000);
    //   drv.writeIndex<int32_t>(0x45, 0x00, -5000);

    //   drv.enableBridge(false);
    //   drv.writeAccess(false);

    AMC::Driver drv(1);
    drv.open_connection("192.168.127.150", 5000, 1);
    double cur = atof(argv[1]);

    for(int i = 0; i < 5000; i++)
    {
        double pos = drv.set_current_get_pos(cur);
        std::cout << cur << " " << pos << std::endl;
    }
    return 0;
}
