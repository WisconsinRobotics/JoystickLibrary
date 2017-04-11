#pragma once

#include "JoystickService.hpp"

namespace JoystickLibrary
{
    enum class Extreme3DProButton : int
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

    class Extreme3DProService : public JoystickService
    {
    public:
        const std::vector<JoystickDescriptor> EXTREME_3D_PRO_IDS {
            { 0x46D, 0xC215 }
        };

        const int NUMBER_BUTTONS = 12;

        static Extreme3DProService& GetInstance()
        {
            static Extreme3DProService instance;
            return instance;
        }

        Extreme3DProService();
        Extreme3DProService(Extreme3DProService const&) = delete;
        void operator=(Extreme3DProService const&) = delete;
        ~Extreme3DProService();

        /**
        * Gets the X axis value of the specified joystick ID.
        * Pulling to the left constitutes as negative with a min value of -100.
        * Pulling to the right is interpreted as positive with a max value of +100.
        * @param joystickID the joystick ID
        * @param x A reference in which to save the value. Will not be modified if call fails.
        * @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetX(int joystickID, int& x);

        /**
        * Gets the Y axis value of the specified joystick ID.
        * Pushing forwards (away from you) constitutes as positive with a max value of +100.
        * Pushing backwards (towards you) constitutes as negative with a min value of -100.
        * @param joystickID the joystick ID
        * @param y A reference in which to save the value. Will not be modified if call fails.
        * @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetY(int joystickID, int& y);

        /**
        * Gets the Z rotation axis value of the specified joystick ID.
        * Twisting counter clockwise constitutes as negative, with a min value of -100.
        * Twisting clockwise constitutes as positive, with a max value of +100.
        * @param joystickID the joystick ID
        * @param zRot A reference in which to save the value. Will not be modified if call fails.
        * @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetZRot(int joystickID, int& zRot);

        /**
        * Gets the slider value of the specified joystick ID.
        * The '-' marking on the slider is 0, and the "+" marking is 100.
        * @param joystickID the joystick ID
        * @param slider A reference in which to save the value. Will not be modified if call fails.
        * @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetSlider(int joystickID, int& slider);

        /**
        * Gets the button state of the specified joystick ID and button.
        * @param joystickID the joystick ID
        * @param button the specific button to query
        * @param buttonVal A reference in which to save the value. Will not be modified if call fails.
        * @return false if invalid joystickID, disconnected joystick, or button not pressed, true otherwise.
        */
        bool GetButton(int joystickID, Extreme3DProButton button, bool& buttonVal);

        /**
        * Gets all of the button states of the specified joystick ID.
        * @param joystickID the joystick ID
        * @param buttons A reference in which to save the value. Will not be modified if call fails.
        * @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetButtons(int joystickID, std::map<Extreme3DProButton, bool>& buttons);

        /**
        * Gets the POV hat value of the specified joystick ID.
        * @param joystickID the joystick ID
        * @param pov A reference in which to save the value. Will not be modified if call fails.
        * @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetPOV(int joystickID, POV& pov);

    protected:
        void OnDeviceChanged(DeviceStateChange ds);
    };
}

