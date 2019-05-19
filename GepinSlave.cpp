
#include "GepinSlave.h"
#include "queue.h"
#include "utils.h"

void queue_push(const char l_item);

GepinSlave::GepinSlave(uint32_t *pVariablesInit)
{
	pVariables = pVariablesInit;
	GepinSlave();
}

GepinSlave::GepinSlave(void)
{
	variable_list_len = 16;
	// assign variables
	message.header = &msg_header;
	message.data = msg_data;
}

void GepinSlave::update(void)
{
  int inByte;         // incoming serial byte	
  if (Serial1.available() > 0) {
    // get incoming byte:
    inByte = Serial1.read();
    Serial.print(" IN:");
    Serial.print(inByte, HEX); 
    queue_push(inByte);
    Serial.write(inByte); //Debug
  }

  if (waitingForHeader && isHeaderAvailable()){
    getMessageHeader(message.header);
    waitingForHeader = false;
    Serial.print("Rx Header \n");
  }

  if (waitingForHeader == false && (isDataAvailable(message.header->len) || message.header->command==0)){
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
      if(message.header->len >= 1){
        if (message.header->addr + message.header->len <= variable_list_len){
          for (int i=0; i<message.header->len; i++){
            ((uint32_t *) pVariables)[message.header->addr+i] =  message.data[i];
            Serial.print("Saved: "); Serial.print(message.data[i],HEX);
          }
          //generate response    
          response_generated = true;
          Serial.print("write");
        }
      }
    }

    // read command handling
    if (message.header->command == 0) {
      if(message.header->len >= 1){
        if (message.header->addr + message.header->len <= variable_list_len){
          for (int i=0; i<message.header->len; i++){
            message.data[i] = ((uint32_t *) pVariables)[message.header->addr+i];
            Serial.print("Retrived: "); Serial.print(message.data[i],HEX);
          }
          //generate response
          response_generated = true;
          Serial.print("read");
        }
      }
    }

    if (response_generated == false) {
      // generate nack
      message.header->flags.fields.nack=1;
    }
    
    sendMessage(&message);
    waitingForHeader=true;
    
  }
}

bool GepinSlave::isMessageAvailable(){
 return (queueLength>=16);
}

bool GepinSlave::isHeaderAvailable(){
 return (queueLength>=header_len);
}

bool GepinSlave::isDataAvailable(uint16_t data_len_words){
  return (queueLength>=4*data_len_words);
}

void GepinSlave::getMessageHeader(msg_header_t *header){
    for (int i=0; i<header_len; i++) {
      ((uint8_t *)header)[i] = queue_pop();
    }
	header->addr = ntohl(header->addr);
	header->len = ntohl(header->len);
}

void GepinSlave::getMessageData(uint32_t *data, uint8_t len){
  for (int i=0; i<len; i++){
    uint32_t dataword=0;
    uint32_t b_rx;
    for (int j=0; j<4; j++){
      b_rx = queue_pop();
      dataword = dataword + (b_rx<<((3-j)*8));
    }
    data[i] = ntohl(dataword); 
  }
}

void GepinSlave::getMessage(message_t *message){
   if (isMessageAvailable()){
    // get request
    for (int i=0; i<12; i++) {
      ((uint8_t *)(message->header))[i] = queue_pop();
    }
	message->header->addr = ntohl(message->header->addr);
	message->header->len = ntohl(message->header->len);
	
	uint32_t data;
    if (message->header->command == 1){
      for (int i=0; i<message->header->len; i++){
        data=0;
        for (int j=0; j<4; j++){
          data = data + (queue_pop()<<((3-j)*8));
        }
        message->data[i] = ntohl(data); 
      } 
    }
         
  } else {;}
}

void GepinSlave::sendMessage(message_t *message){
  message->header->addr = htonl(message->header->addr);
  message->header->len = htonl(message->header->len);
  for (int i=0; i<12; i++) {
    Serial1.write(((uint8_t *)(message->header))[i]);
  }
  message->header->addr = ntohl(message->header->addr);
  message->header->len = ntohl(message->header->len);

  uint32_t data;
  if (message->header->command == 0){	
    for (int i=0; i<message->header->len; i++){
	data = htonl(message->data[i]);		
      for (int j=0; j<4; j++){
        Serial1.write(data>>((3-j)*8));
      } 
    }
  }
}