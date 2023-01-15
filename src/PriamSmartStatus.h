#pragma once
#include "arduino.h"
namespace Priam
{

class InterfaceStatus
  {
    public:
      //The Smart Interface interface status bits
      enum SmartIfaceStatusBit {
        DATABUSENABLE = bit(0),
        READWRITEREQUEST = bit(1),
        DATAXFERREQUEST = bit(2),
        INTERFACEBUSY = bit(3),
        COMPLETIONREQUEST = bit(6),
        COMMANDREJECT = bit(7)
      };

    InterfaceStatus() :
      statusregval_(0)
      {}

      InterfaceStatus(uint8_t regvalue) :
      statusregval_(regvalue)
      {}

      void SetStatusRegValue(uint8_t val) {statusregval_ = val;}
      uint8_t GetRawStatusVal() {return statusregval_;}

      bool ReadyForCommand();

      bool DatabusEnabled();
      bool TransferRequest();
      bool ReadRequest();
      bool WriteRequest();
      bool Busy();
      bool CompletionRequest();
      bool CommandRejected();

     private:
       uint8_t statusregval_;
  };







};