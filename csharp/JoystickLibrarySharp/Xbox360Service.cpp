#include "JoystickLibrary.hpp"

using namespace JoystickLibrarySharp;

Xbox360Service::Xbox360Service()
{
}

Xbox360Service::~Xbox360Service()
{
}

bool Xbox360Service::Initialize()
{
    return this->xs.Initialize();
}

bool Xbox360Service::GetLeftX(int joystickID, System::Int32% leftX)
{
    int val = 0;
    bool success = this->xs.GetLeftX(joystickID, val);
    leftX = val;
    return success;
}

bool Xbox360Service::GetLeftY(int joystickID, System::Int32% leftY)
{
    int val = 0;
    bool success = this->xs.GetLeftY(joystickID, val);
    leftY = val;
    return success;
}

bool Xbox360Service::GetRightX(int joystickID, System::Int32% rightX)
{
    int val = 0;
    bool success = this->xs.GetRightX(joystickID, val);
    rightX = val;
    return success;
}

bool Xbox360Service::GetRightY(int joystickID, System::Int32% rightY)
{
    int val = 0;
    bool success = this->xs.GetRightY(joystickID, val);
    rightY = val;
    return success;
}

bool Xbox360Service::GetLeftTrigger(int joystickID, System::Int32% leftTrigger)
{
    int val = 0;
    bool success = this->xs.GetLeftTrigger(joystickID, val);
    leftTrigger = val;
    return success;
}

bool Xbox360Service::GetRightTrigger(int joystickID, System::Int32% rightTrigger)
{
    int val = 0;
    bool success = this->xs.GetRightTrigger(joystickID, val);
    rightTrigger = val;
    return success;
}

bool Xbox360Service::GetButton(int joystickID, Xbox360Button button, System::Boolean% buttonVal)
{
    bool pressed = false;
    bool success = this->xs.GetButton(joystickID, (JoystickLibrary::Xbox360Button) button, pressed);
    buttonVal = pressed;
    return success;
}

bool Xbox360Service::GetButtons(int joystickID, array<bool>^% buttons)
{
    std::map<JoystickLibrary::Xbox360Button, bool> btn_map;
    bool success = this->xs.GetButtons(joystickID, btn_map);
    buttons = gcnew array<bool>(static_cast<int>(btn_map.size()));
    for (auto& pair : btn_map)
    {
        buttons[(int) pair.first] = pair.second;
    }
    return success;
}

System::Collections::Generic::List<int>^ JoystickLibrarySharp::Xbox360Service::GetIDs()
{
    std::vector<int> ids = this->xs.GetIDs();
    System::Collections::Generic::List<int>^ list = gcnew System::Collections::Generic::List<int>();
    for (int i = 0; i < ids.size(); i++)
        list->Add(ids[i]);
    return list;
}

bool Xbox360Service::GetDpad(int joystickID, POV% dpad)
{
    JoystickLibrary::POV pov = JoystickLibrary::POV::POV_NONE;
    bool success = this->xs.GetDpad(joystickID, pov);
    dpad = (POV)((int)pov);
    return success;
}

System::Int32 Xbox360Service::GetNumberConnected()
{
    return this->xs.GetNumberConnected();
}
