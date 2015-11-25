using System;
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

        DirectInput directInputHandle;
        Joystick joystick;
        long xVelocity;
        long yVelocity;
        long zRotation;

        public JoystickQueryThread()
        {
            xVelocity = 0;
            yVelocity = 0;
            zRotation = 0;
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

                JoystickState joystickstate = joystick.GetCurrentState();
                xRawVelocity = joystickstate.X;
                yRawVelocity = joystickstate.Y;
                xVelocityValue = xRawVelocity - CENTER_VALUE;
                yVelocityValue = CENTER_VALUE - yRawVelocity;
                zRawRotation = joystickstate.RotationZ;
                zRotationValue = CENTER_VALUE - zRawRotation;

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

        public void Dispose()
        {
            directInputHandle.Dispose();
            joystick.Dispose();
        }
    }
}
