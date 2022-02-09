#include "drvAMC.hpp"
#include <sys/ioctl.h>
#include <sys/time.h>

Driver::Driver(uint8_t address) : m_address(address)
{

#ifdef ETHERNET_CONV
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = 9002;
    m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    server = gethostbyname("192.168.127.254");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    SetSocketBlockingEnabled(m_fd, false);
    std::cout << "[INFO] Try to connect..." << std::flush;
    connect(m_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(m_fd, &fdset);

    if(select(m_fd + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if(so_error == 0)
        {
            std::cout << "OK" << std::endl;
            m_is_connected = true;
        }
    }
    else
        std::cout << "Failed" << std::endl;

    SetSocketBlockingEnabled(m_fd, true);

#endif

#ifdef USB_CONV
    //display("> Check connection: ");
    m_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
    if(m_fd < 0)
        exit(-1);
    //display("OK\n");

    struct termios tty;
    if(tcgetattr(m_fd, &tty) != 0)
        exit(-1);

    tty.c_cflag &= ~PARENB;  // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;  // Clear stop field, only 1 stop bit (most common)
    tty.c_cflag &= ~CSIZE;   // Clear all bits that set the data size
    tty.c_cflag |= CS8;      // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow ctrl (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrlline(CLOCAL = 1)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;   // Disable echo
    tty.c_lflag &= ~ECHOE;  // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG;   // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                     ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent spe. interp. of out bytes (newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conv of newline to car. ret/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 10; // Wait for up to 1s, ret when any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if(tcsetattr(m_fd, TCSANOW, &tty) != 0)
        exit(-1);
#endif

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
        read(m_fd, buf + i, 1);
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
    int n = len - 2;             //remaining bytes to read
    n -= read(m_fd, buf + 2, n); // read reply of the driver
    while(n > 0)
        n -= read(m_fd, buf + len - n, n); // ensure all the bytes are read
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
        n -= write(m_fd, m_buf, 8); //send request
        while(n > 0)
            n -= write(m_fd, m_buf + 8 - n, n);    //ensure evtg is written
        read_until_master_reply(m_buf, size + 10); //get the master reply

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

        n -= write(m_fd, m_buf, 8 + size + 2); // send request
        while(n > 0) n -= write(m_fd, m_buf + size + 10 - n, n);

        read_until_master_reply(m_buf, 8);

        if(CRC(m_buf, 6) != *(uint16_t *)(m_buf + 6))
            throw "crc1 error";
    }
};

/** Returns true on success, or false if there was an error */
bool
Driver::SetSocketBlockingEnabled(int fd, bool blocking)
{
    if(fd < 0)
        return false;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}
