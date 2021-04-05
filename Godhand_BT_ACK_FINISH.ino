///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name: Chan Car Lin
// UID: 3035604568
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a BluetoothSerial object
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a PWMServoController object
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "PWMServoController.hpp"

//PWMServoController pwmServo(0x41);
PWMServoController pwmServo;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a TaskHandle_t objects for multi-core programming (Bluetooth communication)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TaskHandle_t task_bluetooth;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Finger pins
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int servo_thumb = 12; // The pin for servo in thumb
int servo_index = 9;  // The pin for servo in index finger
int servo_middle = 6; // The pin for servo in middle finger
int servo_ring = 3; // The pin for servo in ring finger
int servo_pinky = 0; // The pin for servo in pinky finger
int servo_list[5] = {servo_thumb, servo_index, servo_middle, servo_ring, servo_pinky};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Angle time gap for each finger
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int atg_thumb = 30; // The angle time gap for thumb
int atg_index = 28; // The angle time gap for index finger
int atg_middle = 28;  // The angle time gap for middle finger
int atg_ring = 26;  // The angle time gap for ring finger
int atg_pinky = 22; // The angle time gap for pinky finger
int atg_list[5] = {atg_thumb, atg_index, atg_middle, atg_ring, atg_pinky};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Angle parameters
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int relax = 60;  // The angle of relaxed finger
int thumb_fine_tune = 10; // The fine tune value of thumb, used in thumb_index_middle()
int straight_default = 10; // The default angle for straight fingers
int straight_thumb = 10;  // The angle of straight thumb
int straight_index = 10;  // The angle of straight index
int straight_middle = 10;  // The angle of straight middle
int straight_ring = 20;  // The angle of straight ring
int straight_pinky = 20;  // The angle of straight pinky
int straight_list[5] = {straight_thumb, straight_index, straight_middle, straight_ring, straight_pinky};
int clenched_default = 150; // The default angle for clenched fingers
int clenched_thumb = 150;  // The angle of clenched thumb
int clenched_index = 150;  // The angle of clenched index
int clenched_middle = 150;  // The angle of clenched middle
int clenched_ring = 160;  // The angle of clenched ring
int clenched_pinky = 175;  // The angle of clenched pinky
int clenched_list[5] = {clenched_thumb, clenched_index, clenched_middle, clenched_ring, clenched_pinky};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Touch sensor pins
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int touch_thumb = 4; // The pin for touching sersor in thumb
int touch_index = 15;  // The pin for touching sersor in index finger
int touch_middle = 14; // The pin for touching sersor in middle finger
int touch_ring = 12; // The pin for touching sersor in ring finger
int touch_pinky = 13;  // The pin for touching sersor in pinky finger
int touch_list[5] = {touch_thumb, touch_index, touch_middle, touch_ring, touch_pinky};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a TouchSensor object
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Arduino.h"
#include "TouchSensor.h"

