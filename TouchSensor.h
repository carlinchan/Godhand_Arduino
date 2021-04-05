#ifndef TouchSensor_h
#define TouchSensor_h
#include "Arduino.h"

// TouchSensor class
class TouchSensor {  
  private:
    struct {
      int initial; // The initial value of touch sensor
      int accumulation; // The accumulative time of touching
      int max_time; // The maximum time of touching (unit: millissecond)
      long prev_time, current_time; // Used to store the current time and previous time
      bool is_first_touch;  // Used to determine whether the touch is first touch or not
      float sensitive;  // Used to determine whether is a touch or not; Range: 0 < sensitive < 1; 0: No sensitive, 1: Absolute sensitive
    }data[5]; // The struct of touch sensor (0:thumb; 1:index; 2:middle; 3:ring; 4:pinky)
         
  public:
    TouchSensor(int thumb, int index, int middle, int ring, int pinky); // Contractor
    bool is_touch(int index, int current, int pin);  // Used to determine whether touch or not
    void set_is_first_touch_true(int index, int pin);  // Used to set "is_first_touch" to true
};
#endif
