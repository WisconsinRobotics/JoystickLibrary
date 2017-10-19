#pragma once

#include "JoystickService.hpp"

namespace JoystickLibrary
{
#ifdef _WIN32
    enum class Xbox360Button : int
    {
        A = 0,
        B = 1,
        X = 2,
        Y = 3,
        LB = 4,
        RB = 5,
        Back = 6,
        Start = 7,
        LeftThumbstick = 8,
        RightThumbstick = 9
    };
#else
    enum class Xbox360Button : int
    {
        A = BTN_SOUTH,
        B = BTN_EAST,
        X = BTN_NORTH,
        Y = BTN_WEST,
        LB = BTN_TR,
        RB = BTN_TL,
        Back = BTN_SELECT,
        Start = BTN_START,
        LeftThumbstick = BTN_THUMBL,
        RightThumbstick = BTN_THUMBR
    };
#endif

    class Xbox360Service : public JoystickService
    {
    public:
        const std::vector<JoystickDescriptor> XBOX_IDS {
            { 0x045E, 0x028E },	// Microsoft Xbox 360 Controller
			{ 0x0E6F, 0x0401 },	// Microsoft Xbox 360 Controller
            { 0x045E, 0x0291 },	// Microsoft Xbox 360 Wireless Controller
            { 0x0E6F, 0x0213 }	// Afterglow AX.1 for Xbox 360
        };

        const int NUMBER_BUTTONS = 11;

        static Xbox360Service& GetInstance()
        {
            static Xbox360Service instance;
            return instance;
        }

        Xbox360Service(Xbox360Service const&) = delete;
        void operator=(Xbox360Service const&) = delete;
        ~Xbox360Service();

        bool GetLeftX(int joystickID, int& leftX);
        bool GetLeftY(int joystickID, int& leftY);
        bool GetRightX(int joystickID, int& rightX);
        bool GetRightY(int joystickID, int& rightY);
        bool GetLeftTrigger(int joystickID, int& leftTrigger);
        bool GetRightTrigger(int joystickID, int& rightTrigger);
        bool GetDpad(int joystickID, POV& dpad);
        bool GetButton(int joystickID, Xbox360Button button, bool& buttonVal);
        bool GetButtons(int joystickID, std::map<Xbox360Button, bool>& buttons);

    protected:
        void OnDeviceChanged(DeviceStateChange ds);

    private:
        Xbox360Service();
    };
}



