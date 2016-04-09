using System;
using System.Collections.Generic;
using System.Threading;

using JoystickLibrary;

namespace JoystickLibraryTest
{
    public class Program
    {
        const int NUMBER_JOYSTICKS = 2;

        public static void Main(string[] args)
        {
            Console.WriteLine("Please make the console window as wide as possible, then type stuff and hit enter.");
            Console.ReadLine();

            JoystickQueryThread joystick = new JoystickQueryThread(NUMBER_JOYSTICKS);
            joystick.Start();

            Console.WriteLine("{0,-15} {1,-10} {2,-10} {3,-10} {4,-10} {5,-100} {6,-10} {7,-10}", "Joystick #", "Success", "XVelocity", "YVelocity", "ZRotation", "Buttons", "Slider", "POV");
            while (true)
            {
                List<int> ids = joystick.GetJoystickIDs();

                int index = 0;
                foreach (int id in ids)
                {
                    bool success = true;
                    long XVelocity;
                    long YVelocity;
                    long ZRotation;
                    bool[] Buttons;
                    long POV;
                    long Slider;

                    success &= joystick.GetYVelocity(id, out YVelocity);
                    success &= joystick.GetXVelocity(id, out XVelocity);
                    success &= joystick.GetZRotation(id, out ZRotation);
                    success &= joystick.GetButtons(id, out Buttons);
                    success &= joystick.GetSlider(id, out Slider);
                    success &= joystick.GetPOV(id, out POV);

                    string formattedButtonString = string.Empty;
                    for (int i = 0; i < Buttons.Length; i++)
                        formattedButtonString += string.Format("[{0}]: {1} ", i, Buttons[i] ? 1 : 0);

                    Console.WriteLine("{0,-15} {1,-10} {2,-10} {3,-10} {4,-10} {5,-100} {6,-10} {7,-10}", index, success, XVelocity, YVelocity, ZRotation, formattedButtonString, Slider, POV);

                    index++;
                }

                Thread.Sleep(50);
            }
        }
    }
}
