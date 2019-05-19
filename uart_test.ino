#include "GepinSlave.h"

// variable list declaration
typedef struct{
uint32_t var1;
uint32_t var2;
uint32_t var3;
uint32_t var4[5];
} __attribute__ ((aligned (4), packed)) variable_list_t, *p_variable_list_t;

variable_list_t variable_list;

GepinSlave gepin_slave;

void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  gepin_slave.variable_list_len = sizeof(variable_list)/sizeof(uint32_t); 
  gepin_slave.pVariables = (uint32_t *) &variable_list;
  Serial.print(" -----Start:");
}

void loop() {
  gepin_slave.update();
}
