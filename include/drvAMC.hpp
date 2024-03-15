#ifndef ADVANCED_MOTION_DRIVER_HPP
#define ADVANCED_MOTION_DRIVER_HPP

#include <iostream>
#include <string>

#include "serial_client.hpp"
#include "strANSIseq.hpp"
#include "tcp_client.hpp"
#include <math.h>
#include <mutex>
#include <stdexcept>

namespace AMC
{

class Driver : public Communication::TCP
{
    public:
    Driver(bool verbose = false)
        : ESC::CLI(verbose, "AMC_Driver"){

          };
    ~Driver(){};

    void _sendCmd(const char *cmd, double_t val)
    {
        if(m_is_connected)
        {
            m_buff[0] = cmd[0];
            *(double_t *)(m_buff + 2) = val;
            this->writeS(m_buff, m_pkgSize);
        }
    };

    template <typename T>
    T recvCmd()
    {
        if(m_is_connected)
        {
            size_t n = sizeof(T);
            this->readS(m_buff, n);
            return *(T *)m_buff;
        }
        return -1;
    };

    void set_current(double_t c)
    {
        _sendCmd("C", c);
        // if(recvCmd<uint8_t>() != 0xaa)
        //     logln("error", true);
    };

    void set_target_position(double_t t)
    {
        _sendCmd("T", t);
        // if(recvCmd<uint8_t>() != 0xaa)
        //     logln("error", true);
    };

    double get_position()
    {
        _sendCmd("P", 0);
        return recvCmd<double>();
    };

    void reset()
    {
        _sendCmd("R", 0);
        //wait for 2s
        usleep(2000000);
    };

    void set_Kp(double Kp) { _sendCmd("K", Kp); }

    void set_Kd(double Kd) { _sendCmd("D", Kd); }

    double set_current_get_pos(double_t c)
    {
        _sendCmd("X", c);
        return recvCmd<double_t>();
    };

    private:
    double_t m_pos_target;
    double_t m_pos;
    double_t m_current;
    uint16_t m_pkgSize = 10;
    uint8_t m_buff[6];
};

class Driver_setting_com : virtual public ESC::CLI
{
    public:
    Driver_setting_com(uint8_t address = 0x3f, int verbose = -1);
    ~Driver_setting_com();

    int writeAccess(bool active = true);
    int enableBridge(bool active = true);

    void set_p_gain(int32_t gain)
    {
        this->writeIndex<uint32_t>(0x38, 0x00, gain);
    };
    void set_i_gain(int32_t gain)
    {
        this->writeIndex<uint32_t>(0x38, 0x02, gain);
    };
    void set_d_gain(int32_t gain)
    {
        this->writeIndex<uint32_t>(0x38, 0x04, gain);
    };

    void set_pos(int32_t pos) { this->writeIndex<int32_t>(0x45, 0x00, pos); };

    int32_t get_pos() { return this->readIndex<int32_t>(0x12, 0x00); };

    template <typename T>
    T readIndex(uint8_t index, uint8_t offset)
    {
        uint8_t size = sizeof(T); //get the size of the variable to read
        _readIndex(index, offset, size);
        return *(T *)(m_buf + 8); //return value of the variable
    };

    template <typename T>
    int writeIndex(uint8_t index, uint8_t offset, T val)
    {
        uint8_t size = sizeof(T); //get the size of the variable to read
        *(T *)(m_buf + 8) = val;  // store value to write in the buffer
        _writeIndex(index, offset, size);
        return 1;
    };

    private:
    void _read_until_master_reply(uint8_t *buf, int len);

    void _readIndex(uint8_t index, uint8_t offset, uint8_t size);

    void _writeIndex(uint8_t index, uint8_t offset, uint8_t size);
    bool m_is_connected = false;
    uint8_t m_address;
    uint16_t m_crctable[256];
    uint16_t m_crc_accumulator;
    uint8_t m_buf[50];
    Communication::Client *m_client;
    std::mutex *m_mutex;
};

} // namespace AMC
#endif //ADVANCED_MOTION_DRIVER_HPP
