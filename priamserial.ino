////////////////////////
// Serial-to-PriamSmart interface v1.02, 13 Jan 2023
// Simple interface to read and write Priam Smart registers

// Serial format:
// Register read - in 1 byte regindex; out 1 byte regvalue
// Register write - in 2 bytes: regindex | 0x80, regval. No output
// Buffering for read and write commands, this needs to be implemented in the SIMH driver as well:
// On receiving READ DATA command, read all requested bytes from HDD and send as single buffer to PC
// On receiving WRITE DATA command, read all requested bytes as single buffer from PC and write to disk
// This attempts to mitigate issues causeď by USB latency with single byte transfers
////////////////////////

#include <stdint.h>


#include "src/PriamSmartInterface.h"
#include "src/PriamDrive.h"

#define PRIAMSTATUS_CMDREJECT (1 << 7)
#define PRIAMSTATUS_COMPREQ (1 << 6)
#define PRIAMSTATUS_BUSY (1 << 3)
#define PRIAMSTATUS_DATAXFERREQ (1 << 2)
#define PRIAMSTATUS_ISREADREQ (1 << 1)
#define PRIAMSTATUS_DBUSENABLE (1 << 0)


using namespace Priam;

PriamSmart smartInterface;
PriamDrive drive(smartInterface);

uint8_t globalBuf[1024];
uint8_t sectorsrequested;

#define PINKLED 19

void setup() {
  Serial.begin(115200);
  

  if (!smartInterface.Open(false))
    Serial.print(F("Interface class open error!\n"));
  

  digitalWrite(PINKLED, HIGH);   // turn the LED off
  pinMode(PINKLED, OUTPUT); //19 A5 led on shield

  
}


// the loop function runs over and over again forever
void loop() {
  
  uint8_t reg, val;
  
  if  (!Serial.available())
  {
  
    return;
  }

  
  reg = Serial.read();

  if (reg == 0x55)
  {
    //Ping from host
    Serial.write(0xAA);
    digitalWrite(PINKLED, LOW);   // turn the LED on
    return;
  }

  if (reg & 0x80)
  {
    
    //Write request, get value to write
    if (Serial.readBytes(&val, 1) != 1)
    {
      //Timed out
      return;
    }

    //Check register valid
    reg = reg & 0x7F;
    if (reg > 7)
      return;

    if (reg == PriamSmart::WriteRegister::PARAM4)
    {
      //Save number of sectors for write buffering
      sectorsrequested = val;
    }
    


    if (reg == PriamSmart::WriteRegister::COMMAND && (val == PriamCommandsByteValues::READDATANORETRY
                                          || val == PriamCommandsByteValues::READDATAWITHRETRY))
    {
      //Kick off the read command
      smartInterface.RegisterWrite((PriamSmart::WriteRegister) reg, val);

      //Read until completion request signalled
      uint8_t s;
      uint32_t index = 0;
      do
      {
        do
        {
          smartInterface.RegisterRead(PriamSmart::ReadRegister::IFACESTATUS, s);
        } while ((!(s & PRIAMSTATUS_DATAXFERREQ)) && (!(s & PRIAMSTATUS_COMPREQ)));
      
        if (s & PRIAMSTATUS_COMPREQ)
          break;
        
        uint8_t d;
        smartInterface.RegisterRead(PriamSmart::ReadRegister::READDISCDATA, d);
        globalBuf[index] = d;
        index++;
      } while(1);
      
      //for (uint32_t i = 0; i < index; i++)
      Serial.write(globalBuf, index);
    }
    else if (reg == PriamSmart::WriteRegister::COMMAND && (val == PriamCommandsByteValues::WRITEDATANORETRY
                                          || val == PriamCommandsByteValues::WRITEDATAWITHRETRY))
    {
      //Receive bytes to be written
      Serial.readBytes(globalBuf, sectorsrequested * 512);
      //Kick off the write command
      smartInterface.RegisterWrite((PriamSmart::WriteRegister) reg, val);

      //Write until completion request signalled
      uint8_t s;
      uint32_t index = 0;
      do
      {
        do
        {
          smartInterface.RegisterRead(PriamSmart::ReadRegister::IFACESTATUS, s);
        } while ((!(s & PRIAMSTATUS_DATAXFERREQ)) && (!(s & PRIAMSTATUS_COMPREQ)));
      
        if (s & PRIAMSTATUS_COMPREQ)
          break;
        
        smartInterface.RegisterWrite(PriamSmart::WriteRegister::WRITEDISCDATA, globalBuf[index]);
        index++;
      } while(1);
      
    }
    else
    {
      smartInterface.RegisterWrite((PriamSmart::WriteRegister) reg, val);
    }
  }
  else
  {
    //Read request
    if (reg > 7)
      return;

    smartInterface.RegisterRead((PriamSmart::ReadRegister) reg, val);
    Serial.write(val);
  }
  
  
  
}
