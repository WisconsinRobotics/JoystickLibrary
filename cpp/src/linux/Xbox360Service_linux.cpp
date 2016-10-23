#include "joystick.h"

using namespace JoystickLibrary;

constexpr int XBOX360_VENDOR_ID = 0x045E;
constexpr int XBOX360_PRODUCT_ID = 0x028E;
constexpr int NUMBER_BUTTONS = 11;
constexpr int X_MIN = 0;
constexpr int X_MAX = 1023;
constexpr int Y_MIN = 0;
constexpr int Y_MAX = 1023;
constexpr int TRIGGER_MIN = 0;
constexpr int TRIGGER_MAX = 255;

Xbox360Service::Xbox360Service(int number_joysticks) : JoystickService(number_joysticks)
{
    this->product_id = XBOX360_PRODUCT_ID;
    this->vendor_id = XBOX360_VENDOR_ID;
}

Xbox360Service::~Xbox360Service()
{
}

bool Xbox360Service::GetLeftX(int joystickID, int& leftX)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    leftX = NormalizeAxisValue(this->GetState(joystickID).axes[ABS_X], X_MIN, X_MAX);
    return true;
}

bool Xbox360Service::GetLeftY(int joystickID, int& leftY)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    leftY = -NormalizeAxisValue(this->GetState(joystickID).axes[ABS_Y], Y_MIN, Y_MAX);
    return true;
}

bool Xbox360Service::GetRightX(int joystickID, int& rightX)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    rightX = NormalizeAxisValue(this->GetState(joystickID).axes[ABS_RX], X_MIN, X_MAX);
    return true;
}

bool Xbox360Service::GetRightY(int joystickID, int& rightY)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    rightY = -NormalizeAxisValue(this->GetState(joystickID).axes[ABS_RY], Y_MIN, Y_MAX);
    return true;
}

bool Xbox360Service::GetLeftTrigger(int joystickID, int& leftTrigger)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    leftTrigger = NormalizeAxisValue(this->GetState(joystickID).axes[ABS_Z], TRIGGER_MIN, TRIGGER_MAX);
    return true;
}

bool Xbox360Service::GetRightTrigger(int joystickID, int& rightTrigger)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    rightTrigger = NormalizeAxisValue(this->GetState(joystickID).axes[ABS_RZ], TRIGGER_MIN, TRIGGER_MAX);
    return true;
}

bool Xbox360Service::GetDpad(int joystickID, POV& dpad)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    POV horizontal, vertical;
    switch (this->GetState(joystickID).axes[ABS_HAT0X])
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

    switch (this->GetState(joystickID).axes[ABS_HAT0Y])
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

    dpad = static_cast<POV>(static_cast<int>(vertical) | static_cast<int>(horizontal));
    return true;
}

bool Xbox360Service::GetButton(int joystickID, Xbox360Button button, bool& buttonVal)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    buttonVal = this->GetState(joystickID).buttons[static_cast<int>(button)];
    return true;
}

bool Xbox360Service::GetButtons(int joystickID, std::map<Xbox360Button, bool>& buttons)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    for (auto& pair : this->GetState(joystickID).buttons)
        buttons[static_cast<Xbox360Button>(pair.first)] = !!pair.second;
        
    return true;
}
