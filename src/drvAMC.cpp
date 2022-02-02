#include "drvAMC.hpp"

Driver::Driver(const char *path, uint8_t address) : m_address(address)
{

  // int sockfd, portno, n;
  //   struct sockaddr_in serv_addr;
  //   struct hostent *server;

  //   portno = 9002;
  //   sockfd = socket(AF_INET, SOCK_STREAM, 0);
  //   server = gethostbyname("192.168.127.254");
  //   bzero((char *)&serv_addr, sizeof(serv_addr));
  //   serv_addr.sin_family = AF_INET;
  //   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
  //         server->h_length);
  //   serv_addr.sin_port = htons(portno);
    
  //   if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  //       printf("ERROR connecting\n");
    
    //display("> Check connection: ");
    m_fd = open(path, O_RDWR | O_NOCTTY);
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
