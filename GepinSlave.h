// Gepin.h

#ifndef Gepin_h
#define Gepin_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

class GepinSlave
{
public:

	// header definition
	typedef struct { 
	  uint8_t id;
	  uint8_t x;// protocol identifier?
	  uint8_t command;
	  union {
		struct {
		  uint8_t request : 1;
		  uint8_t incr : 1;
		  uint8_t nack : 1; // tbc
		  uint8_t use_checksum : 1; //tbc
		  uint8_t reserved : 4; //byte/word addressed, 8/16/32/64 bit addressing, address shift
		} fields;
		uint8_t value;
	  } flags;	  
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
	
	static const uint8_t header_len = 12; //in bytes
	static const uint8_t max_data_len = 16;
	uint32_t msg_data[max_data_len];

	message_t message;
	
	uint32_t *pVariables;
	uint32_t variable_list_len;
	
	bool waitingForHeader = true;
	
	GepinSlave(uint32_t *pVariables);
	GepinSlave(void);
	
	void update(void);
	
private:
	bool isMessageAvailable(void);
	bool isHeaderAvailable(void);
	bool isDataAvailable(uint16_t data_len_words);
	void getMessageHeader(msg_header_t *header);
	void getMessageData(uint32_t *data, uint8_t len);
	void getMessage(message_t *message);
	void sendMessage(message_t *message);
	
};

#endif 

	
	

