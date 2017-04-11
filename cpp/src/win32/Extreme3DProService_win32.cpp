#include "Extreme3DProService.hpp"

using namespace JoystickLibrary;


Extreme3DProService::Extreme3DProService()
{
}

Extreme3DProService::~Extreme3DProService()
{
}

bool Extreme3DProService::GetX(int joystickID, int& x)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    x = this->GetState(joystickID).lX;
    return true;
}

bool Extreme3DProService::GetY(int joystickID, int& y)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    y = -this->GetState(joystickID).lY;
    return true;
}

bool Extreme3DProService::GetZRot(int joystickID, int& zRot)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    zRot = this->GetState(joystickID).lRz;
    return true;
}

bool Extreme3DProService::GetSlider(int joystickID, int& slider)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    slider = (100 - this->GetState(joystickID).rglSlider[0]) / 2;
    return true;
}

bool Extreme3DProService::GetButton(int joystickID, Extreme3DProButton button, bool& buttonVal)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    buttonVal = !!this->GetState(joystickID).rgbButtons[static_cast<int>(button)];
    return true;
}

bool Extreme3DProService::GetButtons(int joystickID, std::map<Extreme3DProButton, bool>& buttons)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    buttons.clear();
    for (int i = 0; i < NUMBER_BUTTONS; i++)
        buttons[static_cast<Extreme3DProButton>(i)] = !!this->GetState(joystickID).rgbButtons[i];
    return true;
}

bool Extreme3DProService::GetPOV(int joystickID, POV& pov)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    // POV values are done in 45 deg segments (0 is up) * 100.
    // i.e. straight right = 90 deg = 9000.
    // determine POV value by way of lookup table (divide by 4500)
    unsigned int povListIndex = this->GetState(joystickID).rgdwPOV[0] / 4500;
    pov = (povListIndex < povList.size()) ? povList[povListIndex] : POV::POV_NONE;
    return true;
}

void Extreme3DProService::OnDeviceChanged(DeviceStateChange ds)
{
    this->ProcessDeviceChange(EXTREME_3D_PRO_IDS, ds);
}
