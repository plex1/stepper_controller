#include "GepinSlave.h"


// global variables
GepinSlave gepin_slave;

// variable list declaration
typedef struct{
uint32_t motor1_target_pos;
uint32_t motor2_target_pos;
uint32_t motor1_pos;
uint32_t motor2_pos;
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
  
  Serial.print(" -----Start:");
}

void loop() {
  gepin_slave.update();
  gepin_slave.newMessage = false;
  //if (gepin_slave.newMessage && (gepin_slave.message.header->command == GepinSlave::COMMAND_WRITE))
  var_table.motor1_pos = 1;
  var_table.motor2_pos = 2;
}

bool writeCallback(GepinSlave::message_t *message){
  
    if (gepin_slave.message.header->addr == gepin_slave.getVarAddr(&var_table.motor1_target_pos)) {
      Serial.print("target motor 1 changed");
    } 
    else if (gepin_slave.message.header->addr == gepin_slave.getVarAddr(&var_table.motor2_target_pos)) {
      Serial.print("target motor 2 changed");
    }
    else {
      return false;
    }
  return true;
}
