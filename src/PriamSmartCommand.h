#pragma once
#include "arduino.h"


#include "PriamRegisters.h"

#define SMARTCMDMAXPARAMS 6

namespace Priam
{

class PriamSmart;

enum PriamCommandsByteValues {
    COMPLETIONACK = 0, 
    READDRIVETYPE = 0x86,
    READDRIVEPARAM = 0x85,
    READDATAWITHRETRY = 0x53,
    WRITEDATANORETRY = 0x42,
    WRITEDATAWITHRETRY = 0x52,
    READDATANORETRY = 0x43,
    INTERNALSTATUS = 5,
    SOFTWARERESET = 7,
    SEQUENCEUPANDRETURN = 0x83,
    SEQUENCEUPANDWAIT = 0x82,
    SEQUENCEDOWN = 0x81,
    SEEKWITHRETRY = 0x51,
    SEEKNORETRY = 0x41,
    VERIFYDISK = 0xA3
    };

  template <int NUMPARAMS, int NUMRESULTREGS>
class CommandInfo
{
  public:
  CommandInfo(uint8_t cmdCode) : cmdCode_(cmdCode) {};
  uint8_t commandRegValue(){return cmdCode_;}
  uint8_t NumParams() {return NUMPARAMS;}
  uint8_t NumResultRegs() {return NUMRESULTREGS;}
  private:
  uint8_t cmdCode_;
};

//Parameter class for drive number
class DriveParam
{
  public:
    static const uint8_t NUMREGS = 1;
    DriveParam(uint8_t drivenr) : drivenr_(drivenr) {};
    uint8_t GetDriveNr() {return drivenr_;}

    static RegisterValues<NUMREGS> MakeRegs(DriveParam &p) 
    {
      uint8_t r[NUMREGS];
      r[0] = p.GetDriveNr();
      return RegisterValues<NUMREGS> (r, true);
    };

  private:
    uint8_t drivenr_;
};

//Helper class for head/cylinder address
class HeadAndCylinderParamHelper
  {
    public:
        
        HeadAndCylinderParamHelper(uint8_t head, uint16_t cylinder) :
        head_(head), cylinder_(cylinder)
        {
            
        }

        void ToRegisters(uint8_t &reg1, uint8_t &reg2)
        {
          reg1 = (uint8_t) (((head_ & 7) << 4) | (cylinder_ >> 8));
          reg2 = (uint8_t) (cylinder_ & 0xFF);
        }

        uint8_t Head() {return head_; };
        uint16_t Cylinder() {return cylinder_; };
        
        
    private:
        uint8_t head_;
        uint16_t cylinder_;
  };

//Parameter class for seek operation
class SeekParam
{
  public:
    static const uint8_t NUMREGS = 3;
    SeekParam(uint8_t drivenr, uint8_t head, uint16_t cylinder) : 
    drivenr_(drivenr), headAndCylinder_(head, cylinder) {};
    uint8_t GetDriveNr() {return drivenr_;}
    HeadAndCylinderParamHelper GetHeadAndCylinder(){return headAndCylinder_;}

    static RegisterValues<NUMREGS> MakeRegs(SeekParam &p) 
    {
      uint8_t r[NUMREGS] = {0};
      r[0] = p.GetDriveNr();
      p.GetHeadAndCylinder().ToRegisters(r[1], r[2]);
      return RegisterValues<NUMREGS> (r, true);
    };

  private:
    uint8_t drivenr_;
    HeadAndCylinderParamHelper headAndCylinder_;
};

//Parameter class for Read operation
class DiskReadParam
{
  public:
    static const uint8_t NUMREGS = 5;
    DiskReadParam(uint8_t drivenr, uint8_t head, uint16_t cylinder, uint8_t sector, uint8_t multiSectorCount) : 
    drivenr_(drivenr), headAndCylinder_(head, cylinder), sector_(sector), multiSectorCount_(multiSectorCount) {};
    uint8_t GetDriveNr() {return drivenr_;}
    HeadAndCylinderParamHelper GetHeadAndCylinder(){return headAndCylinder_;}
    uint8_t Sector(){return sector_;}
    uint8_t MultiSectorCount(){return multiSectorCount_;}

    static RegisterValues<NUMREGS> MakeRegs(DiskReadParam &p) 
    {
      uint8_t r[NUMREGS] = {0};
      r[0] = p.GetDriveNr();
      p.GetHeadAndCylinder().ToRegisters(r[1], r[2]);
      r[3] = p.Sector();
      r[4] = p.MultiSectorCount();
      return RegisterValues<NUMREGS> (r, true);
    };

  private:
    uint8_t drivenr_;
    HeadAndCylinderParamHelper headAndCylinder_;
    uint8_t sector_;
    uint8_t multiSectorCount_;
};



}