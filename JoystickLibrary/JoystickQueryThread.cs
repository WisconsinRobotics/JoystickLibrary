using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using SlimDX;
using SlimDX.DirectInput;

namespace JoystickLibrary
{
    public class JoystickQueryThread : IDisposable
    {
        const int MAX_AXIS_VALUE = 65535;
        const int CENTER_VALUE = 32767;
        const float ANGLE_RATIO = 0.0054933317056795f;
        const float VELOCITY_RATIO = 0.0030518509475997f;
        const float ROTATION_RATIO = 0.0054933317056795f;
        public const int NUMBER_BUTTONS = 12;

        DirectInput directInputHandle;
        Joystick joystick;
        long xVelocity;
        long yVelocity;
        long zRotation;
        bool[] buttons;
        long slider;
        long pov;
        List<JoystickWrapper> joysticks;

        public JoystickQueryThread()
        {
            xVelocity = 0;
            yVelocity = 0;
            zRotation = 0;
            slider = 0;
            pov = 0;
            buttons = new bool[NUMBER_BUTTONS];
        }

        public bool InitializeJoystick()
        {
            if (directInputHandle != null && joystick != null)
                return true;

            directInputHandle = new DirectInput();
            IList<DeviceInstance> devicelist = directInputHandle.GetDevices();

            DeviceInstance joystickinstance = null;
            for (int i = 0; i < devicelist.Count; i++) // Traverse through all the lists and find the joystick to enumerate
            {
                DeviceInstance dinstance = devicelist.ElementAt(i);
                if (dinstance.Type == DeviceType.Joystick || dinstance.Type == DeviceType.Gamepad)
                {
                    joystickinstance = dinstance;
                    break;
                }
            }

            if (joystickinstance == null)
            {
                return false;
            }

            joystick = new Joystick(directInputHandle, joystickinstance.ProductGuid); // Create a joystick object to interface with 
            Result acquire = joystick.Acquire(); // Pull all data from it 

            if (acquire.IsFailure)
            {
                joystick = null;
                directInputHandle = null;
                return false;
            }

            return true;
        }

        public void QueryJoystick()
        {
            long xRawVelocity;
            long yRawVelocity;
            long xVelocityValue;
            long yVelocityValue;
            long zRawRotation;
            long zRotationValue;
            int[] pointOfViewControllers;

            while (true)
            {
                if (joystick == null)
                {
                    if (!InitializeJoystick())
                    {
                        Thread.Sleep(1000);
                        continue;
                    }
                }

                if (joystick.Disposed)
                {
                    return;
                }

                // read values from the joystick
                JoystickState joystickstate = joystick.GetCurrentState();
                xRawVelocity = joystickstate.X;
                yRawVelocity = joystickstate.Y;

                xVelocityValue = xRawVelocity - CENTER_VALUE;
                yVelocityValue = CENTER_VALUE - yRawVelocity;

                zRawRotation = joystickstate.RotationZ;
                zRotationValue = CENTER_VALUE - zRawRotation;

                slider = joystickstate.GetSliders()[0];

                pointOfViewControllers = joystickstate.GetPointOfViewControllers();
                pov = ((pointOfViewControllers[0] == -1) ? -1 : pointOfViewControllers[0] / 100);

                buttons = joystickstate.GetButtons();

                // account for dead zone: angle
                if (Math.Abs(xVelocityValue) < 4000)
                {
                    XVelocity = 0;
                }
                else
                {
                    XVelocity = (long)(xVelocityValue * ANGLE_RATIO);
                }

                // account for dead zone: velocity
                if (Math.Abs(yVelocityValue) < 3000)
                {
                    YVelocity = 0;
                }
                else
                {
                    YVelocity = (long)(yVelocityValue * VELOCITY_RATIO);
                }

                // account for dead zone: rotation
                if (Math.Abs(zRotationValue) < 3000)
                {
                    ZRotation = 0;
                }
                else
                {
                    ZRotation = -(long)(zRotationValue * ROTATION_RATIO);
                }
            }
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

        public bool[] Buttons
        {
            get
            {
                // since the buttons are represented as an exploded
                // bitset, place them into a long in order to exchange
                // it atomically with another thread
                long buttonBitSetValue = 0;
                for (int i = 0; i < NUMBER_BUTTONS; i++)
                {
                    buttonBitSetValue |= (Convert.ToUInt32(this.buttons[i]) << i);
                }

                // pass compressed bitset to other thread
                long localButtonBitSetValue = Interlocked.Read(ref buttonBitSetValue);

                // once the condensed button bitset has been exchanged to the
                // other thread, decompress the returned integer back into bool[]
                bool[] localButtons = new bool[NUMBER_BUTTONS];

                for (int i = 0; i < NUMBER_BUTTONS; i++)
                {
                    localButtons[i] = Convert.ToBoolean(localButtonBitSetValue & (1 << i));
                }

                return localButtons;
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

        public void Dispose()
        {
            directInputHandle.Dispose();
            joystick.Dispose();
        }
    }
}
