#include "joystick.h"

using namespace JoystickLibrary;

JoystickService::JoystickService(int number_joysticks)
{
    this->requestedJoysticks = number_joysticks;
    this->connectedJoysticks = 0;
    this->jsPollerStop = false;
    this->initialized = false;
    this->nextJoystickID = 1;
}

void JoystickService::GetJoystickIDs(std::vector<int>& list)
{
    for (auto pair : this->jsMap)
        if (pair.second.alive)
            list.push_back(pair.first);
}

int JoystickService::GetConnectedJoysticksCount(void)
{
    return this->connectedJoysticks;
}

bool JoystickService::Start(void)
{
    if (!this->initialized)
        return false;
    
    jsPollerStop = false;
    jsPoller = std::thread(&JoystickService::PollJoysticks, this);
    return true;
}

bool JoystickService::GetX(int joystickID, int& x)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;
    
    x = jsMap[joystickID].x;
    return true;
}

bool JoystickService::GetY(int joystickID, int& y)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    y = jsMap[joystickID].y;
    return true;
}

bool JoystickService::GetZRot(int joystickID, int& zRot)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    zRot = jsMap[joystickID].rz;    
    return true;
}

bool JoystickService::GetSlider(int joystickID, int& slider)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;  

    slider = jsMap[joystickID].slider;
    return true;    
}

bool JoystickService::GetButtons(int joystickID, std::array<bool, NUMBER_BUTTONS>& buttons)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    buttons = jsMap[joystickID].buttons;
    return true;
}

bool JoystickService::GetPOV(int joystickID, POV& pov)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;  

    pov = jsMap[joystickID].pov;    
    return true;
}

bool JoystickService::IsValidJoystickID(int joystickID)
{
    return (this->jsMap.find(joystickID) != this->jsMap.end()) 
                && this->jsMap[joystickID].alive;
}