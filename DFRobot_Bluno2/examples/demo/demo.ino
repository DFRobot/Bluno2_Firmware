//bluno2 network
#include <DFRobot_Bluno2.h>
#include <ArduinoJson.h>

DFRobot_Bluno2 blunoNet;
int blunoID=0;
void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  blunoNet.begin(Serial1);//获取id
  blunoNet.setDbgSerial(Serial);
}

void loop()
{
  uint8_t event=blunoNet.getEvent();//queue
  switch(event)
  {
    case EVENT_NETINFO:
    {
      eventNode e = blunoNet.popEvent();
      
      LDBG("e.event=");LDBG(e.event);
      LDBG("e.payload=");LDBG((const char *)e.payload);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject((const char *)e.payload);
      if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
      }
      int s =  root["r"].size();
      for(int i = 0; i < s; i++){
        if(root["r"][i]["s"].as<int>()){
          blunoID = root["r"][i]["i"].as<int>();
          break;
        }
      LDBG("blunoID=");LDBG(blunoID);
      break;
      }
    }
    case EVENT_DATA:
    {
      eventNode e = blunoNet.popEvent();
      LDBG("e.payload=");LDBG((const char *)e.payload);
      blunoNet.sendPacket(e.src, blunoID, "hello12345678901234567890",26);
      break;
    }
    default: //no event
    {
      break;
    }
  }
  blunoNet.loop();
  if(Serial.available()){
    char message=Serial.read();
    blunoNet.sendPacket(!blunoID, blunoID, &message,1);
  }
}