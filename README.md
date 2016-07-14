# JoystickLibrary

This library is a cross-platform solution for reading from multiple Logitech Extreme 3D Pro joysticks in a fault-tolerant fashion. It is used by a variety of systems written by Wisconsin Robotics for both direct and remote robot control.

JoystickLibrary is licensed under the 3-clause BSD license.

## Building
Supported platforms are Windows and Linux, and you must have a compiler that supports C++11. For the C++ library, you must have cmake installed. Additional dependencies will vary by operating system.

### Windows
This library was built and tested with MSVC 19 (Visual Studio 2015). To build the C++ library, run the following:
```
cd cpp/
mkdir build/
cd build/
cmake ..
cmake --build .
```
You will find the built binary within src/Debug. The example application can be found within sample/Debug.

To build the C# library, open up the solution file in the csharp/ folder, and compile within Visual Studio.

### Linux
The Linux build depends on libevdev-dev. pkg-config must also be present on your system as well. To build, run:
```
cd cpp/
mkdir build/
cmake ..
make
```

You will find the binaries within the src and sample folders.

### Example application
The jstester application is a simple application that displays various state information about each connected joystick. Run with ```jstester <number_joysticks>```.