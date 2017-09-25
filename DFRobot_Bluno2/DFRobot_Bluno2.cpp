#include <DFRobot_Bluno2.h>
#include <string.h>
Stream *dbg=NULL;
eventNode::eventNode()
  :next(NULL),event(EVENT_NONE)
{

}

eventNode::eventNode(pPacketHeader header)
  :next(NULL)
{
  LDBG("Construct+++++++++\r\n");
  uint8_t *srcAddr;
  LDBG("\r\n");
  if(header->header.src == 0xFF){ //event
    LDBG("\r\n");
    event = header->header.payload[0];
    payloadLength = header->length-5;
    srcAddr = header->header.payload + 1;
  }else{                   //data
    LDBG("\r\n");
    event = EVENT_DATA;
    payloadLength = header->length-4;
    srcAddr = header->header.payload;
  }

  payload = (uint8_t*)malloc(payloadLength+1);
  memcpy(payload, srcAddr, payloadLength);
  payload[payloadLength] = 0;
  dst = header->header.dst;
  src = header->header.src;
  frameNo = header->header.frameNo;
  ttl = header->header.ttl;
  LDBG("event=");LDBG(event);
  LDBG("dst=");LDBG(dst);
  LDBG("src=");LDBG(src);
  LDBG("frameNo=");LDBG(frameNo);
  LDBG("ttl=");LDBG(ttl);
  LDBG("payloadLength=");LDBG(payloadLength);
  LDBG("payload=");LDBG((const char *)payload);
}

eventNode::~eventNode()
{
  LDBG("Destruct---------\r\n");
  if(payload)
    free(payload);
  payload=NULL;
}

eventNode::eventNode(const eventNode& e)
{
  LDBG("copy Construct++++++++++\r\n");
  event = e.event;
  dst = e.dst;     //FF:broadcast    Others: p2p
  src = e.src;     //FF:event  Others:data
  frameNo = e.frameNo; //FF  0 FF   0 1 FF
  ttl = e.ttl;     //
  payloadLength = e.payloadLength;
  payload=(uint8_t*)malloc(payloadLength+1);
  memcpy(payload,e.payload,payloadLength+1);
}

eventNode::eventNode(const eventNode* e)
{
  LDBG("copy Construct++++++******++++\r\n");
  event = e->event;
  dst = e->dst;     //FF:broadcast    Others: p2p
  src = e->src;     //FF:event  Others:data
  frameNo = e->frameNo; //FF  0 FF   0 1 FF
  ttl = e->ttl;     //
  payloadLength = e->payloadLength;
  payload=(uint8_t*)malloc(payloadLength+1);
  memcpy(payload,e->payload,payloadLength+1);
}

bool eventNode::updateEvent(pPacketHeader header)
{
  uint8_t *srcAddr;
  uint16_t length;
  LDBG("\r\n");
  if(frameNo == 0xff){
    LDBG("\r\n");
    return false;
  }
  if((event == EVENT_NETINFO) && (event != header->header.payload[0])){
    LDBG("event=");LDBG(event);
    LDBG("Xevent=");LDBG(header->header.payload[0]);
    return false;
  }
  if((dst != header->header.dst) ||
    (src != header->header.src)){
      LDBG("dst=");LDBG(dst);
      LDBG("XDST=");LDBG(header->header.dst);
      LDBG("src=");LDBG(src);
      LDBG("Xsrc=");LDBG(header->header.src);
    
      return false;
  }
  if(event == EVENT_NETINFO){
    length = header->length-5;
    srcAddr = header->header.payload+1;
    LDBG("\r\n");
  }else if(event == EVENT_DATA){
    length = header->length-4;
    srcAddr = header->header.payload;
    LDBG("\r\n");
  }
  LDBG("length=");LDBG(length);
  LDBG("payloadLength=");LDBG(payloadLength);
  uint8_t * tmp = (uint8_t*)malloc(payloadLength+length+1);
  memcpy(tmp,payload,payloadLength);
  memcpy(tmp+payloadLength,srcAddr, length);
  tmp[payloadLength+length]=0;
  payloadLength += length;
  free(payload);
  payload = tmp;
  LDBG("payloadLength=");LDBG(payloadLength);
  LDBG("payload=");LDBG((const char *)payload);
  frameNo = header->header.frameNo;
  return true;
}

uint8_t DFRobot_Bluno2::getCS(pPacketHeader packet)
{
	uint8_t cs = 0;
	uint8_t *p = (uint8_t*)packet;
	
	for(int i=0; i < packet->length; i++){
		cs += p[i+4];
	}
	return cs;
}

DFRobot_Bluno2::DFRobot_Bluno2()
  :s(NULL), eventListHeader(NULL), eventListTail(NULL), sendHeader(NULL),netinfo(NULL), recvOffset(0),
  recvPacketFinish(false)
{
  for(uint8_t i=0; i<TMP_SIZE; i++){
    tmpEventNode[i]=NULL;
  }
  memset(recvBuf, 0, sizeof(recvBuf));
}

DFRobot_Bluno2::~DFRobot_Bluno2()
{
  
}

bool DFRobot_Bluno2::begin(Stream &s_)
{
  s=&s_;
}

