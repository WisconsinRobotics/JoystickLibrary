using System.Threading;
using SlimDX.DirectInput;

namespace JoystickLibrary
{
    class JoystickWrapper
    {
        Joystick joystick;
        int id;

        long xVelocity;
        long yVelocity;
        long zRotation;
        long buttons;
        long slider;
        long pov;

        public JoystickWrapper(Joystick joystick, int id)
        {
            this.joystick = joystick;
            this.id = id;
            XVelocity = 0;
            YVelocity = 0;
            ZRotation = 0;
            slider = 0;
            pov = 0;
            buttons = 0;
        }

        public void Reset()
        {
            XVelocity = 0;
            YVelocity = 0;
            ZRotation = 0;
            slider = 0;
            pov = 0;
            buttons = 0;

            joystick.Unacquire();
            joystick.Dispose();
        }

        public long XVelocity
        {
            get
            {
                return Interlocked.Read(ref xVelocity);
            }
            set
            {
                Interlocked.Exchange(ref xVelocity, value);
            }
        }

        public long YVelocity
        {
            get
            {
                return Interlocked.Read(ref yVelocity);
            }
            set
            {
                Interlocked.Exchange(ref yVelocity, value);
            }
        }

        public long ZRotation
        {
            get
            {
                return Interlocked.Read(ref zRotation);
            }
            set
            {
                Interlocked.Exchange(ref zRotation, value);
            }
        }

        // TODO BUTTON FUNCTIOANLITY IS UNTESTED
        public long Buttons
        {
            get
            {
                return Interlocked.Read(ref buttons);
            }
            set
            {
                Interlocked.Exchange(ref buttons, value);
            }
        }

        public long POV
        {
            get
            {
                return Interlocked.Read(ref pov);
            }
            set
            {
                Interlocked.Exchange(ref pov, value);
            }
        }

        public long Slider
        {
            get
            {
                return Interlocked.Read(ref slider);
            }
            set
            {
                Interlocked.Exchange(ref slider, value);
            }
        }

        public Joystick Joystick
        {
            get
            {
                return joystick;
            }
        }

        public int ID
        {
            get
            {
                return id;
            }
        }
    }
}
