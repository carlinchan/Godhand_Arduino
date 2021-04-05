#ifndef __PWM_SERVO__
#define __PWM_SERVO__
#include <Adafruit_PWMServoDriver.h> //https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library
class PWMServoController: public Adafruit_PWMServoDriver {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  private:
    const int servo_min = 105;  // Min value of servo
    const int servo_max = 540;  // Max value of servo
    struct {
      int angle;  // The current angle of servo
      bool rev; // Used to determine whether reverse or not
      long prev_time, current_time; // Used to store the previous and current time
      bool is_new_action; // Used to determine the action is new or not
    } data[16];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
  public:
    PWMServoController(int addr): Adafruit_PWMServoDriver(addr) {
      for (int i = 0; i < 16; i++) {
        data[i].rev = false;
        data[i].angle = 0;
        data[i].prev_time = 0;
        data[i].current_time = 0;
        data[i].is_new_action = true;
      }
    }

    PWMServoController(): PWMServoController(0x40) {

    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used to start up the Adafruit_PWMServoDriver
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    void setup() {
      Adafruit_PWMServoDriver::begin();
      Adafruit_PWMServoDriver::setPWMFreq(50);
      delay(10);
      //Serial.println("begin");
    }
    
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setting the angle to the servo
// Para: index - Servo pin
// Para: angle - The angle we want to achieve
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    void write(int index, int angle) {
      setPWM(index, 0, map(angle, 0, 180, servo_min, servo_max));
      if (angle != data[index].angle)data[index].angle = angle;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// THe move action
// Para: i - The reference no. came by Godhand file. 0: Thumb; 1: Index; 2: Middle; 3: Ring; 4: Pinky
// Para: index - Servo pin
// Para: target_angle - The angle we want to achieve
// Para: angle_time_gap - The angle time gap of the servo
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    bool move(int i, int index, int target_angle, int angle_time_gap) {
      // Checking the action whether is new or not. If yes, update the prev_time first
      if (data[index].is_new_action ==  true) {
        data[index].prev_time = millis();
        data[index].is_new_action = false;
        return false;
      }
      else {
        if (data[index].angle == target_angle)  {
          //Serial.println(String("angle: ") + data[index].angle + String(", target_angle: ") + target_angle);
          Serial.println("Finger '" + String(i) + "' action finish! In pin '" + String(index) + "'");
          data[index].is_new_action = true;
          return true;
        }
        data[index].current_time = millis();
        if (data[index].current_time - data[index].prev_time >= angle_time_gap) {
          long diff_time = data[index].current_time - data[index].prev_time;
          int diff_angle = diff_time / angle_time_gap;
          data[index].prev_time = data[index].current_time;
          //Serial.println(String("angle: ") + data[index].angle + String(", target_angle: ") + target_angle + String(", diff_angle: ") + diff_angle);
          write(index, data[index].angle);
          if (target_angle < data[index].angle) {
            data[index].angle -= diff_angle;
            if (data[index].angle < 0) data[index].angle = 0;
          } else {
            data[index].angle += diff_angle;
            if (data[index].angle > 180) data[index].angle = 180;
          }
        }
        return false;
      }     
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setting "is_new_action" to true when the finger movement is finished (Including the touch is sensed)
// Para: i - The reference no. came by Godhand file. 0: Thumb; 1: Index; 2: Middle; 3: Ring; 4: Pinky
// Para: index - Servo pin
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    void set_is_new_action_true(int i, int index) {
      data[index].is_new_action = true;
//      Serial.println("Set is_new_action in finger '" + String(i) + "' to true! In pin '" + String(index) + "'");
    }   

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used for getting the value of finger angle
// Para: index - Servo pin
// Return: finger angle
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    int finger_angle(int index) {
      return data[index].angle;
    }
};
#endif
