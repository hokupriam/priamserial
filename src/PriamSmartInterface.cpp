#include "Arduino.h"
#include "PriamSmartInterface.h"
#include "PriamHighlevelCommands.h"


using namespace Priam;

const uint8_t PriamSmart::DBUS0_7_Pins[8]  = {DBUS0, DBUS1, DBUS2, DBUS3, DBUS4, DBUS5, DBUS6, DBUS7};
const uint8_t PriamSmart::ADBUS0_3_Pins[3]  = {AD0, AD1, AD2};

PriamSmart::PriamSmart() :
state_(PriamSmart::state::NOTOPEN), resultRegisters_{0}
{
  //Constructor
  //This is called too early to setup ports, arduino init will overwrite it
  //Open() method sets up ports, call in Setup()
}

bool PriamSmart::AssertReset()
{
  if (state_ == PriamSmart::state::RESETHOLD)
  {
    Serial.println(F("Assert reset: already in reset hold state!"));
    return false;
  }

  digitalWrite(RESETLINE, LOW);    // hold interface in reset
  pinMode(RESETLINE, OUTPUT); //D13 reset line active low
  
  state_ = PriamSmart::state::RESETHOLD;
  return true;
}

bool PriamSmart::ReleaseFromReset()
{
  if (state_ != PriamSmart::state::RESETHOLD)
  {
    Serial.println(F("Release from reset: wrong state"));
    return false;
  }

  //digitalWrite(RESETLINE, 1);
  
  pinMode(RESETLINE, INPUT); //Reset HIGHZ

  state_ = PriamSmart::state::WAITBUSREADY;
  return true;
}

bool PriamSmart::PulseReset(unsigned long pulseLength_ms)
{
  if (!AssertReset())
    return false;

  delay(pulseLength_ms);

  return ReleaseFromReset();
}

void PriamSmart::WaitForDriveReady(unsigned long pausebetweenTries_ms, unsigned int maxtriesBeforeReset)
{

  unsigned int tryCount = 0;

  while(GetState() != PriamSmart::state::READY)
  {
    tryCount++;
    if (tryCount > maxtriesBeforeReset && maxtriesBeforeReset != 0)
    {
      Serial.println(F("Max tries waiting for drive ready exceeded, resetting interface.."));
      PulseReset();
      tryCount = 0;
    }
    delay(pausebetweenTries_ms);
  }
}

PriamSmart::state PriamSmart::GetState()
{
  switch (state_)
  {
    //If we are waiting for ready, check if interface ready now
    case PriamSmart::state::WAITBUSREADY:
    {
      uint8_t enastate = (uint8_t) digitalRead(DBUSENA);
      Serial.print(F("Waiting for interface ready, DBUSENA is "));
      Serial.println(enastate);
      if (!enastate)
      {
        state_ = PriamSmart::state::WAITINITIALCOMPREQ;
        Serial.println(F("Interface is ready, waiting for initial completion request"));
        //delay(5000); //wait for interface to settle
      }
    }
    break;

    //If we are waiting for initial completion request, wait foer it with timeout
    case PriamSmart::state::WAITINITIALCOMPREQ:
    {
        Serial.println(F("Wait for initial completion request..."));

        //If the controller is in the power-up or reset state, it will issue an initial comppletion request
        //Wait at most 5 seconds for the request
        //If no request received, assume controller was not in initial state, reset it
        InterfaceStatus stat;
        GetInterfaceStatus(stat);

        
        if (!stat.CompletionRequest())
        {
          Serial.print(F("No completion request, status is 0x"));
          Serial.println(stat.GetRawStatusVal(), HEX);
          state_ = PriamSmart::state::READY;
          
        }
        else
        {
          Serial.print(F("Completion request, status is 0x"));
          Serial.println(stat.GetRawStatusVal(), HEX);
          Serial.println(F("Acknowledge initial completion request"));
          CompletionAcknowledge();
          state_ = PriamSmart::state::READY;
          Serial.println(F("Initialization complete"));
        }
    }
      break;
    case NOTOPEN:
    case RESETHOLD:
    case READY:
    default:
      break;
  }

  return state_;
}

bool PriamSmart::IsOpen()
{
  return state_ != PriamSmart::state::NOTOPEN;
}

bool PriamSmart::Open(bool holdInReset)
{
  if (IsOpen())
    return false;

  //Initially databus is input (HIGHZ)
   SetDBUSMode(INPUT);
    
  //Set address bus as HIGHZ to save some driving current
  SetADDRBUSMode(INPUT);
    

  //Reset line  
  pinMode(RESETLINE, INPUT); //Reset line HighZ
  

  pinMode(DTREQ, INPUT); //A1 DTREQ
  pinMode(DBUSENA, INPUT); //A0 DUSENA
  pinMode(16, INPUT); //A2 not used
  
  digitalWrite(HRD, HIGH);
  pinMode(HRD, OUTPUT); //17 A3 HRD

  digitalWrite(HWR, HIGH);
  pinMode(HWR, OUTPUT); //18 A4 HWR
  
  state_ = PriamSmart::state::WAITBUSREADY;

  if (holdInReset)
    return AssertReset();
  else
    return true;
}

