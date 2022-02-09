#ifndef ADVANCED_MOTION_DRIVER_H
#define ADVANCED_MOTION_DRIVER_H

#define ETHERNET_CONV
///#define USB_CONV

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

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdexcept>

class Driver
{
    public:
    Driver(const char *path, uint8_t address = 0x3f);
    ~Driver() { close(m_fd); };

    template <typename T>
    T
    readIndex(uint8_t index, uint8_t offset)
    {
        uint8_t size = sizeof(T); //get the size of the variable to read
        _readIndex(index, offset, size);
        return *(T *)(m_buf + 8); //return value of the variable
    };

    template <typename T>
    int
    writeIndex(uint8_t index, uint8_t offset, T val)
    {
        uint8_t size = sizeof(T); //get the size of the variable to read
        *(T *)(m_buf + 8) = val;  // store value to write in the buffer
        _writeIndex(index, offset, size);
        return 1;
    };

    int
    writeAccess(bool active = true);
    int
    enableBridge(bool active = true);

    private:
    uint16_t
    crchware(uint16_t data, uint16_t genpoly, uint16_t accum);

    void
    mk_crctable(uint16_t poly = 0x1021);

    void
    CRC_check(uint8_t data);

    uint16_t
    CRC(uint8_t *buf, int n);

    void
    printBit(int8_t val);

    void
    read_until_master_reply(uint8_t *buf, int len);

    void
    _readIndex(uint8_t index, uint8_t offset, uint8_t size);

    void
    _writeIndex(uint8_t index, uint8_t offset, uint8_t size);

    int m_fd;
    uint8_t m_address;
    uint16_t m_crctable[256];
    uint16_t m_crc_accumulator;
    uint8_t m_buf[20];
};

#endif
// std::cout << std::dec<<n << "\n";
// for(int i=0; i<n; i++)
//   std::cout << std::hex << (int)buf[i] <<  " ";
// std::cout << "\n";