uint8_t DFRobot_Bluno2::getEvent()
{
  if(eventListHeader){
    return eventListHeader->event;
  }
  return EVENT_NONE;
}

bool DFRobot_Bluno2::validPacket(void* arg)
{
	bool ret = true;
	pPacketHeader packet = (pPacketHeader)arg;
	if((packet->header55 != 0x55) || (packet->headeraa != 0xaa)
		||(recvOffset != (packet->length + 4))/* || (getCS(packet) != packet->payload[packet->length])*/){
			//LDBG("recvOffset=");LDBG(recvOffset);
      //LDBG("packet->length+4=");LDBG(packet->length+4);
      ret = false;
	}
	return ret;
}

void DFRobot_Bluno2::rAPPacket(void)
{
	if(s->available()){
		recvBuf[recvOffset++] = s->read();
		LDBG((uint8_t)recvBuf[recvOffset-1],HEX);
    if(recvOffset==sizeof(recvBuf)){
      recvBuf[0] = recvBuf[sizeof(recvBuf)-1];
      recvOffset = 1;
    }else{
		  if(((uint8_t)recvBuf[0]) != 0x55){
			  LDBG((uint8_t)recvBuf[recvOffset-1],HEX);
			  recvOffset = 0;
		  }
		  if((recvOffset == 2) && (((uint8_t)recvBuf[1]) != 0xAA)){
			  LDBG((uint8_t)recvBuf[1],HEX);
			  recvOffset = 0;
		  }
    }
	  if(validPacket(recvBuf)){
      LDBG("recvPacketFinish!!!\r\n");
      recvPacketFinish = true;
		  recvOffset = 0;
	  }
  }
}

bool DFRobot_Bluno2::pushEvent(eventNode *event)
{
  LDBG("\r\n");
  if( eventListHeader == NULL){
    LDBG("\r\n");
    eventListHeader = event;
    eventListTail   = event;
    LDBG("\r\n");
  }else{
    LDBG("\r\n");
    eventListTail->next = event;
    eventListTail = event;
  }
}

eventNode DFRobot_Bluno2::popEvent()
{
  if(eventListHeader == NULL){
    return eventNode();
  }else{
    LDBG("\r\n");
    eventNode * tmp = eventListHeader;
    eventListHeader = eventListHeader->next;
    LDBG("\r\n");
    eventNode e(*tmp);
    LDBG("\r\n");
    delete(tmp);
    return e;
  }
}

eventNode** DFRobot_Bluno2::getTmpEventNode(uint8_t src)
{
  uint8_t i;
  for(i = 0; i < TMP_SIZE; i++){
    if(tmpEventNode[i] && (tmpEventNode[i]->src == src)){
      break;
    }
  }
  if( i == TMP_SIZE){
    for(i = 0; i < TMP_SIZE; i++)
    if(tmpEventNode[i] == NULL){
      break;
    }
  }
  if(i<TMP_SIZE){
    LDBG(i);
    return &tmpEventNode[i];
  }else{
    LDBG("full");
    return NULL;
  }
}

void DFRobot_Bluno2::loop( void )
{
  rAPPacket();
  if(recvPacketFinish){
    uint8_t src = ((pPacketHeader)recvBuf)->header.src;
    eventNode ** _ppEventNode = getTmpEventNode(src);
    if(_ppEventNode == NULL){
      recvPacketFinish = false;
      return;
    }
    if( *_ppEventNode == NULL ){
      LDBG("\r\n");
      *_ppEventNode = new eventNode((pPacketHeader)recvBuf);
    }else{
      if((*_ppEventNode)->updateEvent((pPacketHeader)recvBuf) == false){
        LDBG("\r\n");
        delete(*_ppEventNode);
        *_ppEventNode = NULL;
      }
    }
    if((*_ppEventNode)->frameNo == 0xff){
      LDBG("\r\n");
      pushEvent(*_ppEventNode);
      *_ppEventNode = NULL;
    }
    recvPacketFinish = false;
  }
}

bool DFRobot_Bluno2::sendData(pPacketHeader header)
{
  LDBG("dst=");LDBG(header->header.dst);
  LDBG("src=");LDBG(header->header.src);
  LDBG("frameNo=");LDBG(header->header.frameNo);
  LDBG("ttl=");LDBG(header->header.ttl);
  LDBG("payload=");LDBG((const char *)header->header.payload);
  s->write((uint8_t *)header,header->length+4);
}

bool DFRobot_Bluno2::sendPacket(uint8_t dst, uint8_t src,const void *message, uint16_t len)
{
  uint8_t index=0;
  int16_t left = len;
  int16_t total = 0;
  while(left){
    uint8_t n=left>16?16:left;
    left -= n;
    pPacketHeader header= (pPacketHeader)malloc(4+4+n+1);
    memset(header,0,4+4+n+1);
    header->header55 = 0x55;
    header->headeraa = 0xaa;
    header->length = 4+n;
    header->header.dst = dst;
    header->header.src = src;
    header->header.ttl = 5;
    header->header.frameNo = left>0?index:0xff;
    memcpy(header->header.payload, ((uint8_t*)message)+total, n);
    header->cs = getCS(header);
    index++;
    
    sendData(header);
    free(header);
    total += n;
  }
}
