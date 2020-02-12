#include "Extreme3DProService.hpp"

using namespace JoystickLibrary;

constexpr int X_MIN = 0;
constexpr int X_MAX = 1023;
constexpr int Y_MIN = 0;
constexpr int Y_MAX = 1023;
constexpr int Z_MIN = 0;
constexpr int Z_MAX = 255;
constexpr int SLIDER_MIN = 255;
constexpr int SLIDER_MAX = 0;

Extreme3DProService::Extreme3DProService() : JoystickService()
{ 
}

Extreme3DProService::~Extreme3DProService()
{
}

void Extreme3DProService::OnDeviceChanged(DeviceStateChange ds)
{
    this->ProcessDeviceChange(EXTREME_3D_PRO_IDS, ds);
}

bool Extreme3DProService::GetX(int joystickID, int& x)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    x = NormalizeAxisValue(this->GetAxis(joystickID, ABS_X), X_MIN, X_MAX);
    return true;
}

bool Extreme3DProService::GetY(int joystickID, int& y)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    y =  -NormalizeAxisValue(this->GetAxis(joystickID, ABS_Y), Y_MIN, Y_MAX);
    return true;
}

bool Extreme3DProService::GetZRot(int joystickID, int& zRot)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    zRot = NormalizeAxisValue(this->GetAxis(joystickID, ABS_RZ), Z_MIN, Z_MAX);
    return true;
}

bool Extreme3DProService::GetSlider(int joystickID, int& slider)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    slider = 100 + (int) (((100.0 / (SLIDER_MAX - SLIDER_MIN)) * (this->GetAxis(joystickID, ABS_THROTTLE))));
    return true;
}

bool Extreme3DProService::GetButton(int joystickID, Extreme3DProButton button, bool& buttonVal)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    buttonVal = !!this->GetState(joystickID).buttons[BTN_TRIGGER + static_cast<int>(button)];
    return true;
}

bool Extreme3DProService::GetButtons(int joystickID, std::map<Extreme3DProButton, bool>& buttons)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    for (auto& pair : this->GetState(joystickID).buttons)
        buttons[static_cast<Extreme3DProButton>(pair.first)] = !!pair.second;
    return true;
}

bool Extreme3DProService::GetPOV(int joystickID, POV& pov)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    POV horizontal, vertical;
    switch (this->GetAxis(joystickID, ABS_HAT0X))
    {
        case -1:
            horizontal = POV::POV_WEST;
            break;
        case 1:
            horizontal = POV::POV_EAST;
            break;
        default:
            horizontal = POV::POV_NONE;
            break;
    }

    switch (this->GetAxis(joystickID, ABS_HAT0Y))
    {
        case -1:
            vertical = POV::POV_NORTH;
            break;
        case 1:
            vertical = POV::POV_SOUTH;
            break;
        default:
            vertical = POV::POV_NONE;
            break;
    }

    pov = static_cast<POV>(static_cast<int>(vertical) | static_cast<int>(horizontal));
    return true;
}