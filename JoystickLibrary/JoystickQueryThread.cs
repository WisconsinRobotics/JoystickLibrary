using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Linq;
using System.Threading;
using SlimDX;
using SlimDX.DirectInput;

namespace JoystickLibrary
{
    public enum JoystickButton
    {
        Trigger = 0,
        Button2 = 1,
        Button3 = 2,
        Button4 = 3,
        Button5 = 4,
        Button6 = 5,
        Button7 = 6,
        Button8 = 7,
        Button9 = 8,
        Button10 = 9,
        Button11 = 10,
        Button12 = 11
    }

    public class JoystickQueryThread : IDisposable
    {
        const int MAX_AXIS_VALUE = 65535;
        const int CENTER_VALUE = 32767;
        const float VELOCITY_RATIO = 0.0030518509475997f;
        const float ROTATION_RATIO = 0.0054933317056795f;
        public const int NUMBER_BUTTONS = 12;
        public const int PRIMARY_JOYSTICK_UNASSIGNED = 0;

        int maxNumJoysticks;
        int primaryId; // TODO actually use this once it's detected
        bool primaryIdSet; //Would have used an out paramater and just return true or false
        //but didn't want to change GetPrimaryId method in case it broke other code.
        static object joysticksLock;
        ConcurrentDictionary<int, JoystickWrapper> joysticks;
        List<int> prevIds;
        DirectInput directInputHandle;

        public JoystickQueryThread(int maxNumJoysticks = 1)
        {
            this.maxNumJoysticks = maxNumJoysticks;
            joysticks = new ConcurrentDictionary<int, JoystickWrapper>();
            joysticksLock = new object();
            directInputHandle = new DirectInput();
            primaryId = PRIMARY_JOYSTICK_UNASSIGNED;
            primaryIdSet = false;
        }

        ////////////////////////////
        ///// Public funcitons /////
        ////////////////////////////

        public void Start()
        {
            LocateJoysticks();
            Thread thread = new Thread(QueryJoystick);
            thread.Start();
        }

        public void Dispose()
        {
            foreach (KeyValuePair<int, JoystickWrapper> pair in joysticks)
            {
                pair.Value.Joystick.Dispose();
            }
            directInputHandle.Dispose();
        }


        public List<int> GetJoystickIDs()
        {
            // need to make sure LocateJoysticks doesn't update the Dictionary while we're reading it
            lock (joysticksLock)
            {
                return new List<int>(from entry in joysticks select entry.Value.ID);
            }
        }

        public bool PrimaryIDAssigned()
        {
            return primaryId != PRIMARY_JOYSTICK_UNASSIGNED;
        }

        public int GetPrimaryID()
        {
            return primaryId;
        }

        // if there are exactly 2 joysticks, this retruns the id of the non-primary joystick
        // returns 0 if there is no secondary id
        public int GetSecondaryID()
        {
            if (!PrimaryIDAssigned())
            {
                throw new InvalidOperationException("Primary ID has not been set. " +
                    "Cannot access secondary ID without setting Primary ID.");
            }
            List<int> ids = GetJoystickIDs();
            if (ids.Count != 2)
                return 0;

            foreach (int id in ids)
            {
                if (id != primaryId)
                    return id;
            }
            return 0;
        }

        ////////////////////////////
        //// Accessor funcitons ////
        ////////////////////////////

        public bool GetXVelocity(int joystickID, out long XVelocity)
        {
            lock (joysticksLock)
            {
                bool idFound = joysticks.ContainsKey(joystickID);
                XVelocity = (idFound) ? joysticks[joystickID].XVelocity : default(long);
                return idFound;
            }
        }

        public bool GetYVelocity(int joystickID, out long YVelocity)
        {
            lock (joysticksLock)
            {
                bool idFound = joysticks.ContainsKey(joystickID);
                YVelocity = (idFound) ? joysticks[joystickID].YVelocity : default(long);
                return idFound;
            }
        }

