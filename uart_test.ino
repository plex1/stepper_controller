#include "GepinSlave.h"
#include <AccelStepper.h>


AccelStepper stepper1(AccelStepper::DRIVER, 2, 3);
AccelStepper stepper2(AccelStepper::DRIVER, 4, 5);
int sleepPin = 7;

// global variables
GepinSlave gepin_slave;

// variable list declaration
typedef struct{
uint32_t motor1_target_pos;
uint32_t motor2_target_pos;
uint32_t motor1_pos;
uint32_t motor2_pos;
uint32_t motor1_status;
uint32_t motor2_status;
uint32_t var4[5];
} __attribute__ ((aligned (4), packed)) var_table_t, *p_var_table_t;

var_table_t var_table;

// function declarations
bool writeCallback(GepinSlave::message_t *message);

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
  stepper1.setMaxSpeed(int(f*1000.0));
  stepper1.setAcceleration(f*400.0);
  stepper1.setSpeed(int(f*500.0));
  stepper2.setMaxSpeed(int(f*1000.0*64.0/42.0));
  stepper2.setAcceleration(f*400.0*64.0/42.0);
  stepper2.setSpeed(int(f*500.0*64.0/42.0));
  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, HIGH);
 
  Serial.print(" -----Start:");
}

void loop() {
  gepin_slave.update();
  gepin_slave.newMessage = false;
  
  //if (gepin_slave.newMessage && (gepin_slave.message.header->command == GepinSlave::COMMAND_WRITE))
  /*var_table.motor1_pos = stepper1.currentPosition;
  var_table.motor2_pos = stepper2.currentPosition;
  var_table.motor1_status = stepper1.isRunning()
  var_table.motor2_status = stepper2.isRunning()*/
  
  stepper1.run();
  stepper2.run();
}

bool writeElemCallback(uint32_t addr, uint32_t data){
 // Serial.print(" --write elem callback -- ");
  if (addr == gepin_slave.getVarAddr(&var_table.motor1_target_pos)) {
      //Serial.print("target motor 1 changed");
      stepper1.moveTo((int32_t) data);
   
    } 
  else if (addr == gepin_slave.getVarAddr(&var_table.motor2_target_pos)) {
      //Serial.print("target motor 2 changed");
      stepper2.moveTo((int32_t) data);
   
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
