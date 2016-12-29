#include "JoystickLibrary.h"

using namespace JoystickLibrary;

constexpr int NUMBER_BUTTONS = 11;

Xbox360Service::Xbox360Service(int number_joysticks) : JoystickService(number_joysticks)
{
    this->valid_devices->push_back({ 0x045E, 0x028E });	// Microsoft Xbox 360 Controller
    this->valid_devices->push_back({ 0x045E, 0x0291 });	// Microsoft Xbox 360 Wireless Controller
    this->valid_devices->push_back({ 0x0E6F, 0x0213 });	// Afterglow AX.1 for Xbox 360
}

Xbox360Service::~Xbox360Service()
{
}

bool Xbox360Service::GetLeftX(int joystickID, Int32% leftX)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    leftX = this->GetState(joystickID).lX;
    return true;
}

bool Xbox360Service::GetLeftY(int joystickID, Int32% leftY)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    leftY = -this->GetState(joystickID).lY;
    return true;
}

bool Xbox360Service::GetRightX(int joystickID, Int32% rightX)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    rightX = this->GetState(joystickID).lRx;
    return true;
}

bool Xbox360Service::GetRightY(int joystickID, Int32% rightY)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    rightY = -this->GetState(joystickID).lRy;
    return true;
}

bool Xbox360Service::GetLeftTrigger(int joystickID, Int32% leftTrigger)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    // triggers share the lZ field. left: [0, 100], right: [0, -100]    
    int lz = this->GetState(joystickID).lZ;
    leftTrigger = (lz > 0) ? lz : 0;
    return true;
}

bool Xbox360Service::GetRightTrigger(int joystickID, Int32% rightTrigger)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    // triggers share the lZ field. left: [0, 100], right: [0, -100]
    int lz = this->GetState(joystickID).lZ;
    rightTrigger = (lz < 0) ? -lz : 0;
    return true;
}

bool Xbox360Service::GetButton(int joystickID, Xbox360Button button, Boolean% buttonVal)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    buttonVal = !!this->GetState(joystickID).rgbButtons[static_cast<int>(button)];
    return true;
}

bool Xbox360Service::GetButtons(int joystickID, array<bool>^% buttons)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    buttons = gcnew array<bool>(NUMBER_BUTTONS);
    for (int i = 0; i < NUMBER_BUTTONS; i++)
        buttons[static_cast<int>(i)] = !!this->GetState(joystickID).rgbButtons[i];
    return true;
}

bool Xbox360Service::GetDpad(int joystickID, POV% dpad)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    int povListIndex = this->GetState(joystickID).rgdwPOV[0] / 4500;
    dpad = (povListIndex < povList->Length) ? povList[povListIndex] : POV::POV_NONE;
    return true;
}
