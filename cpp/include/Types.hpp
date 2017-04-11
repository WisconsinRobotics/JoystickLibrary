#pragma once
#include <map>
#include <vector>
#include <functional>
#include <cstring>
#include <string>
#include <algorithm>

#ifdef _WIN32
    #define DIRECTINPUT_VERSION 0x0800
    #define WIN32_LEAN_AND_MEAN

    #include <dinput.h>
    #include <windows.h>
    #include <Dbt.h>

    #pragma comment(lib, "dinput8.lib")
    #pragma comment(lib, "dxguid.lib")

    typedef LPDIRECTINPUTDEVICE8 JoystickHandle;
    typedef DIJOYSTATE JoystickState;
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <linux/input.h>
    #include <libevdev/libevdev.h>
    #include <libudev.h>
    #include <stdlib.h>
    #include <locale.h>
    #include <thread>
    #include <mutex>

    typedef struct JoystickHandle
    {
        struct libevdev *dev;
        int fd;
        char path[64]; // "/dev/input/event*"
    } JoystickHandle;

    struct JoystickState
    {
        std::map<int, int> axes;
        std::map<int, bool> buttons;
    };
#endif

namespace JoystickLibrary
{
    enum class POV : int
    {
        POV_NONE = 0,                               /**< POV hat is not in use.              */
        POV_WEST = 1 << 0,                          /**< POV hat is facing left.             */
        POV_EAST = 1 << 1,                          /**< POV hat is facing right.            */
        POV_NORTH = 1 << 2,                         /**< POV hat is facing up.               */
        POV_SOUTH = 1 << 3,                         /**< POV hat is facing down.             */
        POV_NORTHWEST = POV_NORTH | POV_WEST,       /**< POV hat is facing up and left.      */
        POV_NORTHEAST = POV_NORTH | POV_EAST,       /**< POV hat is facing up and right.     */
        POV_SOUTHWEST = POV_SOUTH | POV_WEST,       /**< POV hat is facing south and left.   */
        POV_SOUTHEAST = POV_SOUTH | POV_EAST        /**< POV hat is facing south and right.  */
    };

    struct JoystickDescriptor
    {
        int vendor_id;
        int product_id;
        
        bool operator== (const JoystickDescriptor &j1) const
        {
            return (vendor_id == j1.vendor_id) 
                && (product_id == j1.product_id);
        }
    };

    struct JoystickData
    {
        bool alive;
        JoystickState state;
        JoystickHandle handle;
        JoystickDescriptor descriptor;
    };

    struct DeviceStateChange
    {
        enum class State
        {
            ADDED, REMOVED
        };

        State state;
        JoystickDescriptor descriptor;
        int id;

        bool operator== (const DeviceStateChange &dsc) const
        {
            return (state == dsc.state) 
                && (descriptor == dsc.descriptor) 
                && (dsc.id == id);
        }
    };

    static int NormalizeAxisValue(int val, int min, int max)
    {
        return (int)((200.0 / (max - min)) * (val)-100 * ((max + min) / (max - min)));
    }
}
