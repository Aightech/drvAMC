//class used to unify the different functions used to interact with the EMG modules
class DrvAMC
{
  public:
    DrvAMC()
    {
      pinMode(m_PWM_pin, OUTPUT);
      pinMode(m_DIR_pin, OUTPUT);
      analogWriteResolution(15);
      analogWrite(m_PWM_pin, 15);
    };
    ~DrvAMC() {};

    void begin()
    {

    };

    void set_current(int16_t val)//[-32768; 32768]
    {
      m_current = val;
      if (val > 15)
      {
        analogWrite(m_PWM_pin, m_current);
        digitalWrite(m_DIR_pin, HIGH);
      }
      else if(val < -15)
      {
        analogWrite(m_PWM_pin, -m_current);
        digitalWrite(m_DIR_pin, LOW);
      }
      else
      {
        analogWrite(m_PWM_pin, 15);
        digitalWrite(m_DIR_pin, (val>0)?HIGH:LOW);
      }
    }

    uint16_t* get_pos_ptr()
    {
      m_pos = analogRead(m_POS_pin);
      return &m_pos;
    }



  private:
    int m_PWM_pin=5;
    int m_DIR_pin=6;
    int m_POS_pin=0;

    int16_t m_current;
    uint16_t m_pos;


};
