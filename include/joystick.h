#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <iostream>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include <linux/joystick.h>
#include <vector>
#include <unistd.h>

#define JOYSTICK_DEV "/dev/input/js0"
#define EVENT_DEV "/dev/input/event14"

struct joystick_position {
	float theta, r, x, y;
};

struct joystick_state {
	std::vector<signed short> button;
        __u32 btn_b;
	std::vector<signed short> axis;
};

class cJoystick {
  private:
	pthread_t thread;
	bool active;
	int joystick_fd;

    int event_fd;
    struct ff_effect effects[4];
    struct input_event play, gain;

	js_event *joystick_ev;
	joystick_state *joystick_st;
	__u32 version;
	__u8 axes;
	__u8 buttons;
        
	char name[256];

    void play_f(__u16 code, __s32 value);
protected:
  public:
	cJoystick();
	~cJoystick();
	static void* loop(void* obj);
	void readEv();
	joystick_position joystickPosition(int n);
	int joystickValue(int n);
    bool buttonPressed(int n);
    uint32_t button_bytes();
    void leftPulse();
    void leftON();
    void leftOFF();
    void rightPulse();
    void rightON();
    void rightOFF();
    void pulse();
    void rumbleON();
    void rumbleOFF();
};

#endif
