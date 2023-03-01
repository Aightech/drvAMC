
class DeepScreen_actuator
{
  public:
  enum Mode{POSITION,CURRENT};

  public:
  DeepScreen_actuator(int pinPWM = 23, int pinDIR = 22, int pinRST = 0): m_pinPWM(pinPWM), m_pinDIR(pinDIR), m_pinRST(pinRST), m_ads(&Wire)
  {
    pinMode(pinPWM, OUTPUT);
    pinMode(pinDIR, OUTPUT);
    pinMode(pinRST, OUTPUT);

    analogWriteFrequency(pinPWM, 4577.64); 
    analogWriteResolution(15);

    digitalWrite(pinRST, LOW);
    digitalWrite(pinDIR, LOW);
    analogWrite(pinPWM, 15);//min 14

    digitalWrite(pinRST, LOW);
    delay(1000);
    digitalWrite(pinRST, HIGH);
    delay(1000);
    digitalWrite(pinRST, LOW);

    m_ads.setAddr_ADS1115(ADS1115_IIC_ADDRESS1);   // 0x48
    m_ads.setGain(eGAIN_TWOTHIRDS);   // 2/3x gain
    m_ads.setMode(eMODE_SINGLE);       // single-shot mode
    m_ads.setRate(eRATE_860);          // 128SPS (default)
    m_ads.setOSMode(eOSMODE_SINGLE);   // Set to start a single-conversion
    m_ads.init();
    if(!m_ads.checkADS1115())
      Serial.println("ADS1115 Disconnected!");    
    m_pos = m_ads.readVoltage(0)-2636;
    m_pos_prev=m_pos;
    m_pos_target=m_pos;

    s_running_act = this;
    m_adcTimer.begin(act_update, 2000);
    Serial.println("start");  


  };

  void read_adc()
  {
    m_pos_prev=m_pos;
    m_pos = -(m_ads.readVoltage(0)-2637)*0.001142792;
  }

  void set_pos_target(double pos_mm)
  {
    m_pos_target = pos_mm;
  }

  void update_current()
  {

        double a_spd = 0.5;
        double a_cur = 0.5;
    switch(m_mode)
    {
      case POSITION:
        read_adc();
        m_speed = a_spd*m_speed + (1-a_spd)*(m_pos-m_pos_prev);
        m_current = a_cur*m_current + (1-a_cur)*(m_Kp*(m_pos-m_pos_target) + m_Kd*m_speed);
      break;
      case CURRENT:
        m_current = -32757/20;
      break;
    }
    Serial.println(m_pos);

    int val = abs(m_current);
    val = (val<32757)?val:32757;
    int dir = (m_current>0)?LOW:HIGH;
    analogWrite(m_pinPWM, 15+val);//add the min value =0
    digitalWrite(m_pinDIR, dir);    
  }

  static void act_update()
  {
    s_running_act->update_current();
  }

  DeepScreen_actuator::Mode& mode()
  {
    return m_mode;
  }

  double get_pos()
  {
    return m_pos;
  }

  double& current()
  {
    return m_current;     
  }

  double& target_position()
  {
    return m_pos_target;     
  }


  private:
  int m_pinPWM = 23;
  int m_pinDIR = 22;
  int m_pinRST = 0; 


  DFRobot_ADS1115 m_ads;
  double m_pos;
  double m_pos_target;
  double m_pos_prev;
  double m_speed=0;  
  double m_current=0;
  double m_Kp = 30;
  double m_Kd = 200;
  

  DeepScreen_actuator::Mode m_mode=Mode::CURRENT;

  IntervalTimer m_adcTimer;
  static DeepScreen_actuator* s_running_act;

};