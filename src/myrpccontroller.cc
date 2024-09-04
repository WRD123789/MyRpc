#include "myrpccontroller.h"

MyrpcController::MyrpcController()
{
    _failed = false;
    _errText = "";
}

void MyrpcController::Reset()
{
    _failed = false;
    _errText = "";
}

inline bool MyrpcController::Failed() const
{
    return _failed;
}

inline std::string MyrpcController::ErrorText() const
{
    return _errText;
}

void MyrpcController::SetFailed(const std::string &reason)
{
    _failed = true;
    _errText = reason;
}