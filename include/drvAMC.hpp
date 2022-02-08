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
        if(n != 8)
            throw "Msg not fully sent";
        // std::cout << "bytes written: " << std::dec << n << "\n" << std::flush;
        // std::cout << std::dec<<n << "\n";
        // for(int i=0; i<n; i++)
        //   std::cout << std::hex << (int)buf[i] <<  " ";
        // std::cout << "\n";

        int i = 0;
        while(1)
        {
            n = read(m_fd, buf + i, 1);
            if(n == 0)
                throw "cannot receive anything";

            if(i == 1)
            {
                if(buf[1] == 0xff)
                    break;
                else
                    i--;
            }
            else if(i == 0 && buf[0] == 0xa5)
                i++;
        }
        n = size + 8;
        n -= read(m_fd, buf + 2, n); // read reply of the driver
        while(n > 0) n -= read(m_fd, buf + size + 10 - n, n);

        uint16_t crc = CRC(buf, 6);
        if(crc != *(uint16_t *)(buf + 6))
            throw "crc1 error";

        size = buf[5] * 2;
        crc = CRC(buf + 8, size);
        if(crc != *(uint16_t *)(buf + 8 + size))
            throw "crc2 error";

        if(buf[0] != 0xa5 || buf[1] != 0xff)
            throw "not from master";

        // std::cout << "bytes read: " << std::dec << n << "\n" << std::flush;
        // std::cout << std::dec << n << "\n";
        // for(int i = 0; i < size + 10; i++)
        //     std::cout << std::hex << (int)buf[i] << " ";
        // std::cout << "\n";

        return *(T *)(buf + 8); //return value of the variable
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
        int n = 8 + size + 2;
        n -= write(m_fd, buf, 8 + size + 2); // send request
        while(n > 0) n -= write(m_fd, buf + size + 10 - n, n);

	int i = 0;
        while(1)
        {
            n = read(m_fd, buf + i, 1);
            if(n == 0)
                throw "cannot receive anything";

            if(i == 1)
            {
                if(buf[1] == 0xff)
                    break;
                else
                    i--;
            }
            else if(i == 0 && buf[0] == 0xa5)
                i++;
        }
        n = 6;
        n -= read(m_fd, buf + 2, n); // read reply of the driver
        while(n > 0) n -= read(m_fd, buf + 8 - n, n);
        uint16_t crc = CRC(buf, 6);
        if(crc != *(uint16_t *)(buf + 6))
            throw "crc1 error";

        return 1; //read reply
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
