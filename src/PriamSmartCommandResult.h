#pragma once
#include "arduino.h"
#include "PriamSmartStatus.h"
#include "PriamRegisters.h"
namespace Priam
{

//Result class for transaction status
class TransactionStatus
  {
    public:
    enum CompletionType {
        GOOD = 0,
        SYSERROR = 1,
        OPERATORINTERVENTION = 2,
        CMDDRIVEERROR = 3
      };
    static const uint8_t NUMREGS = 1;
    TransactionStatus(uint8_t statusregval, bool commsError) :
      commsError_(commsError), statusregval_(statusregval)
      {
        drive_ = (uint8_t) (statusregval_ >> 6);
        comptype_ = (CompletionType) ((statusregval_ >> 4) & 3);
        compcode_ = statusregval_ & 0xF;
      }

      uint8_t GetRawStatusVal() {return statusregval_;}

      bool CommsError() {return commsError_;}
      uint8_t Drive() {return drive_; };
      CompletionType CompType(){return comptype_;}
      uint8_t Code(){return compcode_;};

      bool IsErrorStatus() {return comptype_ != 0;}
      
      static TransactionStatus ParseStatus(RegisterValues<NUMREGS> regs) 
      {

        return TransactionStatus(regs.GetRegisterValue(0), !regs.Valid());
      }
     
     private:
       bool commsError_;
       uint8_t statusregval_;
       uint8_t drive_;
       CompletionType comptype_;
       uint8_t compcode_;
  };

//Helper class for head/cylinder address
class HeadAndCylinder
  {
    public:
        
        HeadAndCylinder(uint8_t reg1, uint8_t reg2)
        {
            head_ = (uint8_t) ((reg1 >> 4) & 7);
            cylinder_ = (uint16_t) (((reg1 & 0xF) << 8) | reg2);
        }

        uint8_t Head() {return head_; };
        uint16_t Cylinder() {return cylinder_; };
        
        
    private:
        uint8_t head_;
        uint16_t cylinder_;
  };

//Result class for drive parameters
class ResultDriveParams
{
  public:
  static const uint8_t NUMREGS = 6;
  ResultDriveParams(uint8_t regstatus, uint8_t regheadcyl1, uint8_t regheadcyl2, 
              uint8_t regsectors, uint8_t regsectsizeMSB, uint8_t regsectsizeLSB, bool commsError) :
  status_(regstatus, commsError), headAndCyl_(regheadcyl1, regheadcyl2), sectorsPertrack_(regsectors)
  {
    logicalSectorSize_ = uint16_t ((regsectsizeMSB << 8) | regsectsizeLSB);
  }

  TransactionStatus GetStatus() {return status_;}
  uint8_t Heads() {return headAndCyl_.Head(); }
  uint16_t Cylinders() {return headAndCyl_.Cylinder();}
  uint8_t SectorsPerTrack(){return sectorsPertrack_;}
  uint16_t LogicalSectorSize(){return logicalSectorSize_;};
  
  static ResultDriveParams ParseStatus(RegisterValues<NUMREGS> regs) 
  {

    return ResultDriveParams(regs.GetRegisterValue(0), 
                       regs.GetRegisterValue(1),
                       regs.GetRegisterValue(2),
                       regs.GetRegisterValue(3),
                       regs.GetRegisterValue(4),
                       regs.GetRegisterValue(5),
                       !regs.Valid());
  }
  
  private:
    TransactionStatus status_;
    HeadAndCylinder headAndCyl_;
    uint8_t sectorsPertrack_;
    uint16_t logicalSectorSize_;

};

//Result class cylinder address
class ResultCylinder
{
  public:
  static const uint8_t NUMREGS = 3;
  ResultCylinder(uint8_t regstatus, uint8_t regcylMSB, uint8_t regcylLSB, bool commsError) :
  status_(regstatus, commsError), headAndCyl_(regcylMSB, regcylLSB) {}

  TransactionStatus GetStatus() {return status_;}
  // uint8_t Heads() {return headAndCyl_.Head(); }
  uint16_t Cylinder() {return headAndCyl_.Cylinder();}
  
  
  static ResultCylinder ParseStatus(RegisterValues<NUMREGS> regs) 
  {

    return ResultCylinder(regs.GetRegisterValue(0), 
                       regs.GetRegisterValue(1),
                       regs.GetRegisterValue(2),
                       !regs.Valid());
  }
  
  private:
    TransactionStatus status_;
    HeadAndCylinder headAndCyl_;    

};

//Result class Head/cylinder/sector
class ResultHeadCylinderSector
{
  public:
  static const uint8_t NUMREGS = 4;
  ResultHeadCylinderSector(uint8_t regstatus, uint8_t regcylMSB, uint8_t regcylLSB, uint8_t regsector, bool commsError) :
  status_(regstatus, commsError), headAndCyl_(regcylMSB, regcylLSB), sector_(regsector) {}

  TransactionStatus GetStatus() {return status_;}
  uint8_t Head() {return headAndCyl_.Head(); }
  uint16_t Cylinder() {return headAndCyl_.Cylinder();}
  uint8_t Sector() {return sector_;}
  
  static ResultHeadCylinderSector ParseStatus(RegisterValues<NUMREGS> regs) 
  {

    return ResultHeadCylinderSector(regs.GetRegisterValue(0), 
                       regs.GetRegisterValue(1),
                       regs.GetRegisterValue(2),
                       regs.GetRegisterValue(3),
                       !regs.Valid());
  }
  
  private:
    TransactionStatus status_;
    HeadAndCylinder headAndCyl_;  
    uint8_t sector_;  

};

}