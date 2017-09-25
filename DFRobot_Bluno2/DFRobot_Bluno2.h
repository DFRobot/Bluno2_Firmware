#ifndef DFROBOT_BLUNO2_H
#define DFROBOT_BLUNO2_H

#include <stdint.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

#define EVENT_NETINFO_UPDATE 1
#define EVENT_DATA_AVAILABLE 2
#define EVENT_NONE           0

#define LDBG(...) if(dbg) {dbg->print("["); dbg->print(__FUNCTION__); dbg->print("(): "); dbg->print(__LINE__); dbg->print(" ] "); dbg->println(__VA_ARGS__);}
//#define SDBG(...) {dbg.print("["); dbg->print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}


#define EVENT_NONE    0
#define EVENT_NETINFO 1
#define EVENT_DATA    2
#define TMP_SIZE      5

extern Stream *dbg;
  
typedef struct{
	uint8_t dst;
	uint8_t src;
	uint8_t frameNo;
  uint8_t ttl;
	uint8_t payload[0];
}__attribute__ ((packed)) tBtPacketHeader,*pBtPacketHeader;

typedef struct{
	uint8_t header55;
	uint8_t headeraa;
	uint8_t length;
  uint8_t cs;
	tBtPacketHeader header;
}__attribute__ ((packed)) tPacketHeader,*pPacketHeader;

class eventNode
{
public:
  eventNode();
  eventNode(pPacketHeader header);
  eventNode(const eventNode& e);
  eventNode(const eventNode*);
  ~eventNode();
  bool updateEvent(pPacketHeader);
public:
  uint8_t event;
  uint8_t dst;     //FF:broadcast    Others: p2p
  uint8_t src;     //FF:event  Others:data
  uint8_t frameNo; //FF  0 FF   0 1 FF
  uint8_t ttl;     //
  uint16_t msgLength;
  uint16_t payloadLength;
  void *  message;
  uint8_t *  payload;
  eventNode * next;
}__attribute__ ((packed));

class DFRobot_Bluno2
{
public:
  DFRobot_Bluno2();
  ~DFRobot_Bluno2();
  bool begin(Stream &s_);
  bool setDbgSerial(Stream &s_){dbg = &s_; return true;}
  size_t readN(void* buf_, size_t len);
  int16_t readPacketPrefix( pPacketHeader header );
  bool validPacket(void* arg);
  void rAPPacket(void);
  uint8_t getCS(pPacketHeader packet);
  uint8_t getEvent();
  eventNode** getTmpEventNode(uint8_t src);
  bool pushEvent(eventNode *event);
  eventNode popEvent();
  
  bool sendData(pPacketHeader header);
  bool sendPacket(uint8_t dst, uint8_t src, const void *message, uint16_t len);
  void loop( void );
  
public:
  Stream *s;
  eventNode *eventListHeader,*eventListTail;
  pPacketHeader  sendHeader;
  const char * netinfo;
  uint8_t recvBuf[250];
  uint8_t recvOffset;
  eventNode *tmpEventNode[TMP_SIZE];
  bool recvPacketFinish;
};
extern SoftwareSerial mySerial;
#endif
