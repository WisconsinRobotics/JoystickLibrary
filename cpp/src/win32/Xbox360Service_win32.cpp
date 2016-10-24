#include "joystick.h"

using namespace JoystickLibrary;

constexpr int XBOX360_VENDOR_ID = 0x045E;
constexpr int XBOX360_PRODUCT_ID = 0x028E;
constexpr int NUMBER_BUTTONS = 11;


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

    leftX = this->GetState(joystickID).lX;
    return true;
}

bool Xbox360Service::GetLeftY(int joystickID, int& leftY)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    leftY = -this->GetState(joystickID).lY;
    return true;
}

bool Xbox360Service::GetRightX(int joystickID, int& rightX)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    rightX = this->GetState(joystickID).lRx;
    return true;
}

bool Xbox360Service::GetRightY(int joystickID, int& rightY)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    rightY = -this->GetState(joystickID).lRy;
    return true;
}

bool Xbox360Service::GetLeftTrigger(int joystickID, int& leftTrigger)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    // triggers share the lZ field. left: [0, 100], right: [0, -100]    
    int lz = this->GetState(joystickID).lZ;
    leftTrigger = (lz > 0) ? lz : 0;
    return true;
}

bool Xbox360Service::GetRightTrigger(int joystickID, int& rightTrigger)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    // triggers share the lZ field. left: [0, 100], right: [0, -100]
    int lz = this->GetState(joystickID).lZ;
    rightTrigger = (lz < 0) ? -lz : 0;
    return true;
}

bool Xbox360Service::GetDpad(int joystickID, POV& dpad)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    // POV values are done in 45 deg segments (0 is up) * 100.
    // i.e. straight right = 90 deg = 9000.
    // determine POV value by way of lookup table (divide by 4500)
    unsigned int povListIndex = jsMap[joystickID].state.rgdwPOV[0] / 4500;
    dpad = (povListIndex < povList.size()) ? povList[povListIndex] : POV::POV_NONE;
    return true;
}

bool Xbox360Service::GetButton(int joystickID, Xbox360Button button, bool& buttonVal)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    buttonVal = !!this->GetState(joystickID).rgbButtons[static_cast<int>(button)];
    return true;
}

bool Xbox360Service::GetButtons(int joystickID, std::map<Xbox360Button, bool>& buttons)
{
    if (!IsValidJoystickID(joystickID))
        return false;

    buttons.clear();
    for (int i = 0; i < NUMBER_BUTTONS; i++)
        buttons[static_cast<Xbox360Button>(i)] = !!this->GetState(joystickID).rgbButtons[i];

    return true;
}