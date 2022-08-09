#include <Encoder.h>
#include <QuickPID.h>


#define NB_MOT 2

float Kp = 0.5, Ki = 2 , Kd = 0;

class Motor
{
  public:
    enum Mode {position = 0, current = 1};


    Motor(int encPin1, int encPin2, int drvPinPWM_, int drvPinEn_, int drvPinDir_): m_encoder(Encoder(encPin1, encPin2)),
      m_pid(QuickPID(&m_position, &m_output, &m_setPoint))
    {
      m_id = nb_mot;
      nb_mot++;
      m_pid.SetTunings(Kp, Ki, Kd);
      m_pid.SetMode(m_pid.Control::automatic);
      m_pid.SetOutputLimits(-3686, 3686);
      analogWriteResolution(12);
      m_drvPinPWM = drvPinPWM_;
      m_drvPinEn = drvPinEn_;
      m_drvPinDir = drvPinDir_;
      pinMode(m_drvPinPWM, OUTPUT);
      analogWrite(m_drvPinPWM, 410);
      pinMode(m_drvPinDir, OUTPUT);
      digitalWrite(m_drvPinDir, LOW);
      pinMode(m_drvPinEn, OUTPUT);
      digitalWrite(m_drvPinEn, HIGH);
      //SetAntiWindupMode();

    }

    void update()
    {
      int16_t o=0;
      if(m_mode==Motor::position)
      {
        m_position = m_encoder.read();
        noInterrupts();
        m_pid.Compute();
        interrupts();
        o = m_output;
        
      }
      else if(m_mode==Motor::current)
      {
        o = m_output;
      }

      
      if (o > 0)
      {
        digitalWrite(m_drvPinDir, LOW);   
      }
      else
      {
        o = -o;
        digitalWrite(m_drvPinDir, HIGH);
      }

      o += 410;
      o = (o > 3686) ? 3686 : o;
      analogWrite(m_drvPinPWM, o);
    }


    void set_pos(int sp)
    {
      m_setPoint = sp;

    };

    void set_current(int16_t o)
    {
      m_current = o;
    };

    long get_pos()
    {
      return m_encoder.read();
    }

    static int nb_mot;

    int m_id;

    Encoder m_encoder;
    QuickPID m_pid;
    float m_current = 0;
    float m_position = 0;
    float m_setPoint = 0;
    float m_output = 0;
    int m_drvPinPWM;
    int m_drvPinEn;
    int m_drvPinDir;
    int16_t v;
    Motor::Mode m_mode =  Motor::position;


};
