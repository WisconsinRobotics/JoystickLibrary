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

using System;
using System.Collections.Generic;
using JoystickLibrary;


namespace JoystickLibraryTester
{
    class JoystickTester
    {
        static readonly Dictionary<POV, string> povNameMap = new Dictionary<POV, string>
        {
            { POV.POV_NONE, "POV_NONE" },
            { POV.POV_WEST, "POV_WEST" },
            { POV.POV_EAST, "POV_EAST" },
            { POV.POV_NORTH, "POV_NORTH" },
            { POV.POV_SOUTH, "POV_SOUTH" },
            { POV.POV_NORTHWEST, "POV_NORTHWEST" },
            { POV.POV_NORTHEAST, "POV_NORTHEAST" },
            { POV.POV_SOUTHWEST, "POV_SOUTHWEST" },
            { POV.POV_SOUTHEAST, "POV_SOUTHEAST" }
        };

        static void PrintAbsoluteAxes(JoystickService s, int id)
        {
            int x = 0, y = 0, z = 0, slider = 0;

            if (!s.GetX(id, ref x))
                x = 0;
            if (!s.GetY(id, ref y))
                y = 0;
            if (!s.GetZRot(id, ref z))
                z = 0;
            if (!s.GetSlider(id, ref slider))
                slider = 0;

            Console.WriteLine("X: {0} | Y: {1} | Z: {2} | Slider: {3}", x, y, z, slider);
        }

        static void PrintButtons(JoystickService s, int id)
        {
            bool[] buttons = new bool[JoystickService.NUMBER_BUTTONS];

            if (!s.GetButtons(id, ref buttons))
                for (int i = 0; i < 12; i++)
                    buttons[i] = false;

            for (int i = 0; i < 12; i++)
                Console.Write("{0} {1} ", i, buttons[i]);
            Console.WriteLine();
        }

        static void PrintPOV(JoystickService s, int id)
        {
            POV pov = POV.POV_NONE;
            if (!s.GetPOV(id, ref pov))
                pov = POV.POV_NONE;

            Console.WriteLine("{0}", povNameMap[pov]);
        }

        static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("usage: JoystickLibraryTester.exe <number_joysticks>");
                return;
            }

            int num_joysticks;
            if (!int.TryParse(args[0], out num_joysticks))
            {
                Console.WriteLine("Please enter a valid number of joysticks (> 0).");
                return;
            }

            JoystickService s = new JoystickService(num_joysticks);
            if (!s.Initialize())
            {
                Console.WriteLine("Failed to initialize!");
                return;
            }

            if (!s.Start())
            {
                Console.WriteLine("Failed to start!");
                return;
            }

            Console.WriteLine("Waiting for a joystick to be plugged in...");
            while (s.GetConnectedJoysticksCount() < 1);
            Console.WriteLine("Found one - starting main loop.");

            while (true)
            {
                foreach (int i in s.GetJoystickIDs())
                {
                    Console.Write("[{0}] ", i);
                    PrintAbsoluteAxes(s, i);
                    //PrintButtons(s, i);
                    //PrintPOV(s, i); 
                }
            }
        }
    }
}