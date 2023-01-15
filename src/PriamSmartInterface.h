#pragma once
#include "Arduino.h"
#include "PriamSmartStatus.h"
#include "PriamSmartCommand.h"
#include "PriamSmartCommandResult.h"
//#include "PriamSmartCommandResult.h"
#include "PriamRegisters.h"

//Priam Smart Interface pin assignment
const uint8_t DBUS0 = 2;
const uint8_t DBUS1 = 3;
const uint8_t DBUS2 = 4;
const uint8_t DBUS3 = 5;
const uint8_t DBUS4 = 6;
const uint8_t DBUS5 = 7;
const uint8_t DBUS6 = 8;
const uint8_t DBUS7 = 9;

const uint8_t AD0 = 10;
const uint8_t AD1 = 11;
const uint8_t AD2 = 12;

const uint8_t RESETLINE = 13;

const uint8_t DTREQ = 15;
const uint8_t DBUSENA = 14;

const uint8_t HRD = 17;
const uint8_t HWR = 18;

namespace Priam
{



class PriamSmart
{

public:
  
  //Our state
  enum state {NOTOPEN, RESETHOLD, WAITBUSREADY, WAITINITIALCOMPREQ, READY};



  //The Smart Interface register addresses (read)
  enum ReadRegister {
    IFACESTATUS = 0, 
    READDISCDATA = 1,
    RESULT0 = 2,
    RESULT1 = 3,
    RESULT2 = 4,
    RESULT3 = 5,
    RESULT4 = 6,
    RESULT5 = 7
    };

  

  //The Smart Interface register addresses (write)
  enum WriteRegister {
    COMMAND = 0, 
    WRITEDISCDATA = 1,
    PARAM0 = 2,
    PARAM1 = 3,
    PARAM2 = 4,
    PARAM3 = 5,
    PARAM4 = 6,
    PARAM5 = 7
    };

  

  
  //Constructor
  PriamSmart();
  virtual ~PriamSmart() {};

  //Get state
  virtual PriamSmart::state GetState();

  //Call Open() from Setup(), this will setup the IO 
  // will hold RESET low if parameter holdInReset == true
  //Fails if already open
  bool Open(bool holdInReset = false);

  //Check if Open() has been called
  bool IsOpen();

  //Wait for drive and interface fully ready for commands
  //wait pauseBetweenTries_ms milliseconds between each internal loop iteration
  //Reset drive and continue waiting after maxtriesBeforeReset loop iterations
  //maxtriesBeforeReset == 0 means no forced resets
  void WaitForDriveReady(unsigned long pausebetweenTries_ms, unsigned int maxtriesBeforeReset);

  //Assert reset line
  bool AssertReset();
  //Release reset line
  bool ReleaseFromReset(); 
  //Pulse reset line, duration of pulse as parameter
  virtual bool PulseReset(unsigned long pulseLength_ms = 100);
  
  //Execute complete transaction on the interface, templated on number of of parameters and number of return registers
  template <int NUMPARAMS, int NUMRETURNREGS>
  RegisterValues<NUMRETURNREGS> TransactNew(CommandInfo<NUMPARAMS, NUMRETURNREGS> cmdInfo, const RegisterValues<NUMPARAMS> &parameters);


  //Read a Smart Interface register
  //Sets HAD, pulses HRD HRD and reads HCBUS. Returns to HAD HIGHZ status when done
  virtual bool RegisterRead(PriamSmart::ReadRegister address, uint8_t &value);

  //Write a Smart Interface register
  //Sets HAD and HCBUS, pulses HWR. Returns HAD and HCBUS to HIGHZ status when done
  virtual bool RegisterWrite(PriamSmart::WriteRegister address, uint8_t value);

  //Get Interface status as an InterfaceStatus object
  bool GetInterfaceStatus(InterfaceStatus& stat);

  //Get Transaction status as a TransactionStatus object
  bool GetTransactionStatus(TransactionStatus& stat);

  private:
  
  //Helper routine, set mode on a "bus" passed as an array of arduino pins. First element of array is LSB
  void SetGenericBusMode(const uint8_t * pins, uint8_t numpins, uint8_t mode);

  //Helper routine, set value on a "bus" passed as an array of arduino pins. First element of array is LSB
  //Does NOT set mode (in, out)
  bool SetGenericBusValue(const uint8_t * pins, uint8_t numpins, uint8_t value);

  //Helper routine, read value on a "bus" passed as an array of arduino pins. First element of array is LSB
  //Does NOT set mode (in, out)
  void ReadGenericBusValue(const uint8_t * pins, uint8_t numpins, uint8_t &value);

