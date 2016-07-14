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

#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <memory>

#include "joystick.h"

using namespace JoystickLibrary;

std::map<POV, std::string> povNameMap = {
    { POV::POV_NONE, "POV_NONE" },
    { POV::POV_WEST, "POV_WEST" },
    { POV::POV_EAST, "POV_EAST" },
    { POV::POV_NORTH, "POV_NORTH" },
    { POV::POV_SOUTH, "POV_SOUTH" },
    { POV::POV_NORTHWEST, "POV_NORTHWEST" },
    { POV::POV_NORTHEAST, "POV_NORTHEAST" },
    { POV::POV_SOUTHWEST, "POV_SOUTHWEST" },
    { POV::POV_SOUTHEAST, "POV_SOUTHEAST" }
};

void PrintAbsoluteAxes(JoystickService& s, int id)
{
    int x, y, z, slider;

    if (!s.GetX(id, x))
        x = 0;
    if (!s.GetY(id, y))
        y = 0;    
    if (!s.GetZRot(id, z))
        z = 0;
    if (!s.GetSlider(id, slider))
        slider = 0;

    std::cout << "X: " << x << " | Y: " << y << " | Z: " << z <<  " | Slider: " << slider << std::endl;
}

void PrintButtons(JoystickService& s, int id)
{
    std::array<bool, 12> buttons;
    if (!s.GetButtons(id, buttons))
        std::fill(buttons.begin(), buttons.end(), false);

    for (int i = 0; i < 12; i++)
        std::cout << i << ": " << buttons[i] << " | ";
    std::cout << std::endl;
}

void PrintPOV(JoystickService& s, int id)
{
    POV pov;
    if (!s.GetPOV(id, pov))
        pov = POV::POV_NONE;

    std::cout << "POV: " << povNameMap[pov] << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "usage: " << argv[0] << " <number_joysticks>" << std::endl;
        return 1;
    }

    long int num_joysticks = strtol(argv[1], nullptr, 10);
    if (num_joysticks <= 0)
    {
        std::cout << "Please enter a valid number of joysticks (> 0)." << std::endl;
        return 1;
    }

    std::unique_ptr<JoystickService> s(new JoystickService(num_joysticks));
    if (!s->Initialize())
    {
        std::cout << "Failed to initialize!" << std::endl;
        return 1;
    }

    if (!s->Start())
    {
        std::cout << "Failed to start!" << std::endl;
        return 1;
    }

    std::cout << "Waiting for a joystick to be plugged in..." << std::endl;
    while (s->GetConnectedJoysticksCount() < 1);
    std::cout << "Found one - starting main loop." << std::endl;

    std::vector<int> ids;
    while (true)
    {
        s->GetJoystickIDs(ids);
        for (int i : ids)
        {
            std::cout << "[" << i << "] ";
            PrintAbsoluteAxes(*s, i);
            PrintButtons(*s, i);
            PrintPOV(*s, i); 
        }

        ids.clear();
    }

    return 0;
}
