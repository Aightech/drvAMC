#ifndef ADVANCED_MOTION_DRIVER_HPP
#define ADVANCED_MOTION_DRIVER_HPP

#include <iostream>
#include <string>

#include "com_client.hpp"
#include <mutex>
#include <stdexcept>
#include <math.h>

namespace AMC
{

class Driver : public Communication::Client
{
    public:
    Driver(bool verbose = false){

    };
    ~Driver(){};

    void
    set_current(int16_t c)
    {
        m_buff[0] = 'c';
        m_current=c;
        *(int16_t *)(m_buff + 2) = c;
        this->writeS(m_buff, m_pkgSize);
        this->readS(m_buff, 2);
    };

    uint16_t
    get_pos()
    {
        m_buff[0] = 'p';
        this->writeS(m_buff, m_pkgSize);
        this->readS((uint8_t*)&m_pos, 2);
        return *(uint16_t*)m_buff;
    };

    // uint16_t
    // set_current_get_pos(int16_t c)
    // {
    //     m_buff[0] = 'x';
    //     m_current=c;
    //     *(int16_t *)(m_buff + 2) = c;
    //     this->writeS(m_buff, m_pkgSize);
    //     this->readS((uint8_t*)(&m_pos), 2);
    //     return m_pos;
    // };

    uint16_t
    set_current_get_pos(double c)
    {
        m_buff[0] = 'x';
        c=(c<-1)?-1:(c>1)?1:c;
        m_current=c*32768;
        std::cout << "c " << m_current<< std::endl;
        *(int16_t *)(m_buff + 2) = m_current;
        this->writeS(m_buff, m_pkgSize);
        this->readS((uint8_t*)(&m_pos), 2);
        return m_pos;
    };

    void get_stat()
    {
        m_buff[0] = 'd';
        this->writeS(m_buff, m_pkgSize);
        float vals[4];
        this->readS((uint8_t*)vals, 16);

        std::cout << "mean: " << vals[0] << std::endl;
        std::cout << "std: " << sqrt(vals[1]-vals[0]*vals[0]) << std::endl;
        std::cout << "n: " << vals[2] << std::endl;
        std::cout << "max: " << vals[3] << std::endl;
        
    };

    private:
    uint16_t m_pos;
    int16_t m_current;
    uint16_t m_pkgSize=6;
    uint8_t m_buff[6];
    bool m_verbose;
};

class Driver_setting_com
{
    public:
    Driver_setting_com(uint8_t address = 0x3f, bool verbose = false);
    ~Driver_setting_com();

    int
    writeAccess(bool active = true);
    int
    enableBridge(bool active = true);

    void
    set_p_gain(int32_t gain)
    {
        this->writeIndex<uint32_t>(0x38, 0x00, gain);
    };
    void
    set_i_gain(int32_t gain)
    {
        this->writeIndex<uint32_t>(0x38, 0x02, gain);
    };
    void
    set_d_gain(int32_t gain)
    {
        this->writeIndex<uint32_t>(0x38, 0x04, gain);
    };

    void
    set_pos(int32_t pos)
    {
        this->writeIndex<int32_t>(0x45, 0x00, pos);
    };

    int32_t
    get_pos()
    {
        return this->readIndex<int32_t>(0x12, 0x00);
    };

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
    _read_until_master_reply(uint8_t *buf, int len);

    void
    _readIndex(uint8_t index, uint8_t offset, uint8_t size);

    void
    _writeIndex(uint8_t index, uint8_t offset, uint8_t size);

    /** Returns true on success, or false if there was an error */
    bool
    SetSocketBlockingEnabled(int fd, bool blocking);

    bool m_is_connected = false;
    uint8_t m_address;
    uint16_t m_crctable[256];
    uint16_t m_crc_accumulator;
    uint8_t m_buf[50];
    bool m_verbose;
    Communication::Client m_client;
    std::mutex *m_mutex;
};

} // namespace AMC
#endif //ADVANCED_MOTION_DRIVER_HPP
