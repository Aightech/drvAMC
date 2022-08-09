//class used to unify the different functions used to interact with the EMG modules
class DrvAMC
{
  public:
    DrvAMC()
    {
      pinMode(m_PWM_pin, OUTPUT);
      pinMode(m_DIR_pin, OUTPUT);
      analogWriteResolution(15);
    };
    ~DrvAMC() {};

    void begin()
    {

    };

    void set_current(int16_t val)//[-32768; 32768]
    {
      m_current = val;
      if (val > 0)
      {
        analogWrite(m_PWM_pin, m_current);
        digitalWrite(m_DIR_pin, HIGH);
      }
      else
      {
        analogWrite(m_PWM_pin, -m_current);
        digitalWrite(m_DIR_pin, LOW);
      }
    }

    uint16_t* get_pos_ptr()
    {
      m_pos = analogRead(m_POS_pin);
      return &m_pos;
    }



  private:
    int m_PWM_pin;
    int m_DIR_pin;
    int m_POS_pin;

    int16_t m_current;
    uint16_t m_pos;


};
