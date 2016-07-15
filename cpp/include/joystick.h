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

#define NUMBER_BUTTONS 12
#define JOYSTICK_VENDOR_ID 0x46D
#define JOYSTICK_PRODUCT_ID 0xC215 

#include <array>
#include <map>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <cstdint>

namespace JoystickLibrary
{
    /**
     * Represents the point of view (POV) hat on the joystick.
     */
    enum class POV : int
    {
        POV_NONE = 0,                               /**< POV hat is not in use.              */
        POV_WEST = 1 << 0,                          /**< POV hat is facing left.             */
        POV_EAST = 1 << 1,                          /**< POV hat is facing right.            */
        POV_NORTH = 1 << 2,                         /**< POV hat is facing up.               */
        POV_SOUTH = 1 << 3,                         /**< POV hat is facing down.             */
        POV_NORTHWEST = POV_NORTH | POV_WEST,       /**< POV hat is facing up and left.      */
        POV_NORTHEAST = POV_NORTH | POV_EAST,       /**< POV hat is facing up and right.     */
        POV_SOUTHWEST = POV_SOUTH | POV_WEST,       /**< POV hat is facing south and left.   */
        POV_SOUTHEAST = POV_SOUTH | POV_EAST        /**< POV hat is facing south and right.  */
    };

    /**
     * Represents all of the available buttons on the joystick.
     */
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

    /**
     * Holds the current joystick state as well as a handle to the 
     * underlying OS specific object.
     */
    struct JoystickData
    {
        bool alive;                                 /**< Joystick is plugged in   */
        int x;                                      /**< Current x axis value     */
        int y;                                      /**< Current y axis value     */
        int rz;                                     /**< Current z rotation value */
        int slider;                                 /**< Current slider value     */
        std::array<bool, NUMBER_BUTTONS> buttons;   /**< Current button state     */
        POV pov;                                    /**< Current POV value        */
        void *os_obj;                               /**< Pointer to OS handle     */
    };

    /*
     * The JoystickService class looks for, initializes, configures joysticks
     * and exposes its state to the user.
     */
    class JoystickService
    {
    public:

        /**
         * Constructs a new JoystickService instance with the specified number of joysticks.
         */
        JoystickService(int number_joysticks);
        ~JoystickService(void);

        /**
         * Initializes the JoystickService object.
         * This entails OS specific initialization, such as DirectInput bringup on Windows.
         * @return true on success, false otherwise
         */
        bool Initialize(void);

        /**
         * Gets all connected joystick IDs.
         * @param list A vector reference in which to store the IDs.
         */
        void GetJoystickIDs(std::vector<int>& list);

        /**
         * Gets the number of connected joysticks.
         * @return The number of connected joysticks.  
         */
        int GetConnectedJoysticksCount(void);

        /**
         * Starts the polling/enumeration thread. 
         * @return true on success, false otherwise
         */
        bool Start(void);

        /**
         * Gets the X axis value of the specified joystick ID.
         * @param joystickID the joystick ID
         * @param x A reference in which to save the value. Will not be modified if call fails. 
         * @return false if invalid joystickID or disconnected joystick, true otherwise.
         */
        bool GetX(int joystickID, int& x);

        /**
         * Gets the Y axis value of the specified joystick ID.
         * @param joystickID the joystick ID
         * @param y A reference in which to save the value. Will not be modified if call fails. 
         * @return false if invalid joystickID or disconnected joystick, true otherwise.
         */
        bool GetY(int joystickID, int& y);

        /**
         * Gets the Z rotation axis value of the specified joystick ID.
         * @param joystickID the joystick ID
         * @param zRot A reference in which to save the value. Will not be modified if call fails. 
         * @return false if invalid joystickID or disconnected joystick, true otherwise.
         */
        bool GetZRot(int joystickID, int& zRot);

        /**
         * Gets the Y axis value of the specified joystick ID.
         * @param joystickID the joystick ID
         * @param y A reference in which to save the value. Will not be modified if call fails. 
         * @return false if invalid joystickID or disconnected joystick, true otherwise.
         */
        bool GetSlider(int joystickID, int& slider);

        /**
         * Gets the button state of the specified joystick ID.
         * @param joystickID the joystick ID
         * @param buttons A reference in which to save the value. Will not be modified if call fails. 
         * @return false if invalid joystickID or disconnected joystick, true otherwise.
         */
        bool GetButtons(int joystickID, std::array<bool, NUMBER_BUTTONS>& buttons);

        /**
         * Gets the POV hat value of the specified joystick ID.
         * @param joystickID the joystick ID
         * @param pov A reference in which to save the value. Will not be modified if call fails. 
         * @return false if invalid joystickID or disconnected joystick, true otherwise.
         */
        bool GetPOV(int joystickID, POV& pov);

        /**
         * Removes the specified joystick ID.
         * @param joystickID the joystick ID
         * @return false if invalid joystickID, true otherwise.
         */
        bool RemoveJoystick(int joystickID);
       
    private:

        /**
         * Main polling and enumeration loop.
         */
        void PollJoysticks(void);

        /**
         * Locates, enumerates, and configures connected joysticks.
         */
        void LocateJoysticks(void);

        /**
         * Helper function to check if a joystickID is valid and whether or not
         * the joystick is connected.
         * @param joystickID the joystick ID to check
         * @return true if connected and valid, false otherwise
         */
        bool IsValidJoystickID(int joystickID);
        
        std::map<int, JoystickData> jsMap;
        std::thread jsPoller;
        std::mutex rwLock;
        int requestedJoysticks;
        int connectedJoysticks;
        bool jsPollerStop;
        bool initialized;
        int nextJoystickID;
    };
}