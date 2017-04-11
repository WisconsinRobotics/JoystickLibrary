#pragma once

#include <Xbox360Service.hpp>
#include <Extreme3DProService.hpp>

namespace JoystickLibrarySharp
{
    public enum class POV : int
    {
        POV_NONE = 0,                               /* POV hat is not in use.              */
        POV_WEST = 1 << 0,                          /* POV hat is facing left.             */
        POV_EAST = 1 << 1,                          /* POV hat is facing right.            */
        POV_NORTH = 1 << 2,                         /* POV hat is facing up.               */
        POV_SOUTH = 1 << 3,                         /* POV hat is facing down.             */
        POV_NORTHWEST = POV_NORTH | POV_WEST,       /* POV hat is facing up and left.      */
        POV_NORTHEAST = POV_NORTH | POV_EAST,       /* POV hat is facing up and right.     */
        POV_SOUTHWEST = POV_SOUTH | POV_WEST,       /* POV hat is facing south and left.   */
        POV_SOUTHEAST = POV_SOUTH | POV_EAST        /* POV hat is facing south and right.  */
    };

    public enum class Extreme3DProButton : int
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

    public ref class Extreme3DProService
    {
    public:
        Extreme3DProService();
        ~Extreme3DProService();
        bool Initialize();
        bool GetX(int joystickID, System::Int32% x);
        bool GetY(int joystickID, System::Int32% y);
        bool GetZRot(int joystickID, System::Int32% zRot);
        bool GetSlider(int joystickID, System::Int32% slider);
        bool GetButton(int joystickID, Extreme3DProButton button, System::Boolean% buttonVal);
        bool GetButtons(int joystickID, array<bool>^% buttons);
        bool GetPOV(int joystickID, POV% pov);
        System::Collections::Generic::List<int>^ GetIDs();
        System::Int32 Extreme3DProService::GetNumberConnected();

    private:
        JoystickLibrary::Extreme3DProService& es = JoystickLibrary::Extreme3DProService::GetInstance();
    };

    public enum class Xbox360Button : int
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

    public ref class Xbox360Service
    {
    public:
        Xbox360Service();
        ~Xbox360Service();

        bool Initialize();
        bool GetLeftX(int joystickID, System::Int32% leftX);
        bool GetLeftY(int joystickID, System::Int32% leftY);
        bool GetRightX(int joystickID, System::Int32% rightX);
        bool GetRightY(int joystickID, System::Int32% rightY);
        bool GetLeftTrigger(int joystickID, System::Int32% leftTrigger);
        bool GetRightTrigger(int joystickID, System::Int32% rightTrigger);
        bool GetDpad(int joystickID, POV% dpad);
        bool GetButton(int joystickID, Xbox360Button button, System::Boolean% buttonVal);
        bool GetButtons(int joystickID, array<bool>^% buttons);
        System::Collections::Generic::List<int>^ GetIDs();
        System::Int32 GetNumberConnected();

    private:
        JoystickLibrary::Xbox360Service& xs = JoystickLibrary::Xbox360Service::GetInstance();
    };
}