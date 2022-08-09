#define TEENSY_4
//#define IOT33
//#define UDP_MODE
#define VERSION 0x0200
#include "drvAMC.hpp"
#include "com.hpp"


#define DEBUG(x) Serial.print(x)
#define DEBUGF(x,y) Serial.print(x,y)
#define DEBUGLN(x) Serial.println(x)


byte buff[255];
int pkgSize = 6;
byte vals[255] = {0x00, 0x00, 0x00, 0x00};
int i, nb = 0;

uint16_t arr[2000]={0};

DrvAMC drvAMC;
Com com_interface;

bool first_dt = true;
int32_t dt;
int32_t last_t;
uint32_t mu[4];//mu,mup,n

void compute_dt_stat()
{
  if (first_dt)
  {
    mu[0] = 0;
    mu[1] = 0;
    mu[2] = 0;
    mu[3] = 0;
    first_dt = false;
    last_t = micros();
  }
  else
  {
    dt = micros() - last_t;
    mu[0] = (mu[0] * mu[2] + dt) / (mu[2] + 1);
    mu[1] = (mu[1] * mu[2] + dt * dt) / (mu[2] + 1);
    mu[2]++;
    last_t = micros();
  }
}

void setup()
{
  for(int i =0;i<1000;i++)
    arr[i]=0;
  Serial.begin(9600);
  //while (!Serial) {}
  Serial.println("OK");
  com_interface.begin(9600, 5000, 192, 168, 127, 150);
  Serial.println("OKi");

  drvAMC.begin();

}


void loop()
{
  int nn = com_interface.available();
  if (nn >= pkgSize)
  {
    compute_dt_stat();
    com_interface.read(buff, pkgSize);
    switch (buff[0])
    {
      case 'c':// set current > 'c' | id | val0 | val1
        {
          drvAMC.set_current(*(int16_t*)(buff + 2));
          break;
        }
      case 'p':// get current > 'p'
        {
          com_interface.write((uint8_t*)drvAMC.get_pos_ptr(), 2);
          break;
        }
      case 'd':// get current > 'p'
        {
          com_interface.write((uint8_t*)mu, 12);
          first_dt = true;
          break;
        }
    }
  }


}
