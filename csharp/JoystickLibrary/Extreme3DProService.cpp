#include "JoystickLibrary.h"

using namespace JoystickLibrary;

constexpr int NUMBER_BUTTONS = 12;

Extreme3DProService::Extreme3DProService(int number_joysticks) : JoystickService(number_joysticks)
{
    this->valid_devices->push_back({ 0x46D, 0xC215 });
}

Extreme3DProService::~Extreme3DProService()
{
}

bool Extreme3DProService::GetX(int joystickID, Int32% x)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    x = this->GetState(joystickID).lX;
    return true;
}

bool Extreme3DProService::GetY(int joystickID, Int32% y)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    y = -this->GetState(joystickID).lY;
    return true;
}

bool Extreme3DProService::GetZRot(int joystickID, Int32% zRot)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    zRot = this->GetState(joystickID).lRz;
    return true;
}

bool Extreme3DProService::GetSlider(int joystickID, Int32% slider)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    slider = (100 - this->GetState(joystickID).rglSlider[0]) / 2;
    return true;
}

bool Extreme3DProService::GetButton(int joystickID, Extreme3DProButton button, Boolean% buttonVal)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    buttonVal = !!this->GetState(joystickID).rgbButtons[static_cast<int>(button)];
    return true;
}

bool Extreme3DProService::GetButtons(int joystickID, array<bool>^% buttons)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    buttons = gcnew array<bool>(NUMBER_BUTTONS);
    for (int i = 0; i < NUMBER_BUTTONS; i++)
        buttons[static_cast<int>(i)] = !!this->GetState(joystickID).rgbButtons[i];
    return true;
}

bool Extreme3DProService::GetPOV(int joystickID, POV% pov)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    int povListIndex = this->GetState(joystickID).rgdwPOV[0] / 4500;
    pov = (povListIndex < povList->Length) ? povList[povListIndex] : POV::POV_NONE;
    return true;
}
