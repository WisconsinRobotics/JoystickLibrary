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
            JoystickQueryThread joystick = new JoystickQueryThread(NUMBER_JOYSTICKS);
            joystick.Start();

            while (true)
            {
                System.Threading.Thread.Sleep(10);
                Console.Clear();

                List<int> ids = joystick.GetJoystickIDs();

                long YVelocity0;
                long XVelocity0;
                long ZRotation0;
                long YVelocity1;
                long XVelocity1;
                long ZRotation1;

                if (ids.Count == 2)
                {
                    if (joystick.GetPrimaryID() == 0)
                    {
                        Console.WriteLine("No primary id (click the trigger to get a primary id)");
                        continue;
                    }

                    int id0 = joystick.GetPrimaryID();
                    int id1 = joystick.GetSecondaryID();

                    bool success = true;
                    success &= joystick.GetYVelocity(id0, out YVelocity0);
                    success &= joystick.GetXVelocity(id0, out XVelocity0);
                    success &= joystick.GetZRotation(id0, out ZRotation0);
                    success &= joystick.GetYVelocity(id1, out YVelocity1);
                    success &= joystick.GetXVelocity(id1, out XVelocity1);
                    success &= joystick.GetZRotation(id1, out ZRotation1);

                    Console.WriteLine("\r Joystick0: velocity: {0}    angle: {1}    ZRotation: {2}\n\rJoystick1:   velocity: {3}    angle: {4}    ZRotation: {5}" +
                        "\nPrimary ID: {6}",
                        YVelocity0,
                        XVelocity0,
                        ZRotation0,
                        YVelocity1,
                        XVelocity1,
                        ZRotation1,
                        joystick.GetPrimaryID()
                        );
                }
                else if (ids.Count == 1)
                {
                    int id = ids[0];
                    joystick.GetYVelocity(id, out YVelocity0);
                    joystick.GetXVelocity(id, out XVelocity0);
                    joystick.GetZRotation(id, out ZRotation0);

                    Console.WriteLine("\r velocity: {0}    angle: {1}    ZRotation: {2}", YVelocity0, XVelocity0, ZRotation0);
                }
                else
                {
                    Console.WriteLine("No joysticks or more than 2 joystick");
                    System.Threading.Thread.Sleep(10);
                }
            }
        }
    }
}
