#pragma once
#include "PriamSmartInterface.h"
using namespace Priam;

//Definition of a Priam Smart command, templated on command byte value, parameter and result type
//PARAMS must have a static method MakeRegs(PARAMS &p) returning a RegisterValues object with the physical
//register values to be passed to the interface
//RESULTS must have a static method ParseStatus that converts a RegisterValues object into a status object
//Both PARAMS and RESULTS must have a static const member NUMREGS indicating how many Interface registers are
//associated with the parameters and status respectively
template <uint8_t CMDCODEVAL, typename PARAMS, typename RESULTS>
class CommandDefinition
{
  public:
  
  CommandDefinition() : cmdInfo_(CMDCODEVAL) {};
  
  CommandInfo<PARAMS::NUMREGS, RESULTS::NUMREGS> GetCommandInfo() {return cmdInfo_;}
  
  //If the transaction failed, set RESULT to invalid
  //This indicates a comms error with the interface
  RESULTS Execute(PriamSmart &interface, PARAMS &parameter)
  {
    RegisterValues<RESULTS::NUMREGS> resRegs = 
    interface.TransactNew(cmdInfo_, PARAMS::MakeRegs(parameter));
    return RESULTS::ParseStatus(resRegs) ;  
  }

  private:
  CommandInfo<PARAMS::NUMREGS, RESULTS::NUMREGS> cmdInfo_;
  
};

//The following typedefs define class types for each of the actual command: byte value, parameter type and result type
typedef CommandDefinition<PriamCommandsByteValues::SEQUENCEUPANDWAIT, DriveParam, TransactionStatus> DriveCmd_SpinupAndWait; 
typedef CommandDefinition<PriamCommandsByteValues::SEQUENCEDOWN, DriveParam, TransactionStatus> DriveCmd_SpinDown; 

typedef CommandDefinition<PriamCommandsByteValues::READDRIVEPARAM, DriveParam, ResultDriveParams> DriveCmd_ReadParams; 

typedef CommandDefinition<PriamCommandsByteValues::SEEKWITHRETRY, SeekParam, ResultCylinder> DriveCmd_SeekWithRetry; 

typedef CommandDefinition<PriamCommandsByteValues::VERIFYDISK, DriveParam, ResultHeadCylinderSector> DriveCmd_VerifyDisk; 

typedef CommandDefinition<PriamCommandsByteValues::READDATAWITHRETRY, DiskReadParam, TransactionStatus> DriveCmd_ReadDataWithRetry; 

typedef CommandDefinition<PriamCommandsByteValues::READDATANORETRY, DiskReadParam, TransactionStatus> DriveCmd_ReadDataNoRetry; 

