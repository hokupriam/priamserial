#include "PriamSmartInterface.h"

using namespace Priam;

bool InterfaceStatus::DatabusEnabled()
{
  return (statusregval_ & DATABUSENABLE); 
}

bool InterfaceStatus::TransferRequest()
{
  return (statusregval_ & DATAXFERREQUEST);
}

bool InterfaceStatus::ReadRequest()
{
  return (TransferRequest() && (statusregval_ & READWRITEREQUEST));
}

bool InterfaceStatus::WriteRequest()
{
  return (TransferRequest() && !(statusregval_ & READWRITEREQUEST));
}

bool InterfaceStatus::Busy()
{
  return (statusregval_ & INTERFACEBUSY);
}

bool InterfaceStatus::CompletionRequest()
{
  return (statusregval_ & COMPLETIONREQUEST);
}

bool InterfaceStatus::CommandRejected()
{
  return (statusregval_ & COMMANDREJECT);
}

bool InterfaceStatus::ReadyForCommand()
{
    return (!TransferRequest() && !Busy() && !CompletionRequest() && DatabusEnabled());
}