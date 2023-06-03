#include "drvAMC.hpp"

namespace AMC
{
using namespace ESC;
Driver_setting_com::Driver_setting_com(uint8_t address, int verbose)
    : ESC::CLI(verbose, "Driver_setting"), m_address(address)
{
    m_mutex = new std::mutex();
    try
    {
        logln("Starting TCP Client", true);
        m_client = new Communication::TCP(verbose - 1);
        m_client->open_connection("192.168.127.254", 9002, 3);
        m_is_connected = true;
    }
    catch(std::string msg)
    {
        log_error(msg);
    }


    this->writeAccess();
    this->enableBridge();

    this->set_p_gain(10000);
    this->set_pos(-5000);
}

Driver_setting_com::~Driver_setting_com()
{
    // LOG("%s\tDisabling bridge%s",
    //     ESC::fstr("[AMC Driver]", {ESC::BOLD, ESC::FG_MAGENTA}).c_str(),
    //     ESC::fstr("...", {ESC::BLINK_SLOW}).c_str());
    this->enableBridge(false);
    this->writeAccess(false);
    // LOG("\b\b\b%s\n", ESC::fstr(" OK", {ESC::BOLD, ESC::FG_GREEN}).c_str());
    m_client->close_connection();
}

int Driver_setting_com::writeAccess(bool active)
{
    return this->writeIndex<uint16_t>(0x07, 0x00, active ? 0x000f : 0x0000);
}

int Driver_setting_com::enableBridge(bool active)
{
    return this->writeIndex<uint16_t>(0x01, 0x00, active ? 0x0000 : 0x0001);
}


void Driver_setting_com::_readIndex(uint8_t index, uint8_t offset, uint8_t size)
{
    std::lock_guard<std::mutex> lck(*m_mutex); //ensure only one thread using it
    if(m_is_connected)
    {
        m_buf[0] = 0xa5;
        m_buf[1] = m_address;
        m_buf[2] = 0x01;
        m_buf[3] = index;
        m_buf[4] = offset;
        m_buf[5] = (uint8_t)(size / 2);
        *(uint16_t *)(m_buf + 6) = m_client->CRC(m_buf, 6); //compute crc on 6 first bits

        int n = 8;
        n -= m_client->writeS(m_buf, 8); //send request
        while(n > 0)
            n -= m_client->writeS(m_buf + 8 - n, n); //ensure evtg is written
        _read_until_master_reply(m_buf, size + 10); //get the master reply

        if(m_client->check_CRC(m_buf, 6)) //check if CRCs are valid
            throw "crc1 error";
        if(m_client->check_CRC(m_buf + 8, size))
            throw "crc2 error";
    }
};

void Driver_setting_com::_writeIndex(uint8_t index,
                                     uint8_t offset,
                                     uint8_t size)
{
    std::lock_guard<std::mutex> lck(*m_mutex); //ensure only one thread using it
    if(m_is_connected)
    {
        m_buf[0] = 0xa5;
        m_buf[1] = m_address;
        m_buf[2] = 0x02;
        m_buf[3] = index;
        m_buf[4] = offset;
        m_buf[5] = (uint8_t)(size / 2);
        *(uint16_t *)(m_buf + 6) = m_client->CRC(m_buf, 6); //compute crc on 6 first bits
        *(uint16_t *)(m_buf + 8 + size) = m_client->CRC(m_buf + 8, size); //crc on the val
        int n = 8 + size + 2;

        n -= m_client->writeS(m_buf, 8 + size + 2); // send request
        while(n > 0) n -= m_client->writeS(m_buf + size + 10 - n, n);

        _read_until_master_reply(m_buf, 8);

        if(m_client->CRC(m_buf, 6) != *(uint16_t *)(m_buf + 6))
            throw "crc1 error";
    }
};

void Driver_setting_com::_read_until_master_reply(uint8_t *buf, int len)
{ //should only be used by _writeIndex or _readIndex (to ensure locking properly)
    for(int i = 0;;) //while the replay is not a master reply
    {
        m_client->readS(buf + i, 1);
        if(i == 1)
        {
            if(buf[1] == 0xff)
                break;
            else if(buf[1] != 0xa5)
                i--;
        }
        else if(i == 0 && buf[0] == 0xa5)
            i++;
    }
    int n = len - 2;                 //remaining bytes to read
    n -= m_client->readS(buf + 2, n); // read reply of the driver
    while(n > 0)
        n -= m_client->readS(buf + len - n, n); // ensure all the bytes are read
    // std::cout << "read " << std::endl;
    // for(int i = 0; i < len; i++)
    //     std::cout << std::hex << i << ":" << (int)buf[i] << " " << std::flush;
    // std::cout << std::endl;
};

} // namespace AMC
