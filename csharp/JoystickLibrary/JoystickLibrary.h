#pragma once

#include <dinput.h>
#include <windows.h>
#include <msclr\lock.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#define NUMBER_BUTTONS 12

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Threading;

namespace JoystickLibrary
{
    enum class POV : int
    {
        POV_NONE = 0,
        POV_WEST = 1 << 0,
        POV_EAST = 1 << 1,
        POV_NORTH = 1 << 2,
        POV_SOUTH = 1 << 3,
        POV_NORTHWEST = POV_NORTH | POV_WEST,
        POV_NORTHEAST = POV_NORTH | POV_EAST,
        POV_SOUTHWEST = POV_SOUTH | POV_WEST,
        POV_SOUTHEAST = POV_SOUTH | POV_EAST
    };

    enum class Button : int
    {
        Trigger = 0,
        Button2 = 1,
        Button3 = 2,
        Button4 = 3,
        Button5 = 4,
        Button6 = 5,
        Button7 = 6,
        Button8 = 7,
        Button9 = 8,
        Button10 = 9,
        Button11 = 10,
        Button12 = 11
    };

    ref class JoystickService
    {
    public:
        JoystickService(int number_joysticks);
        ~JoystickService(void);

        bool Initialize(void);
        List<int>^ GetJoystickIDs(void);
        int GetConnectedJoysticksCount(void);
        bool Start(void);
        bool GetX(int joystickID, int& x);
        bool GetY(int joystickID, int& y);
        bool GetZRot(int joystickID, int& zRot);
        bool GetSlider(int joystickID, int& slider);
        bool GetButtons(int joystickID, array<bool>^& buttons);
        bool GetPOV(int joystickID, POV& pov);
        bool RemoveJoystick(int joystickID);

    private:
        void PollJoysticks(void);
        void LocateJoysticks(void);
        bool IsValidJoystickID(int);

        Thread^ jsPoller;
        Object^ m_lock;

        bool jsPollerStop;
        bool initialized;
    };
}