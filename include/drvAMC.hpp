#ifndef ADVANCED_MOTION_DRIVER_H
#define ADVANCED_MOTION_DRIVER_H

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// Linux headers
#include <errno.h>   // Error integer and strerror() function
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#include <stdio.h>

class Driver
{
    public:
    Driver(const char *path, uint8_t address = 0x3f);
    ~Driver() { close(m_fd); };

    uint16_t
    crchware(uint16_t data, uint16_t genpoly, uint16_t accum);

    void
    mk_crctable(uint16_t poly = 0x1021);

    void
    CRC_check(uint8_t data);

    uint16_t
    CRC(uint8_t *buf, int n);

    void
    printBit(int8_t val)
    {
        std::cout << " ";
        for(int i = 0; i < 8; i++)
            if(1 & (val >> i))
                std::cout << "[" + std::to_string(i) + "] ";
    }

    template <typename T>
    T
    readIndex(uint8_t index, uint8_t offset)
    {
        uint8_t size = sizeof(T); //get the size of the variable to read
        uint8_t buf[8 + size + 2] = {0xa5,  m_address, 0x01,
                                     index, offset,    (uint8_t)(size / 2)};
        *(uint16_t *)(buf + 6) = CRC(buf, 6); //compute crc on 6 first bits
        int n = write(m_fd, buf, 8);          //send request
        n = read(m_fd, buf, 8);                   // read the echo of the request
	n = read(m_fd, buf, 8 + size + 2);    // read reply of the driver
        return *(T *)(buf + 8);               //return value of the variable
    };

    template <typename T>
    int
    writeIndex(uint8_t index, uint8_t offset, T val)
    {
        uint8_t size = sizeof(T); //get the size of the variable to read
        uint8_t buf[8 + size + 2] = {0xa5,  m_address, 0x02,
                                     index, offset,    (uint8_t)(size / 2)};
        *(uint16_t *)(buf + 6) = CRC(buf, 6); //compute crc on 6 first bits
        *(T *)(buf + 8) = val; // store value to write in the buffer
        *(uint16_t *)(buf + 8 + size) = CRC(buf + 8, size); //crc on the val
        int n = write(m_fd, buf, 8 + size + 2);             // send request
        read(m_fd, buf, n);        //read echo of the request
        return read(m_fd, buf, 8); //read reply
    };

    int
    writeAccess(bool active = true);
    int
    enableBridge(bool active = true);

    private:
    int m_fd;
    uint8_t m_address;
    uint16_t m_crctable[256];
    uint16_t m_crc_accumulator;
};

#endif
// std::cout << std::dec<<n << "\n";
// for(int i=0; i<n; i++)
//   std::cout << std::hex << (int)buf[i] <<  " ";
// std::cout << "\n";