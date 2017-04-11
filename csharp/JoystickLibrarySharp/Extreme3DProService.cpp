#include "JoystickLibrary.hpp"

using namespace JoystickLibrarySharp;
using System::Int32;


Extreme3DProService::Extreme3DProService()
{
}

Extreme3DProService::~Extreme3DProService()
{
}

bool Extreme3DProService::Initialize()
{
    return this->es.Initialize();
}

bool Extreme3DProService::GetX(int joystickID, Int32% x)
{
    int val = 0;
    bool success = this->es.GetX(joystickID, val);
    x = val;
    return success;
}

bool Extreme3DProService::GetY(int joystickID, Int32% y)
{
    int val = 0;
    bool success = this->es.GetY(joystickID, val);
    y = val;
    return success;
}

bool Extreme3DProService::GetZRot(int joystickID, Int32% zRot)
{
    int val = 0;
    bool success = this->es.GetZRot(joystickID, val);
    zRot = val;
    return success;
}

bool Extreme3DProService::GetSlider(int joystickID, Int32% slider)
{
    int val = 0;
    bool success = this->es.GetSlider(joystickID, val);
    slider = val;
    return success;
}

bool Extreme3DProService::GetButton(int joystickID, Extreme3DProButton button, System::Boolean% buttonVal)
{
    bool pressed = false;
    bool success = this->es.GetButton(joystickID, (JoystickLibrary::Extreme3DProButton) button, pressed);
    buttonVal = pressed;
    return success;
}

bool Extreme3DProService::GetButtons(int joystickID, array<bool>^% buttons)
{
    std::map<JoystickLibrary::Extreme3DProButton, bool> btn_map;
    bool success = this->es.GetButtons(joystickID, btn_map);
    buttons = gcnew array<bool>(static_cast<int>(btn_map.size()));
    for (auto& pair : btn_map)
    {
        buttons[(int)pair.first] = pair.second;
    }
    return success;
}

bool Extreme3DProService::GetPOV(int joystickID, POV% pov)
{
    JoystickLibrary::POV _pov = JoystickLibrary::POV::POV_NONE;
    bool success = this->es.GetPOV(joystickID, _pov);
    pov = (POV)((int)_pov);
    return success;
}

System::Collections::Generic::List<int>^ Extreme3DProService::GetIDs()
{
    std::vector<int> ids = this->es.GetIDs();
    System::Collections::Generic::List<int>^ list = gcnew System::Collections::Generic::List<int>();
    for (int i = 0; i < ids.size(); i++)
        list->Add(ids[i]);
    return list;
}

System::Int32 Extreme3DProService::GetNumberConnected()
{
    return this->es.GetNumberConnected();
}