        public bool GetZRotation(int joystickID, out long ZRotation)
        {
            lock (joysticksLock)
            {
                bool idFound = joysticks.ContainsKey(joystickID);
                ZRotation = (idFound) ? joysticks[joystickID].ZRotation : default(long);
                return idFound;
            }
        }

        public bool GetButton(int joystickID, JoystickButton button, out bool IsButtonPressed)
        {
            bool idFound = joysticks.ContainsKey(joystickID);
            IsButtonPressed = (idFound) ?  Convert.ToBoolean(joysticks[joystickID].Buttons & (1 << (int)button)) : default(bool);
            return idFound;
        }

        public bool GetButtons(int joystickID, out bool[] Buttons)
        {
            bool idFound = joysticks.ContainsKey(joystickID);
            long buttonVal = (idFound) ? joysticks[joystickID].Buttons : default(long);

            Buttons = new bool[NUMBER_BUTTONS];
            for (int i = 0; i < NUMBER_BUTTONS; i++)
                Buttons[i] = Convert.ToBoolean(buttonVal & (1 << i));

            return idFound;
        }

        public bool GetSlider(int joystickID, out long Slider)
        {
            lock (joysticksLock)
            {
                bool idFound = joysticks.ContainsKey(joystickID);
                Slider = (idFound) ? joysticks[joystickID].Slider : default(long);
                return idFound;
            }
        }

        public bool GetPOV(int joystickID, out long POV)
        {
            lock (joysticksLock)
            {
                bool idFound = joysticks.ContainsKey(joystickID);
                POV = (idFound) ? joysticks[joystickID].POV : default(long);
                return idFound;
            }
        }

        // gets the data from the 0th joystick
        [Obsolete]
        public long XVelocity
        {
            get
            {
                List<int> ids = GetJoystickIDs();
                if (ids.Count <= 0)
                    return 0;

                long xvelocity;
                if (GetXVelocity(ids[0], out xvelocity))
                    return xvelocity;
                else
                    return 0;
            }
        }

        // gets the data from the 0th joystick
        [Obsolete]
        public long YVelocity
        {
            get
            {
                List<int> ids = GetJoystickIDs();
                if (ids.Count <= 0)
                    return 0;

                long yvelocity;
                if (GetYVelocity(ids[0], out yvelocity))
                    return yvelocity;
                else
                    return 0;
            }
        }

        // gets the data from the 0th joystick
        [Obsolete]
        public long ZRotation
        {
            get
            {
                List<int> ids = GetJoystickIDs();
                if (ids.Count <= 0)
                    return 0;

                int joystickID = ids[0];
                long zrotation;
                if (GetZRotation(ids[0], out zrotation))
                    return zrotation;
                else
                    return 0;
            }
        }

        ////////////////////////////
        //// Private funcitons /////
        ////////////////////////////

        private void LocateJoysticks()
        {
            IList<DeviceInstance> devicelist = directInputHandle.GetDevices();
            List<int> currIds = new List<int>();

            // find all new joysticks
            for (int i = 0; i < devicelist.Count; i++)
            {
                DeviceInstance dinstance = devicelist.ElementAt(i);

                // if device is not a joystick or gamepad, skip and continue
                if (dinstance.Type != DeviceType.Joystick && dinstance.Type != DeviceType.Gamepad)
                    continue;

                int id = dinstance.InstanceGuid.GetHashCode();
                currIds.Add(id);

                // don't add to the dictionary, but still get all the ids
                if (joysticks.Count >= maxNumJoysticks)
                    continue;

                // add to dictionary
                if (joysticks.ContainsKey(id))
                    continue;

                Joystick joystick = new Joystick(directInputHandle, dinstance.InstanceGuid); // Create a joystick object to interface with 
                Result acquire = joystick.Acquire(); // Pull all data from it 

                if (!acquire.IsFailure)
                {
                    // need to make sure we don't update the Dictionary while GetJoystickIDs is reading it
                    lock (joysticksLock)
                    {
                        JoystickWrapper wrapper = new JoystickWrapper(joystick, id);
                        joysticks.TryAdd(id, wrapper);
                    }
                }
            }

            if (prevIds != null)
            {
                // check if joysticks were unplugged - TODO this may or may not work - I haven't tested it
                foreach (int id in prevIds)
                {
                    if (!currIds.Contains(id))
                    { // this joystick was here last time but now isn't
                        if (joysticks.ContainsKey(id))
                        {
                            JoystickWrapper joystickWrapper = joysticks[id];
                            joystickWrapper.Reset();
                            joysticks.TryRemove(id, out joystickWrapper);

                            // if we are at max joystick capacity (maxNumJoysticks) and a joystick is removed on this loop iteration,
                            // we will remove it and try to add another next iteration
                        }
                    }
                }
            }

            prevIds = GetJoystickIDs(); // need to call GetJoystickIDs(). can't use currIds because it might contain some extra joysticks
        }

