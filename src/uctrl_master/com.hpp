#ifdef TEENSY_4 // only available on teensy 4
#include <NativeEthernet.h>
#endif

#ifdef IOT33
#include <WiFiNINA.h>
#include <SPI.h>
#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
#endif

#ifdef UDP_MODE
#include <WiFiUdp.h>
#endif

// Simple class managing comunication interfaces
class Com
{
  public:
    Com()
    {
      mk_crctable();
    };

    // Begin the comunication with the available interfaces
    // the arguments are
    // - b : the baud of the serial comunication
    // - i3-i0 : are the bytes of the IP address
    // - port : is the port listened by the server
    int begin(int b = 9600, int port = -1, uint8_t i3 = 0, uint8_t i2 = 0, uint8_t i1 = 0, uint8_t i0 = 0)
    {

      if (port != -1)
      {
#ifdef TEENSY_4 // only available on teensy 4
        byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
        Ethernet.begin(mac, IPAddress(i3, i2, i1, i0));
        m_server = new EthernetServer(port);
        m_server->begin();// start the server
#endif
#ifdef IOT33 // only available on nano33 iot
        if (WiFi.status() == WL_NO_MODULE) {// check for the WiFi module:
          Serial.println("Communication with WiFi module failed!");
          while (true);
        }
        if (WiFi.firmwareVersion() < WIFI_FIRMWARE_LATEST_VERSION)
          Serial.println("Please upgrade the firmware");
        WiFi.config(IPAddress(i3, i2, i1, i0));
        if (WiFi.beginAP(ssid, pass) != WL_AP_LISTENING) {
          Serial.println("Creating access point failed");
          while (true);
        }
#ifdef UDP_MODE
        m_server = new WiFiUDP();
        m_server->begin(port);// start the server
#else
        m_server = new WiFiServer(port);
        m_server->begin();// start the server
#endif
        m_isSerial = false;
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Port: ");
        Serial.println(port);
#endif
      }
      else
      {
        m_isSerial = true;
        Serial.begin(b);
      }

      return 0;
    }

    // Return the number of byte available in the comunication buffer of the ethernet socket if available else Serial socket.
    int available()
    {
      if(m_isSerial)
        return Serial.available();

#if defined(TEENSY_4) || defined(IOT33) // only available on teensy 4 or IOT33
#ifdef UDP_MODE
      int n= m_server->parsePacket();
      if(n && m_port==0)
      {
        m_address = m_server->remoteIP();
        m_port = m_server->remotePort();
      };
      return n;
#else
      if (!m_isSocket)
      {
        m_client = m_server->available();
        if (m_client)
        {
          m_isSocket = true;
        }
      }
      if (m_isSocket)
      {
        if (!m_client.connected())
        {
          m_client.stop();
          m_isSocket = false;
          
        }
        else
        {
          return m_client.available();;
        }

      }
#endif
#endif
        return 0;
        
    }

    // Read n bytes the Ethernet socket if available else the Serial socket and store it in buff.
    // If the has_crc arg is true, the two last bytes are checked as CRC16
    // Return the number of bytes successfully read.
    size_t read(uint8_t* buff, size_t n, bool has_crc = false)
    {
      int n_r = 0;
      if (m_isSerial)
        n_r = Serial.readBytes((char*)buff, n);
#if defined(TEENSY_4) || defined(IOT33) // only available on teensy 4 or IOT33
#ifndef UDP_MODE
      else if (m_isSocket)
        n_r = m_client.read(buff, n);
#else
      n_r = m_server->read(buff, n);

#endif
#endif
      if (n_r != n)
        return n_r;
      if (has_crc && CRC(buff, n - 2) != *(uint16_t *)(buff + n - 2))
        return -1;
      return n_r;

    }

    // Write n byte from buff on the Ethernet socket if available else the Serial socket.
    // If the add_crc arg is true, two more CRC16 bytes are written after the n bytes from buff.
    // Return the number of bytes successfully written (crc16  included).
    size_t write(uint8_t* buff, size_t n, bool add_crc = false)
    {
      if (add_crc)
      {
        m_crc = CRC(buff, n);
        memcpy(buff + n, (uint8_t*)&m_crc, 2);
      }
      if (m_isSerial)
        return Serial.write((char*)buff, n + 2 * add_crc);
#if defined(TEENSY_4) || defined(IOT33) // only available on teensy 4 or IOT33
#ifdef UDP_MODE
      //Serial.println(m_address);
      //Serial.println(m_port);
      
      m_server->beginPacket(m_address, m_port); 
      int s = m_server->write(buff, n + 2 * add_crc);
      m_server->endPacket();
      return s;
#else
      else if (m_isSocket)
        return m_client.write(buff, n + 2 * add_crc);
#endif
#endif
      return 0;
    }


    // Compute and return the CRC over the n first bytes of buf
    uint16_t CRC(uint8_t *buf, int n)
    {
      m_crc_accumulator = 0;
      for (int i = 0; i < n; i++) CRC_check(buf[i]);
      return (m_crc_accumulator >> 8) | (m_crc_accumulator << 8);
    }

  private :
    // Create a CRC lookup table to compute CRC16 fatser.
    // poly represent the coeficients use for the polynome of the CRC
    void mk_crctable(uint16_t poly = 0x1021)
    {
      for (int i = 0; i < 256; i++) m_crctable[i] = crchware(i, poly, 0);
    }

    // Generate the values for the CRC lookup table.
    uint16_t crchware(uint16_t data, uint16_t genpoly, uint16_t accum)
    {
      static int i;
      data <<= 8;
      for (i = 8; i > 0; i--)
      {
        if ((data ^ accum) & 0x8000)
          accum = (accum << 1) ^ genpoly;
        else
          accum <<= 1;
        data <<= 1;
      }
      return accum;
    }

    // Function use in CRC computation
    void CRC_check(uint8_t data)
    {
      m_crc_accumulator = (m_crc_accumulator << 8) ^ m_crctable[(m_crc_accumulator >> 8) ^ data];
    };

  private:
#ifdef TEENSY_4 // only available on teensy 4
    EthernetClient m_client;
    EthernetServer* m_server;
#endif
#ifdef IOT33 // only available on iot33

#ifdef UDP_MODE
    WiFiUDP* m_server;
    IPAddress m_address;
    int m_port=0;
#else
    WiFiClient m_client;
    WiFiServer* m_server;
#endif

#endif
    bool m_isSocket = false;
    bool m_isSerial = false;

    uint16_t m_crctable[256];
    uint16_t m_crc_accumulator;
    uint16_t m_crc;

};