  //Set HCBUS mode
  void SetDBUSMode(uint8_t mode);

  //Set HAD mode
  void SetADDRBUSMode(uint8_t mode);
  
  //Read HCBUS
  //Does NOT set mode (in, out)
  void ReadDBUSValue(uint8_t & val); 
  
  //Output val on HCBUS. Changes HCBUS mode to OUTPUT
  bool OutputDBUSValue(uint8_t val);
  
  //Output val on HAD. Changes HAD mode to OUTPUT
  bool OutputADDRBUSValue(uint8_t val);

  //Delay before pulsing HWR/HRD, determined by constant BUSDELAY_SETUP
  void SetupDelay() {delayMicroseconds(BUSDELAY_SETUP);}

  //Delay when HWR/HRD asserted (pulse length), determined by constant BUSDELAY_PULSE
  void PulseDelay() {delayMicroseconds(BUSDELAY_PULSE);}

  //Acknowledge end of operation
  bool CompletionAcknowledge();
  

//Helper variable for setting DBUS
  static const uint8_t DBUS0_7_Pins[8];

//Helper variable for setting ADBUS
  static const uint8_t ADBUS0_3_Pins[3];

  //Delay to use before asserting HWR or HRD
  static const uint8_t BUSDELAY_SETUP = 1;

  //Delay to use for HWR/HRD pulse length
  static const uint8_t BUSDELAY_PULSE = 20;

  


//Our State
  state state_;

  uint8_t resultRegisters_[6];
        

};


template <int NUMPARAMS, int NUMRETURNREGS>
RegisterValues<NUMRETURNREGS> PriamSmart::TransactNew(CommandInfo<NUMPARAMS, NUMRETURNREGS> cmdInfo , const RegisterValues<NUMPARAMS> &parameters)
{
  InterfaceStatus stat;
  uint8_t errRegvals[NUMRETURNREGS] = {0};
  RegisterValues<NUMRETURNREGS> errorReturn(errRegvals, false);
  
  
  if (GetState() != READY)
  {
    Serial.println(F("Transact: Interface not ready!"));
    return errorReturn;
  }

  if (!GetInterfaceStatus(stat))
    return errorReturn;

  while (!stat.ReadyForCommand())
  {
    Serial.print(F("Transact: Interface not ready for command! Interface status: 0x"));
    Serial.println(stat.GetRawStatusVal(), HEX);
    
    if (stat.CompletionRequest())
    {
      Serial.print(F("Transact: Interface is expecting completion ack, acking..."));
      if (!CompletionAcknowledge())
        return errorReturn;
    }
    if (!GetInterfaceStatus(stat))
      return errorReturn;
  }

  //Set parameters
  for (uint8_t i = 0; i < NUMPARAMS; i++)
  {
    RegisterWrite((PriamSmart::WriteRegister) (PriamSmart::WriteRegister::PARAM0 + i), parameters.GetRegisterValue(i));
  }
  
  //Issue command
  RegisterWrite(PriamSmart::WriteRegister::COMMAND, cmdInfo.commandRegValue());

  Serial.println(F("Command issued, wait for completion request from interface"));

  //Read status register until done or error
  InterfaceStatus ifStatus(0);
  uint32_t bytesRead = 0;
  do
  {

    if (!GetInterfaceStatus(ifStatus))
    {
      return errorReturn;
    }

    if (ifStatus.CommandRejected())
    {
      Serial.println(F("The interface rejected the command"));
      return errorReturn;
    }

    //For now only dump bytes to serial monitor
    if (ifStatus.ReadRequest())
    {
      uint8_t val;
      char tmp[8];
      RegisterRead(PriamSmart::ReadRegister::READDISCDATA, val);

      if (!bytesRead)
      {
        //Serial.println(F("Drive has data!"));
      }

      if (bytesRead && !(bytesRead % 16))
        Serial.print(F("\n"));
      
      sprintf(tmp, "%02X ", val);
      Serial.print(tmp);
      bytesRead++;
    }
    
  } while (!ifStatus.CompletionRequest());

  if (bytesRead)
    Serial.print("\n");

  Serial.println(F("Completion request signaled"));

  
  //Read result registers
  uint8_t returnValues[NUMRETURNREGS];
  for (uint8_t i = 0; i < NUMRETURNREGS; i++)
  {
    RegisterRead((PriamSmart::ReadRegister) (PriamSmart::ReadRegister::RESULT0 + i), returnValues[i]);
  }
  RegisterValues<NUMRETURNREGS> cmdResult(returnValues, true);
  
  //Acknowledge
  CompletionAcknowledge();

  //Serial.println(F("Transaction complete"));
  
  return cmdResult;
}


}