TouchSensor touch_sensor(touchRead(touch_thumb), touchRead(touch_index), touchRead(touch_middle), touchRead(touch_ring), touchRead(touch_pinky));

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// These booleans is used to indicate whether the action finish or not
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool finish_thumb = false;  // Refer to thumb
bool finish_index = false;  // Refer to index finger
bool finish_middle = false; // Refer to middle finger
bool finish_ring = false; // Refer to ring finger
bool finish_pinky = false;  // Refer to pinky finger
int finish_list[5] = {finish_thumb, finish_index, finish_middle, finish_ring, finish_pinky};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The string command
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String command_list[1] = {""};
//String command_list[2] = {"SSSSS", ""};
//String command_list[2] = {"CCCCC", ""};
//String command_list[2] = {"RRRRR", ""};
//String command_list[2] = {"SSSSS", "RCCCC"};
//String command_list[3] = {"SSSSS", "SSCSS", ""};
//String command_list[8] = {"SSSSS", "SCCCC", "RRRCC", "CSSCC", "RSRCR", "SSCCS", "RCSCC", ""};
int command_index = 0;
String command_global = command_list[command_index];
char previous_thumb = 'R';

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables for BT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String msg = "";
int command_length = 5;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables for core checking
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool core_check_main = false;
bool core_check_bt = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used to preform the action
// Para: command_string - The command string
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void perform_action(String command_string) {
    // Iterating all fingers
    for (int i = 0; i < 5; i++) {
      // Checking the finger action finish or not. false: keep moving; true = stop
      if (finish_list[i] == false) {
        // There two situations to stop the finger. 1: The destination has arrived; 2: The touch is sensed (for Curl only)
        // **Only the clenched action will use the touch sensor**
        // Clenched
        if (command_decode(i, command_string) == 0) {
          // Control the thumb, index and middle fingers
          if (i <= 2) {
            if (thumb_index_middle(i) == true) {
              pwmServo.set_is_new_action_true(i, servo_list[i]);
              touch_sensor.set_is_first_touch_true(i, touch_list[i]);        
              finish_list[i] = true;
            }            
          }
          // Control the other fingers
          else {
            if (pwmServo.move(i, servo_list[i], clenched_list[i], atg_list[i]) == true || touch_sensor.is_touch(i, touchRead(touch_list[i]), touch_list[i]) == true) {           
              pwmServo.set_is_new_action_true(i, servo_list[i]);
              touch_sensor.set_is_first_touch_true(i, touch_list[i]);        
              finish_list[i] = true;
            }
          }         
        }
        // Relax
        else if (command_decode(i, command_string) == 2) {
          if (pwmServo.move(i, servo_list[i], relax, atg_list[i]) == true) {            
            pwmServo.set_is_new_action_true(i, servo_list[i]);
            touch_sensor.set_is_first_touch_true(i, touch_list[i]);            
            finish_list[i] = true;
          }
        }
        // Straight
        else if (command_decode(i, command_string) == 1) {
          if (pwmServo.move(i, servo_list[i], straight_list[i], atg_list[i]) == true) {            
            pwmServo.set_is_new_action_true(i, servo_list[i]);
            touch_sensor.set_is_first_touch_true(i, touch_list[i]);           
            finish_list[i] = true;
          }
        }      
      }
    }      
    // Checking whether all fingers' movement are finished or not    
    if (finish_list[0] == true && finish_list[1] == true && finish_list[2] == true && finish_list[3] == true && finish_list[4] == true) {      
      for (int i = 0; i < 5; i++) {       
        finish_list[i] = false;
      }
      SerialBT.print("finished");
      Serial.println("Command string '" + command_global + "' is finished!\n");   
      next_action();
    }       
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used to decode the command string
// Para: index - The string index
// Para: command - The command string
// Return: int - 0 = Clenched; 1 = Straight; 2 = Relax; Default = Relax
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int command_decode(int index, String command) {
  if (command.charAt(index) == 'C') {
    return 0;
  }
  else if (command.charAt(index) == 'S') {
    return 1;
  }
  else if (command.charAt(index) == 'R') {
    return 2;
  }
  else {
    Serial.println("Unknown command char '" + String(command.charAt(index)) + "' on '" + String(index) + "' finger.");
    Serial.println("The command char should be 'S' or 'C' or 'R', 'S' for straight, 'C' for clenched and 'R' for relax.");
    Serial.println("Thus, the default action (relax) will be performed!\n");
    return 2;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used to control the thumb, index and middle fingers action
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool thumb_index_middle(int i) {
  bool is_neighbor_c = false;
  bool is_neighbor_less_relax = false;
  // For thumb setting
  if (i == 0) {
    if (command_global[1] == 'C' or command_global[2] == 'C') {
      is_neighbor_c = true;
    }
    if (pwmServo.finger_angle(servo_index) <= relax and pwmServo.finger_angle(servo_middle) <= relax) {
      is_neighbor_less_relax = true;
    }
  }
  // For index and middle setting
  else {   
    // Becasee the thumb has a probability stay in the relax angle,
    // we need to compare a larger value and ensure the thumb angle is less then this value (relax + (thumb_fine_tune)*2)
    // **If index or middle finger is clenched, the thumb must be relax or straight**
    if (pwmServo.finger_angle(servo_thumb) <= (relax + (thumb_fine_tune*2))) {
      is_neighbor_less_relax = true;
    }
  }

  // Only the thumb will go inside the if block, index and middle will go into else block only
  // Because the index and middle finger will not consider the thumb action whether is 'C' or not,
  // the thumb_fine_tune is used to set the thumb in the best position and not affect the index and middle fingers moving
  if (is_neighbor_c == true) {
    int angle = relax;
    if (previous_thumb == 'C') {
      angle -= thumb_fine_tune;
    }
    else {
      angle += thumb_fine_tune;
    }
    if (pwmServo.move(i, servo_list[i], angle, atg_list[i]) == true || touch_sensor.is_touch(i, touchRead(touch_list[i]), touch_list[i]) == true) {           
      return true;
    }
  }
  // If the neighbor's action is not 'C' (Index and middle finger always go into this blosk),
  // the finger will be bended to the maximum clenched angle
  else {
    // Waiting for the other finger's angle less than or equals to relax
    // 1. Thumb wait index and middle finger
    // 2. Middle or index wait thumb
    if (is_neighbor_less_relax) {
      if (pwmServo.move(i, servo_list[i], clenched_list[i], atg_list[i]) == true || touch_sensor.is_touch(i, touchRead(touch_list[i]), touch_list[i]) == true) {           
        return true;
      }
    }
    // Update the previous time
    else {
      pwmServo.set_is_new_action_true(i, servo_list[i]);     
    }
  }  
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used for assigning the next action to command_global
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void next_action() {
  command_index += 1;
//  Serial.println(sizeof(command_list)/sizeof(command_list[0]));
  if (command_index == sizeof(command_list)/sizeof(command_list[0])) {
    command_index = 0;
  }  
  previous_thumb = command_global[0];
  command_global = command_list[command_index];    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The task function for bluetooth communication core
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void task_bluetooth_code(void * pvParameters) {
  for(;;) {
    // For checking the bluetooth run on which core
    // Execute once on the initial
    if (core_check_bt == false) {
      Serial.println();
      Serial.println("Bluetooth core: " + String(xPortGetCoreID()) + ", with time: " + millis());
      core_check_bt = true;
    }  
    // Listening the BT serial
    if (SerialBT.available()) {
      char temp = SerialBT.read(); 
      msg += temp; 
    }
    else {
      // Checking the msg whether is emtpy or not
      if (msg.length() > 0) {
        String sub_msg = msg.substring(0, 5);
        if (sub_msg.length() == 5) {
            if (check_msg_char(sub_msg) == false) {
              SerialBT.print("invalid");
              Serial.println();
              Serial.println("Sub_msg invalid");
              Serial.println("sub_msg: '" + sub_msg + "'");  
              Serial.println("sub_msg.length(): '" + String(sub_msg.length()) + "'"); 
            }
            else {
              // If the new msg is valid, clear the finish list, and assign the new command to command_global
              for (int i = 0; i < 5; i++) {       
                finish_list[i] = false;
              }
              command_global = sub_msg;
              SerialBT.print("valid");
              Serial.println();
              Serial.println("command_global valid");
              Serial.println("command_global: '" + command_global + "'");  
              Serial.println("command_global.length(): '" + String(command_global.length()) + "'");        
            }
        }
        else {
          SerialBT.print("invalid");
          Serial.println();
          Serial.println("Length invalid");
          Serial.println("sub_msg: '" + sub_msg + "'");  
          Serial.println("sub_msg.length(): '" + String(sub_msg.length()) + "'");    
        }           
        msg = "";     
      }
    }
    delay(10);
  }  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used for checking the message valid or not
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool check_msg_char(String sub_msg) {
  for (int i = 0; i < sub_msg.length(); i++) {
//    Serial.println(sub_msg[i]);
    if (sub_msg[i] != 'S' && sub_msg[i] != 'C' && sub_msg[i] != 'R') {
      return false;
    }
  }
  return true;  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  SerialBT.begin("God_Hand"); //Bluetooth device name

  // Initialize the setting of second core
  // Because the default running core of ESP32 is core 1, so the bluetooth task should be assigned to core 0
  xTaskCreatePinnedToCore(
    task_bluetooth_code,  // Task function
    "task_bluetooth",  // Task name
    10000,  // Stack size of task
    NULL,  // Parameter of task
    0, // Priority of task - 0: lower; 1: higher (same as setup() and loop()) 
    &task_bluetooth,  // Task handle
    0   // Core where the task should run
    );

  // Initialize the servomotors
  pwmServo.setup();
  for (int i = 0; i < 16; i++) {
    pwmServo.write(i, relax);
  }
  
  Serial.println("Ready!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// loop()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
// Performing the action with the specific command
  if (command_global != "") {
    perform_action(command_global);
  } 

// For checking the main loop() run on which core
// Execute once on the initial
  if (core_check_main == false) {
    Serial.println();
    Serial.println("Main core: " + String(xPortGetCoreID()) + ", with time: " + millis());
    core_check_main = true;  
  }
}
