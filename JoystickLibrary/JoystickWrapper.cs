using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SlimDX;
using SlimDX.DirectInput;

namespace JoystickLibrary
{
    class JoystickWrapper
    {
        Joystick joystick;
        Guid instanceGuid;
        long xVelocity;
        long yVelocity;
        long zRotation;
        bool[] buttons;
        long slider;
        long pov;

        public JoystickWrapper()
        {
        }
    }
}