#if 0
TransactionStatus PriamSmart::DriveCommandSpinupAndWait(bool &allok, uint8_t driveno)
{
  

  DriveParam drive(driveno);

  RegisterValues<1> resRegs = TransactNew(CommandSpinupAndWait.GetCommandInfo(), CommandSpinupAndWait.MakeParamRegs(drive));
  allok = resRegs.Valid();
  return CommandSpinupAndWait.ParseStatus(resRegs) ;
}
#endif




void PriamSmart::SetGenericBusMode(const uint8_t * pins, uint8_t numpins, uint8_t mode)
{
  
  for (uint8_t i = 0; i < numpins; i++)
  {
    pinMode(pins[i], mode);
  }
}

bool PriamSmart::SetGenericBusValue(const uint8_t * pins, uint8_t numpins, uint8_t value)
{
  //Check if value ok for number of pins
  if (value >= (1 << numpins))
  {
    Serial.print(F("SetGenericBusValue value too large for bus size\n"));
    return false;
  }

  
  for (uint8_t i = 0; i < numpins; i++)
  {

    digitalWrite(pins[i], value & 1);
    value = (uint8_t) (value >> 1);
  }

  return true;
}

void PriamSmart::ReadGenericBusValue(const uint8_t * pins, uint8_t numpins, uint8_t &value)
{
  value = 0;
  for (uint8_t i = 0; i < numpins; i++)
  {
    if (digitalRead(pins[i]))
      value = (uint8_t) (value | (1 << i));
    
  }
}

void PriamSmart::SetDBUSMode(uint8_t mode)
{
  
  SetGenericBusMode(DBUS0_7_Pins, sizeof(DBUS0_7_Pins), mode);

}

void PriamSmart::SetADDRBUSMode(uint8_t mode)
{
  SetGenericBusMode(ADBUS0_3_Pins, sizeof(ADBUS0_3_Pins), mode);
}

void PriamSmart::ReadDBUSValue(uint8_t & val)
{
  ReadGenericBusValue(DBUS0_7_Pins, sizeof(DBUS0_7_Pins), val);
}

bool PriamSmart::OutputDBUSValue(uint8_t val)
{
  if (!SetGenericBusValue(DBUS0_7_Pins, sizeof(DBUS0_7_Pins), val))
    return false;

  SetDBUSMode(OUTPUT);
  return true;
}

bool PriamSmart::OutputADDRBUSValue(uint8_t val)
{
  if (!SetGenericBusValue(ADBUS0_3_Pins, sizeof(ADBUS0_3_Pins), val))
    return false;

  SetADDRBUSMode(OUTPUT);
  return true;
}

bool PriamSmart::RegisterRead(PriamSmart::ReadRegister address, uint8_t &value)
{
  //Bus mode should already be input, but jusţ in case
  SetDBUSMode(INPUT);

  //First output the address
  //OutputADDRBUSValue will put the address bus in output mode
  if (!OutputADDRBUSValue(address))
    return false;

  //make sure address is stable before asserting HRD, minimum 60ns
  SetupDelay();

  //Assert HRD, wait
  digitalWrite(HRD, 0);
  PulseDelay();

  //Read the bus
  ReadDBUSValue(value);

  //Deassert HRD
  digitalWrite(HRD, 1);

  //And address bus back to input
  SetADDRBUSMode(INPUT);

  return true;
}

bool PriamSmart::RegisterWrite(PriamSmart::WriteRegister address, uint8_t value)
{
  //First output the address
  //OutputADDRBUSValue will put the address bus in output mode
  if (!OutputADDRBUSValue(address))
    return false;

  //Output the address
  //OutputDBUSValue will put the address bus in output mode
  if (!OutputDBUSValue(value))
    return false;

  //make sure address and data are stable before asserting HWR, minimum 60ns
  SetupDelay();
  
  //HWR pulse
  digitalWrite(HWR, 0);
  PulseDelay();
  digitalWrite(HWR, 1);

  //Address bus back to input
  SetADDRBUSMode(INPUT);

  //Databus back to input
  SetDBUSMode(INPUT);
  
  return true;
}

bool PriamSmart::CompletionAcknowledge()
{
  return RegisterWrite(PriamSmart::WriteRegister::COMMAND, PriamCommandsByteValues::COMPLETIONACK);
}

/// Interface status
bool PriamSmart::GetInterfaceStatus(InterfaceStatus& stat)
{
  uint8_t statusval;
  if (!RegisterRead(PriamSmart::ReadRegister::IFACESTATUS, statusval))
    {
      Serial.println(F("Error reading Interface status register"));
      return false;
    }

  stat.SetStatusRegValue(statusval);
  return true;
}

