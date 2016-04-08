using System;
using System.Collections.Generic;
using System.Threading;

using JoystickLibrary;

namespace JoystickLibraryTest
{
    public class Program
    {
        public static void Main(string[] args)
        {
            long YVelocity0;
            long XVelocity0;
            long ZRotation0;
            long YVelocity1;
            long XVelocity1;
            long ZRotation1;

            JoystickQueryThread joystick = new JoystickQueryThread(2);
            joystick.Start();

            while (true)
            {
                List<int> ids = joystick.GetJoystickIDs();

                if (ids.Count >= 2)
                {
                    int id0 = ids[0];
                    int id1 = ids[1];

                    bool test = true;
                    test = joystick.GetYVelocity(id0, out YVelocity0);
                    test = joystick.GetXVelocity(id0, out XVelocity0);
                    test = joystick.GetZRotation(id0, out ZRotation0);
                    test = joystick.GetYVelocity(id1, out YVelocity1);
                    test = joystick.GetXVelocity(id1, out XVelocity1);
                    test = joystick.GetZRotation(id1, out ZRotation1);

                    Console.WriteLine("\r Joystick0: velocity: {0}    angle: {1}    ZRotation: {2}\n\rJoystick1:   velocity: {3}    angle: {4}    ZRotation: {5}",
                        YVelocity0,
                        XVelocity0,
                        ZRotation0,
                        YVelocity1,
                        XVelocity1,
                        ZRotation1
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
                    Console.WriteLine("No joysticks");
                    System.Threading.Thread.Sleep(10);
                }

                System.Threading.Thread.Sleep(1);
                Console.Clear();
            }
        }
    }
}
