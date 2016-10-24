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
#include "ServiceTester.h"
#include <memory>

using namespace JoystickLibrary;

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

    //std::unique_ptr<Extreme3DProServiceTester> s(new Extreme3DProServiceTester(num_joysticks));
    std::unique_ptr<Xbox360ServiceTester> s(new Xbox360ServiceTester(num_joysticks));
    if (!s->Start())
    {
        std::cout << "Failed to start/init!" << std::endl;
        return 1;
    }

    std::cout << "Waiting for a joystick to be plugged in..." << std::endl;
    while (s->service->GetConnectedJoysticksCount() < 1);
    std::cout << "Found one - starting main loop." << std::endl;

    std::vector<int> ids;
    while (true)
    {
        s->service->GetJoystickIDs(ids);
        for (int i : ids)
        {
            std::cout << "[" << i << "] ";
            //s->PrintAbsoluteAxes(i);
            s->PrintButtons(i);
            //s->PrintDpad(i);
        }

        ids.clear();
    }

    return 0;
}
