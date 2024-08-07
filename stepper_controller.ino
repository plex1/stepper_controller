#include "GepinSlave.h"
#include <AccelStepper.h>


AccelStepper stepper1(AccelStepper::DRIVER, 2, 3);
AccelStepper stepper2(AccelStepper::DRIVER, 4, 5);
const int sleepPin = 7;
const int m0_pin = 8;
const int m1_pin = 9;
const int speedScale = 1024;

// global variables
GepinSlave gepin_slave;

// variable list declaration
typedef struct{
uint32_t id;
uint32_t control;  
uint32_t motor1_target_pos;
uint32_t motor2_target_pos;
uint32_t motor1_pos;
uint32_t motor2_pos;
uint32_t motor1_status;
uint32_t motor2_status;
uint32_t target_speed;
uint32_t var4[5];
} __attribute__ ((aligned (4), packed)) var_table_t, *p_var_table_t;

var_table_t var_table;

// function declarations
bool writeCallback(GepinSlave::message_t *message);
void set_speed(float f);

void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  gepin_slave.variable_list_len = sizeof(var_table)/sizeof(uint32_t); 
  gepin_slave.pVariables = (uint32_t *) &var_table;
  gepin_slave.registerWriteCallback(&writeCallback);
  gepin_slave.registerWriteElemCallback(&writeElemCallback);
  
  float f = 1.0; // factor to change speed
  set_speed(f);
  var_table.target_speed = (int) (f * (float) speedScale);

  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, LOW);
  pinMode(m0_pin, OUTPUT);
  pinMode(m1_pin, OUTPUT);
  digitalWrite(m0_pin, HIGH);
  digitalWrite(m1_pin, HIGH);
  var_table.id = 0xfa84;
 
  Serial.print(" -----Start:");
}

void loop() {
  gepin_slave.update();
  gepin_slave.newMessage = false;
  
  //if (gepin_slave.newMessage && (gepin_slave.message.header->command == GepinSlave::COMMAND_WRITE))
  var_table.motor1_pos = (uint32_t) stepper1.currentPosition();
  var_table.motor2_pos = (uint32_t) stepper2.currentPosition();
  var_table.motor1_status = stepper1.isRunning();
  var_table.motor2_status = stepper2.isRunning();
  
  stepper1.run();
  stepper2.run();
}

bool writeElemCallback(uint32_t addr, uint32_t data){
 // Serial.print(" --write elem callback -- ");
  if (addr == gepin_slave.getVarAddr(&var_table.motor1_target_pos)) {
      //Serial.print("target motor 1 changed");
      stepper1.moveTo((int32_t) data);
      return true;
    } 
  else if (addr == gepin_slave.getVarAddr(&var_table.motor2_target_pos)) {
      //Serial.print("target motor 2 changed");
      stepper2.moveTo((int32_t) data);
      return true;
  }
   else if (addr == gepin_slave.getVarAddr(&var_table.target_speed)) {
      //Serial.print("target motor 2 changed");
      if (data < 65535) {
        set_speed(float(data)/((float) speedScale));
        return true;
      }
      return false;   
  }
  else if (addr == gepin_slave.getVarAddr(&var_table.control)) {
    
      if (data & (1<<0)) {
        digitalWrite(sleepPin, LOW);
      }
      if (data & (1<<1)) {
        digitalWrite(sleepPin, HIGH);
      }
      if (data & (1<<2)) { //set pos to zero
        stepper1.setCurrentPosition(0);
        stepper2.setCurrentPosition(0);
      }
      if (data & (1<<5)) { //set microstep resolution
        digitalWrite(m0_pin, ((data & (1<<3))>0));
        digitalWrite(m1_pin, ((data & (1<<4))>0));
        
      }
      return true;
      
  }
  return false;
}

// change to one message callback?
bool writeCallback(GepinSlave::message_t *message){
  
    /*if (gepin_slave.message.header->addr == gepin_slave.getVarAddr(&var_table.motor1_target_pos)) {
      Serial.print("target motor 1 changed");
      stepper1.moveTo((int32_t) gepin_slave.message.data[0]);
   
    } 
    else if (gepin_slave.message.header->addr == gepin_slave.getVarAddr(&var_table.motor2_target_pos)) {
      Serial.print("target motor 2 changed");
      stepper2.moveTo((int32_t) gepin_slave.message.data[0]);
   
    }
    else {
      return false;
    }
  return true;*/
  return false;
}

void set_speed(float f){
  stepper1.setMaxSpeed(int(f*1000.0));
  stepper1.setAcceleration(f*400.0);
  stepper1.setSpeed(int(f*500.0));
  stepper2.setMaxSpeed(int(f*1000.0*64.0/42.0));
  stepper2.setAcceleration(f*400.0*64.0/42.0);
  stepper2.setSpeed(int(f*500.0*64.0/42.0));
}