        public void QueryJoystick()
        {
            while (true)
            {
                LocateJoysticks();

                if (joysticks.Count == 0)
                {
                    Thread.Sleep(100);
                    continue;
                }

                lock (joysticksLock)
                {
                    foreach (KeyValuePair<int, JoystickWrapper> pair in joysticks)
                    {
                        JoystickWrapper joystickWrapper = pair.Value;
                        if (joystickWrapper.Joystick.Disposed)
                        {
                            // TODO maybe remove it, but I don't think you can modify the Dictionary in a foreach
                            // you could add it to a list to remove after the loop is complete
                            continue;
                        }

                        long xRawVelocity = 0L;
                        long yRawVelocity = 0L;
                        long xVelocityValue = 0L;
                        long yVelocityValue = 0L;
                        long zRawRotation = 0L;
                        long zRotationValue = 0L;
                        int[] pointOfViewControllers;
                        bool[] buttons = new bool[NUMBER_BUTTONS];

                        JoystickState joystickstate = joystickWrapper.Joystick.GetCurrentState();
                        int test = joystickWrapper.Joystick.GetHashCode();

                        xRawVelocity = joystickstate.X;
                        yRawVelocity = joystickstate.Y;

                        xVelocityValue = xRawVelocity - CENTER_VALUE;
                        yVelocityValue = CENTER_VALUE - yRawVelocity;

                        zRawRotation = joystickstate.RotationZ;
                        zRotationValue = CENTER_VALUE - zRawRotation;

                        // set the buttons                        
                        long buttonBitSetValue = 0;
                        buttons = joystickstate.GetButtons();
                        for (int i = 0; i < NUMBER_BUTTONS; i++)
                            buttonBitSetValue |= (Convert.ToUInt32(buttons[i]) << i);
                        joystickWrapper.Buttons = buttonBitSetValue;

                        if (primaryId == 0 && buttons[0])
                            primaryId = joystickWrapper.ID;

                        if (joystickWrapper.ID == primaryId)
                            joystickWrapper.IsPrimary = 1;

                        joystickWrapper.Slider = joystickstate.GetSliders()[0];
                        pointOfViewControllers = joystickstate.GetPointOfViewControllers();
                        joystickWrapper.POV = ((pointOfViewControllers[0] == -1) ? -1 : pointOfViewControllers[0] / 100);

                        // account for dead zone: angle
                        if (Math.Abs(xVelocityValue) < 4000)
                        {
                            joystickWrapper.XVelocity = 0;
                        }
                        else
                        {
                            joystickWrapper.XVelocity = (long)(xVelocityValue * VELOCITY_RATIO);
                        }

                        // account for dead zone: velocity
                        if (Math.Abs(yVelocityValue) < 3000)
                        {
                            joystickWrapper.YVelocity = 0;
                        }
                        else
                        {
                            joystickWrapper.YVelocity = (long)(yVelocityValue * VELOCITY_RATIO);
                        }

                        // account for dead zone: rotation
                        if (Math.Abs(zRotationValue) < 3000)
                        {
                            joystickWrapper.ZRotation = 0;
                        }
                        else
                        {
                            joystickWrapper.ZRotation = -(long)(zRotationValue * ROTATION_RATIO);
                        }
                    }
                }
            }
        }


    }
}
