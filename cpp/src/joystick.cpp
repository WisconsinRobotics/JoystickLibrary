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

#include "joystick.h"

using namespace JoystickLibrary;

JoystickService::JoystickService(int number_joysticks)
{
    this->requestedJoysticks = number_joysticks;
    this->connectedJoysticks = 0;
    this->jsPollerStop = false;
    this->initialized = false;
    this->nextJoystickID = 1;
}

void JoystickService::GetJoystickIDs(std::vector<int>& list)
{
    for (auto pair : this->jsMap)
        if (pair.second.alive)
            list.push_back(pair.first);
}

int JoystickService::GetConnectedJoysticksCount(void)
{
    return this->connectedJoysticks;
}

bool JoystickService::Start(void)
{
    if (!this->initialized)
        return false;
    
    jsPollerStop = false;
    jsPoller = std::thread(&JoystickService::PollJoysticks, this);
    return true;
}

bool JoystickService::GetX(int joystickID, int& x)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;
    
    x = jsMap[joystickID].x;
    return true;
}

bool JoystickService::GetY(int joystickID, int& y)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    y = jsMap[joystickID].y;
    return true;
}

bool JoystickService::GetZRot(int joystickID, int& zRot)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    zRot = jsMap[joystickID].rz;    
    return true;
}

bool JoystickService::GetSlider(int joystickID, int& slider)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;  

    slider = jsMap[joystickID].slider;
    return true;    
}

bool JoystickService::GetButtons(int joystickID, std::array<bool, NUMBER_BUTTONS>& buttons)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    buttons = jsMap[joystickID].buttons;
    return true;
}

bool JoystickService::GetPOV(int joystickID, POV& pov)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;  

    pov = jsMap[joystickID].pov;    
    return true;
}

bool JoystickService::IsValidJoystickID(int joystickID)
{
    return (this->jsMap.find(joystickID) != this->jsMap.end()) 
                && this->jsMap[joystickID].alive;
}