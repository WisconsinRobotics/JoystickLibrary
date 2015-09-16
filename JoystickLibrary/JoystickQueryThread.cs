using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using SlimDX;
using SlimDX.DirectInput;

namespace JoystickLibrary
{
    public class JoystickQueryThread : IDisposable
    {
        private const int MAX_AXIS_VALUE = 65535;
        private const int CENTER_VALUE = 32767;
        private const float ANGLE_RATIO = 0.0054933317056795f;
        private const float VELOCITY_RATIO = 0.0030518509475997f;

        private DirectInput directInputHandle;
        private Joystick joystick;
        private long angle;
        private long velocity;
        private bool[] dPad;

        public JoystickQueryThread()
        {
            angle = 0;
            velocity = 0;

            
        }

        /* REMOVE AFTER TESTING
        public Joystick Joystick
        {
            get { return joystick; }
        }
        */
        public bool InitializeJoystick()
        {
            directInputHandle = new DirectInput();
            IList<DeviceInstance> devicelist = directInputHandle.GetDevices();

            DeviceInstance joystickinstance = null;
            for (int i = 0; i < devicelist.Count; i++) //Traverse through all the lists and find the joystick to enumerate
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

            joystick = new Joystick(directInputHandle, joystickinstance.ProductGuid); //Create a joystick object to interface with 
            Result acquire = joystick.Acquire(); //Pull all data from it 

            if (acquire.IsFailure)
            {
                joystick = null;
                return false;
            }

            return true;
        }

        public void QueryJoystick()
        {
            long xAngle;
            long yVelocity;
            long angleValue;
            long velocityValue;

            while (true)
            {
                if (joystick == null)
                {
                    if (!InitializeJoystick())
                    {
                        System.Threading.Thread.Sleep(1000);
                        continue;
                    }
                }

                if (joystick.Disposed)
                {
                    return;
                }

                JoystickState joystickstate = joystick.GetCurrentState();
                xAngle = joystickstate.X;
                yVelocity = joystickstate.Y;
                angleValue = xAngle - CENTER_VALUE;
                velocityValue = CENTER_VALUE - yVelocity;


                if (Math.Abs(angleValue) < 4000)
                {
                    Angle = 0;
                }
                else
                {
                    Angle = (long)(angleValue * ANGLE_RATIO);
                }

                if (Math.Abs(velocityValue) < 3000)
                {
                    Velocity = 0;
                }
                else
                {
                    Velocity = (long)(velocityValue * VELOCITY_RATIO);
                }
            }
        }
        
        public long Angle
        {
            get
            {
                return Interlocked.Read(ref angle);
            }

            set
            {
                Interlocked.Exchange(ref angle, value);
            }
        }

        public long Velocity
        {
            get
            {
                return Interlocked.Read(ref velocity);
            }
            set
            {
                Interlocked.Exchange(ref velocity, value);
            }
        }

        public void Dispose()
        {
            directInputHandle.Dispose();
            joystick.Dispose();
        }
    }
}
