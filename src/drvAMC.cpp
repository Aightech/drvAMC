#include "drvAMC.hpp"

namespace AMC
{
Driver::Driver(uint8_t address) : m_address(address)
{
    m_client.open_connection("192.168.127.254", 9002);
    mk_crctable();
}

void
Driver::mk_crctable(uint16_t poly)
{
    for(int i = 0; i < 256; i++) m_crctable[i] = crchware(i, poly, 0);
}

uint16_t
Driver::crchware(uint16_t data, uint16_t genpoly, uint16_t accum)
{
    static int i;
    data <<= 8;
    for(i = 8; i > 0; i--)
    {
        if((data ^ accum) & 0x8000)
            accum = (accum << 1) ^ genpoly;
        else
            accum <<= 1;
        data <<= 1;
    }
    return accum;
}

void
Driver::CRC_check(uint8_t data)
{
    m_crc_accumulator =
        (m_crc_accumulator << 8) ^ m_crctable[(m_crc_accumulator >> 8) ^ data];
};

uint16_t
Driver::CRC(uint8_t *buf, int n)
{
    m_crc_accumulator = 0;
    for(int i = 0; i < n; i++) CRC_check(buf[i]);
    return (m_crc_accumulator >> 8) | (m_crc_accumulator << 8);
}

int
Driver::writeAccess(bool active)
{
    return this->writeIndex<uint16_t>(0x07, 0x00, active ? 0x000f : 0x0000);
}

int
Driver::enableBridge(bool active)
{
    return this->writeIndex<uint16_t>(0x01, 0x00, active ? 0x0000 : 0x0001);
}

void
Driver::printBit(int8_t val)
{
    std::cout << " ";
    for(int i = 0; i < 8; i++)
        if(1 & (val >> i))
            std::cout << "[" + std::to_string(i) + "] ";
}

void
Driver::read_until_master_reply(uint8_t *buf, int len)
{
    for(int i = 0;;) //while the replay is not a master reply
    {
        m_client.readS(buf + i, 1);
        //std::cout<< std::hex << i<< ":" <<(int)buf[i] << " " << std::flush;
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
    //std::cout<< "\n" << std::flush;
    int n = len - 2;                 //remaining bytes to read
    n -= m_client.readS(buf + 2, n); // read reply of the driver
    while(n > 0)
        n -= m_client.readS(buf + len - n, n); // ensure all the bytes are read
}

void
Driver::_readIndex(uint8_t index, uint8_t offset, uint8_t size)
{
    if(m_is_connected)
    {
        m_buf[0] = 0xa5;
        m_buf[1] = m_address;
        m_buf[2] = 0x01;
        m_buf[3] = index;
        m_buf[4] = offset;
        m_buf[5] = (uint8_t)(size / 2);
        *(uint16_t *)(m_buf + 6) = CRC(m_buf, 6); //compute crc on 6 first bits

        int n = 8;
        n -= m_client.writeS(m_buf, 8); //send request
        while(n > 0)
            n -= m_client.writeS(m_buf + 8 - n, n); //ensure evtg is written
        read_until_master_reply(m_buf, size + 10);  //get the master reply

        if(CRC(m_buf, 6) != *(uint16_t *)(m_buf + 6)) //check if CRCs are valid
            throw "crc1 error";
        if(CRC(m_buf + 8, size) != *(uint16_t *)(m_buf + 8 + size))
            throw "crc2 error";
    }
};

void
Driver::_writeIndex(uint8_t index, uint8_t offset, uint8_t size)
{
    if(m_is_connected)
    {
        m_buf[0] = 0xa5;
        m_buf[1] = m_address;
        m_buf[2] = 0x02;
        m_buf[3] = index;
        m_buf[4] = offset;
        m_buf[5] = (uint8_t)(size / 2);
        *(uint16_t *)(m_buf + 6) = CRC(m_buf, 6); //compute crc on 6 first bits
        *(uint16_t *)(m_buf + 8 + size) = CRC(m_buf + 8, size); //crc on the val
        int n = 8 + size + 2;

        n -= m_client.writeS(m_buf, 8 + size + 2); // send request
        while(n > 0) n -= m_client.writeS(m_buf + size + 10 - n, n);

        read_until_master_reply(m_buf, 8);

        if(CRC(m_buf, 6) != *(uint16_t *)(m_buf + 6))
            throw "crc1 error";
    }
};


} // namespace AMC
