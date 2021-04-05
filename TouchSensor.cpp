#include "TouchSensor.h";
#include "Arduino.h";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Contractor
// Para: thumb - The initial value of touch sensor on thumb
// Para: index - The initial value of touch sensor on index
// Para: middle - The initial value of touch sensor on middle
// Para: ring - The initial value of touch sensor on ring
// Para: pinky - The initial value of touch sensor on pinky
// data[0]:thumb; data[1]:index; data[2]:middle; data[3]:ring; data[4]:pinky
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TouchSensor::TouchSensor(int thumb, int index, int middle, int ring, int pinky) {  
  // Initial values 
  data[0].initial = thumb;
  data[1].initial = index;
  data[2].initial = middle;
  data[3].initial = ring;
  data[4].initial = pinky; 
  // The maximum time of touching (unit: millissecond)
  data[0].max_time = 500;
  data[1].max_time = 500;
  data[2].max_time = 500;
  data[3].max_time = 600;
  data[4].max_time = 700;
  // Touching threshold = initial * sensitive
  // Used to determine whether is a touch or not; Range: 0 < sensitive < 1; 0: No sensitive, 1: Absolute sensitive
  data[0].sensitive = 0.8;
  data[1].sensitive = 0.8;
  data[2].sensitive = 0.8;
  data[3].sensitive = 0.8;
  data[4].sensitive = 0.8;
  for (int i = 0; i < 5; i++) {
    data[i].accumulation = 0;
    data[i].prev_time = 0;
    data[i].current_time = 0;
    data[i].is_first_touch = true;
  }   
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used to determine whether touch or not
// Para: index - The touch sensor number (0:thumb; 1:index; 2:middle; 3:ring; 4:pinky)
// Para: current - The current value of touch sensor
// Para: pin - Touch pin
// Return: bool - True = touching; False = no toucing
// The condition of true return is that the accumulation must increment continuously.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TouchSensor::is_touch(int index, int current, int pin){
//  Serial.println("Index: " + String(index) + ", Initial: " + String(data[index].initial) + ", current touch value: " + String(current) + ", max_time: " + String(data[index].max_time));
//  Serial.println("is_first_touch: " + String(data[index].is_first_touch)); 
  data[index].current_time = millis();  
  int threshold = data[index].initial * data[index].sensitive;
  if (current < threshold) {
    // Checking this touch whether is the first or not. If yes, update the prev_time first
    if (data[index].is_first_touch == true) {
      data[index].prev_time = millis();
      data[index].is_first_touch = false;
      data[index].accumulation = 0;
      return false;
    }
    else {
//      Serial.println("current_time: " + String(data[index].current_time) + ", prev_time: " + String(data[index].prev_time));
      data[index].accumulation += (data[index].current_time - data[index].prev_time);
//      Serial.println("Finger: " + String(index) + ", accumulation: " + String(data[index].accumulation));
      if (data[index].accumulation >= data[index].max_time) {
        data[index].accumulation = 0;
        Serial.println("Finger '" + String(index) + "' is Touching! In pin '" + String(pin) + "'");
        data[index].is_first_touch = true;
        return true;
      }
    }
  }
  else {
    data[index].is_first_touch = true;
    data[index].accumulation = 0;    
  }
  data[index].prev_time = data[index].current_time;
  return false;       
}  

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setting "is_first_touch" to true when the finger movement is finished (Including the destination has arrived)
// Para: index - The reference no. came by Godhand file (0: Thumb; 1: Index; 2: Middle; 3: Ring; 4: Pinky)
// Para: pin - Touch pin
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
void TouchSensor::set_is_first_touch_true(int index, int pin) {
  data[index].is_first_touch = true;
//  Serial.println("Set is_first_touch in finger '" + String(index) + "' to true! In pin '" + String(pin) + "'");
}
