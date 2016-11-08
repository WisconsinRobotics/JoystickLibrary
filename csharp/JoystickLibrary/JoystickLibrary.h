/*
* Copyright (c) 2016, Wisconsin Robotics
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the distribution.
* * Neither the name of Wisconsin Robotics nor the
*   names of its contributors may be used to endorse or promote products
*   derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL WISCONSIN ROBOTICS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <map>
#include <dinput.h>
#include <windows.h>
#include <msclr\lock.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


using namespace System;
using namespace System::Collections::Generic;
using namespace System::Threading;

namespace JoystickLibrary
{
    /**
      Represents the point of view (POV) hat on the joystick.
     */
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

    struct JoystickData
    {
        bool alive;
        DIJOYSTATE state;
        LPDIRECTINPUTDEVICE8 os_obj;
    };

    /*
     The JoystickService class looks for, initializes, configures joysticks
     and exposes its state to the user.
     */
    public ref class JoystickService abstract
    {
    public:

        /**
         Constructs a new JoystickService instance with the specified number of joysticks.
         @param number_joysticks Number of joysticks to enumerate
         */
        JoystickService(int number_joysticks);
        ~JoystickService();

        /**
         Initializes the JoystickService object.
         This entails OS specific initialization, such as DirectInput bringup on Windows.
         @return true on success, false otherwise
         */
        bool Initialize();

        /**
         Gets all connected joystick IDs.
         @return A list containing all connected joystick IDs.
         */
        List<int>^ GetJoystickIDs();

        /**
         Gets the number of connected joysticks.
         @return The number of connected joysticks.
         */
        int GetConnectedJoysticksCount();

        /**
         Starts the polling/enumeration thread.
         @return true on success, false otherwise
         */
        bool Start();

        /**
          Removes the specified joystick ID.
          @param joystickID the joystick ID
          @return false if invalid joystickID, true otherwise.
         */
        bool RemoveJoystick(int joystickID);
        
    protected:

        static array<POV>^ povList = {
            POV::POV_NORTH,
            POV::POV_NORTHEAST,
            POV::POV_EAST,
            POV::POV_SOUTHEAST,
            POV::POV_SOUTH,
            POV::POV_SOUTHWEST,
            POV::POV_WEST,
            POV::POV_NORTHWEST,
        };

        /**
          Main polling and enumeration loop.
         */
        void PollJoysticks();

        /**
          Locates, enumerates, and configures connected joysticks.
         */
        void LocateJoysticks();

        /**
          Helper function to check if a joystickID is valid and whether or not
          the joystick is connected.
          @param joystickID the joystick ID to check
          @return true if connected and valid, false otherwise
         */
        bool IsValidJoystickID(int);

        DIJOYSTATE GetState(int joystickID);

        Thread^ jsPoller;
        Object^ m_lock;

        LPDIRECTINPUT8 di;
        std::map<int, JoystickData> *jsMap;
        System::Int32 requestedJoysticks;
        System::Int32 connectedJoysticks;
        System::Int32 nextJoystickID;
        System::Int32 product_id;
        System::Int32 vendor_id;
        System::Boolean jsPollerStop;
        System::Boolean initialized;
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

    public ref class Extreme3DProService : public JoystickService
    {
    public:
        Extreme3DProService(int number_joysticks);
        ~Extreme3DProService();

        /**
        Gets the X axis value of the specified joystick ID.
        Pulling to the left constitutes as negative with a min value of -100.
        Pulling to the right is interpreted as positive with a max value of +100.
        @param joystickID the joystick ID
        @param x A reference in which to save the value. Will not be modified if call fails.
        @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetX(int joystickID, Int32% x);

        /**
        Gets the Y axis value of the specified joystick ID.
        Pushing forwards (away from you) constitutes as positive with a max value of +100.
        Pushing backwards (towards you) constitutes as negative with a min value of -100.
        @param joystickID the joystick ID
        @param y A reference in which to save the value. Will not be modified if call fails.
        @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetY(int joystickID, Int32% y);

        /**
        Gets the Z rotation axis value of the specified joystick ID.
        Twisting counter clockwise constitutes as negative, with a min value of -100.
        Twisting clockwise constitutes as positive, with a max value of +100.
        @param joystickID the joystick ID
        @param zRot A reference in which to save the value. Will not be modified if call fails.
        @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetZRot(int joystickID, Int32% zRot);

        /**
        Gets the slider value of the specified joystick ID.
        The '-' marking on the slider is 0, and the "+" marking is 100.
        @param joystickID the joystick ID
        @param slider A reference in which to save the value. Will not be modified if call fails.
        @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetSlider(int joystickID, Int32% slider);

        /**
        * Gets the button state of the specified joystick ID and button.
        * @param joystickID the joystick ID
        * @param button the specific button to query
        * @param buttonVal A reference in which to save the value. Will not be modified if call fails.
        * @return false if invalid joystickID, disconnected joystick, or button not pressed, true otherwise.
        */
        bool GetButton(int joystickID, Extreme3DProButton button, Boolean% buttonVal);

        /**
        Gets the button state of the specified joystick ID.
        @param joystickID the joystick ID
        @param buttons A reference in which to save the value. Will not be modified if call fails.
        @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetButtons(int joystickID, array<bool>^% buttons);

        /**
        Gets the POV hat value of the specified joystick ID.
        @param joystickID the joystick ID
        @param pov A reference in which to save the value. Will not be modified if call fails.
        @return false if invalid joystickID or disconnected joystick, true otherwise.
        */
        bool GetPOV(int joystickID, POV% pov);
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

    public ref class Xbox360Service : public JoystickService
    {
    public:
        Xbox360Service(int number_joysticks);
        ~Xbox360Service();

        bool GetLeftX(int joystickID, Int32% leftX);
        bool GetLeftY(int joystickID, Int32% leftY);
        bool GetRightX(int joystickID, Int32% rightX);
        bool GetRightY(int joystickID, Int32% rightY);
        bool GetLeftTrigger(int joystickID, Int32% leftTrigger);
        bool GetRightTrigger(int joystickID, Int32% rightTrigger);
        bool GetDpad(int joystickID, POV% dpad);
        bool GetButton(int joystickID, Xbox360Button button, Boolean% buttonVal);
        bool GetButtons(int joystickID, array<bool>^% buttons);
    };
}