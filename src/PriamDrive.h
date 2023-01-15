#pragma once
#include "PriamSmartInterface.h"
#include "PriamSmartCommand.h"
#include "PriamSmartCommandResult.h"
#include "PriamHighlevelCommands.h"
//High level class for "drive" object

using namespace Priam;

class PriamDrive
{
    public:
    PriamDrive(PriamSmart &interface) : interface_(interface) {};

    TransactionStatus SpinupWait(uint8_t driveno)
    {
        DriveParam drive(driveno);
        DriveCmd_SpinupAndWait hlcmd;
        TransactionStatus st = hlcmd.Execute(interface_, drive);   
        return st;
    }

    TransactionStatus SpinDown(uint8_t driveno)
    {
        DriveParam drive(driveno);
        DriveCmd_SpinDown hlcmd;
        TransactionStatus st = hlcmd.Execute(interface_, drive);   
        return st;
    }

    ResultDriveParams ReadParams(uint8_t driveno)
    {
        DriveParam drive(driveno);
        DriveCmd_ReadParams rdpCmd;
        ResultDriveParams res = rdpCmd.Execute(interface_, drive);
        return res;
    }

    ResultCylinder Seek(uint8_t driveno, uint8_t head, uint16_t cylinder, bool withRetry = true)
    {
        SeekParam seekP(driveno, head, cylinder);

        if (withRetry)
        {
            DriveCmd_SeekWithRetry cmdSeekRetry;
            ResultCylinder res = cmdSeekRetry.Execute(interface_, seekP);
            return res;
        }
        else
        {
            DriveCmd_SeekWithRetry cmdSeekNoRetry;
            ResultCylinder res = cmdSeekNoRetry.Execute(interface_, seekP);
            return res;
        }
        
    }

    ResultHeadCylinderSector VerifyDisk(uint8_t driveno)
    {
        DriveParam drive(driveno);
        DriveCmd_VerifyDisk vrfDiskCmd;
        ResultHeadCylinderSector res = vrfDiskCmd.Execute(interface_, drive);
        return res;
    }

    TransactionStatus ReadData(uint8_t driveno, uint8_t head, uint16_t cylinder, uint8_t sector, uint8_t multiSectorCount, bool withRetry = true)
    {
        DiskReadParam readParams(driveno, head, cylinder, sector, multiSectorCount);

        if (withRetry)
        {
            DriveCmd_ReadDataWithRetry readRetry;
            TransactionStatus res = readRetry.Execute(interface_, readParams);
            return res;
        }
        else
        {
            DriveCmd_ReadDataNoRetry readNoRetry;
            TransactionStatus res = readNoRetry.Execute(interface_, readParams);
            return res;
        }
    }

    private:
    PriamSmart & interface_;
};
