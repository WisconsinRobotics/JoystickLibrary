# JoystickLibrary
This library is used for reading joystick values via SlimDX. It polls values
from the X and Y axis of the joystick on a dedicated thread and stores the
values on a scale of -100 to 100. It is used by a variety of systems written
by Wisconsin Robotics for both direct and remote control driving of systems.

The library is intended to be fault tolerant, able to deal with the absence
and loss of joysticks on a system.
