using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using SlimDX;
using SlimDX.DirectInput;

using JoystickLibrary;

namespace JoystickLibraryTest
{
    public class Program
    {

        public static void Main(string[] args)
        {
            long angle;
            long velocity;
            int[] dPad;

            JoystickQueryThread joystick = new JoystickQueryThread();
            Thread thread = new Thread(joystick.QueryJoystick);
            thread.Start();

            while (true)
            {
                /*
                //JoystickState state = joystick.Joystick.GetCurrentState();
                JoystickState state = new JoystickState();
                dPad = state.GetPointOfViewControllers();
               

                string txt = string.Empty;

                for (int i = 0; i < dPad.Length; i++)
                {
                    txt += dPad[i] + "     ";
                }

               // Console.Write("\r{0}", txt);
               // System.Threading.Thread.Sleep(500);
                */
                velocity = joystick.YVelocity;
                angle = joystick.XVelocity;
                //Console.Write("\r velocity: {0}    angle: {1}", velocity, angle);
                Console.Write("\r Z Rotation: {0}", joystick.ZRotation);
                
            }

        }
    }
}
