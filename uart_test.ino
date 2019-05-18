#include "queue.h"

int firstSensor = 0;    // first analog sensor
int secondSensor = 0;   // second analog sensor
int thirdSensor = 0;    // digital sensor
int inByte = 0;         // incoming serial byte

// -- type definitions

// header definition
typedef struct {  
  union {
    struct {
      uint8_t incr : 1;
      uint8_t request : 1;
      uint8_t nack : 1; // tbc
      uint8_t use_checksum : 1; //tbc
      uint8_t reserved : 4; //byte/word addressed, 8/16/32/64 bit addressing
    } fields;
    uint8_t value;
  } flags;
  uint8_t command;
  uint8_t x;// protocol identifier?
  uint8_t id;
  uint32_t addr;
  uint32_t len;
} __attribute__ ((aligned (4), packed)) msg_header_t;

typedef struct{
  msg_header_t *header;
  uint32_t *data;
}
message_t, *pMessage_t;

// instantiation
msg_header_t msg_header;
//uint8_t *msg_header_byte = (uint8_t *)&msg_header;


const uint8_t header_len = 12; //in bytes
const uint8_t max_data_len = 16;
uint32_t msg_data[max_data_len];

message_t message;


uint32_t elem_left = 0;
uint32_t uart_drx[4];
uint32_t data;

// variable list declaration
typedef struct{
uint32_t var1;
uint32_t var2;
uint32_t var3;
uint32_t var4[5];
} __attribute__ ((aligned (4), packed)) variable_list_t, *p_variable_list_t;

variable_list_t variable_list;
uint8_t variable_list_len;

bool waitingForHeader = true;

// ------------------------------------


bool isMessageAvailable(){
 return (queueLength>=16);
}

bool isHeaderAvailable(){
 return (queueLength>=header_len);
}

bool isDataAvailable(uint16_t data_len){
  return (queueLength>=data_len);
}

void getMessageHeader(msg_header_t *header){
    for (int i=0; i<header_len; i++) {
      ((uint8_t *)header)[i] = queue_pop();
    }
}

void getMessageData(uint32_t *data, uint8_t len){
  for (int i=0; i<len/4; i++){
    uint32_t dataword=0;
    uint32_t b_rx;
    for (int j=0; j<4; j++){
      b_rx = queue_pop();
      dataword = dataword + (b_rx<<((3-j)*8));
    }
    data[i] = dataword; 
  }
}

void getMessage(message_t *message){
   if (isMessageAvailable()){
    // get request
    for (int i=0; i<12; i++) {
      ((uint8_t *)(message->header))[i] = queue_pop();
    }
        
    data=0;
    for (int j=0; j<4; j++){
      //while (Serial1.available() <= 0) {}
      uart_drx[j] = queue_pop(); //Serial1.read();
      data = data + (uart_drx[j]<<((3-j)*8));
     }
     message->data[0] = data;      
  } else {;}
  //message= null;  
}

void sendMessage(message_t *message){
  for (int i=0; i<12; i++) {
    Serial1.write(((uint8_t *)(message->header))[i]);
  }
       
  for (int j=0; j<4; j++){
    Serial1.write(message->data[0]>>((3-j)*8));
  } 
}


void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(2, INPUT);   // digital sensor is on digital pin 2
  //establishContact();  // send a byte to establish contact until receiver responds

  // assign variables
  message.header = &msg_header;
  message.data = msg_data;
  variable_list_len = sizeof(variable_list)/sizeof(uint32_t);
}

void loop() {
  // if we get a valid byte, read analog ins:
  if (Serial1.available() > 0) {
    // get incoming byte:
    inByte = Serial1.read();
    Serial.print(" IN:");
    Serial.print(inByte, HEX); 
    queue_push(inByte);
    //Serial.write(inByte);
  }

  if (waitingForHeader && isHeaderAvailable()){
    getMessageHeader(message.header);
    waitingForHeader = false;
    Serial.print("Rx Header \n");
  }

  //if (isMessageAvailable()){
  //  getMessage(&message);

  if (waitingForHeader == false && isDataAvailable(message.header->len)){
    getMessageData(message.data, message.header->len);
    Serial.print(" ID:");
    Serial.print(message.header->id, HEX); 
    Serial.print(" LEN:");
    Serial.print(message.header->len, HEX); 
    Serial.print(" Addr:");
    Serial.print(message.header->addr,HEX); 
    Serial.print(" Data:");
    Serial.print(message.data[0],HEX); 
    Serial.print(" request: ");
    Serial.print(message.header->flags.fields.request, HEX);
    Serial.print(" command: ");
    Serial.print(message.header->command, HEX);
    Serial.print("\n");

    bool response_generated = false;
    message.header->flags.fields.request = 0;
    // write command handling
    if (message.header->command == 1) {
      if(message.header->len == 4){
        if (message.header->addr + message.header->len <= variable_list_len){
          ((uint32_t *) &variable_list)[message.header->addr] =  message.data[0];
          //generate response    
          response_generated = true;
          Serial.print("write");
        }
      }
    }

    // read command handling
    if (message.header->command == 0) {
      if(message.header->len == 4){
        if (message.header->addr + message.header->len <= variable_list_len){
          message.data[0] = ((uint32_t *) &variable_list)[message.header->addr];
          //generate response
          response_generated = true;
          Serial.print("read");
        }
      }
    }

    if (response_generated == false) {
      // generate nack
      message.header->flags.fields.nack=0;
    }
    
    sendMessage(&message);
    waitingForHeader=true;
    
  }

}
