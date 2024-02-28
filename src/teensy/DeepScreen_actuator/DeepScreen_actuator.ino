#include <Wire.h>
#include <DFRobot_ADS1115.h>
#include <NativeEthernet.h>
#include "DeepScreen_actuator.hpp"

DeepScreen_actuator* DeepScreen_actuator::s_running_act=nullptr;//pointer for timer update static functions
DeepScreen_actuator act;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 127, 150);
EthernetServer server(5000);
const int pkgSize = 1 + 1 + 8; //data on 4 byte


void setup(void) 
{
  Serial.begin(115200);

  Ethernet.begin(mac, ip);                              // start the Ethernet connection and the server:
  if (Ethernet.hardwareStatus() == EthernetNoHardware)  // Check for Ethernet hardware present
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
  if (Ethernet.linkStatus() == LinkOFF)
    Serial.println("Ethernet cable is not connected.");

  Serial.println("Ok");
  server.begin();  // start the server
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

}

void loop(void) 
{
  byte buff[255];
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    while (client.connected())
    {
      int len = client.available();
      if (len >= pkgSize)
      {
        client.read(buff, pkgSize);
        uint8_t  index = buff[1];
        switch (buff[0])
        {
          case 'T'://set target position
            {
                act.target_position()=*(double*)(buff + 2);
                break;
            }
            case 'C'://set value (current, position, speed depending of the mode seleted)
            {
              act.current()=*(double*)(buff + 2);
              break;
            }
          case 'P':// return position
            {
              double v = act.get_pos();
              client.write((uint8_t*)(&v), 8);
              break;
            }
        }
      }
    }

    client.stop();// close the connection
    Serial.println("client disconnected");
  }
}
