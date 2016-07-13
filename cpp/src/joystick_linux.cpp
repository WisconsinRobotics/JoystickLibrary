#include <errno.h>
#include <cstring>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <libevdev/libevdev.h>
#include <string>
#include <algorithm>
#include <errno.h>

#include "joystick.h"

#include <iostream>

using namespace JoystickLibrary;

std::vector<std::string> jsHandleList;

constexpr int X_MIN = 0;
constexpr int X_MAX = 1023;
constexpr int Y_MIN = 0;
constexpr int Y_MAX = 1023;
constexpr int Z_MIN = 0;
constexpr int Z_MAX = 255;
constexpr int SLIDER_MIN = 255;
constexpr int SLIDER_MAX = 0;


static int NormalizeJoystickValue(int val, int min, int max)
{
    return (int) ((200.0 / (max - min)) * (val) - 100);
}

JoystickService::~JoystickService(void)
{
    this->jsPollerStop = true;
    this->jsPoller.join();
    
    for (auto pair : jsMap)
    {
        struct libevdev *evdev = (struct libevdev *) pair.second.os_obj;
        if (!evdev)
            continue;
        
        close(libevdev_get_fd(evdev));
        libevdev_free(evdev);
    }
}

bool JoystickService::Initialize(void)
{
    this->initialized = true;
    return true;
}

bool JoystickService::RemoveJoystick(int joystickID)
{
    if (!this->initialized || !this->IsValidJoystickID(joystickID))
        return false;
    
    this->rwLock.lock();
    
    this->jsMap[joystickID].alive = false;
    struct libevdev *joystick = (struct libevdev *) jsMap[joystickID].os_obj;
    close(libevdev_get_fd(joystick));
    this->connectedJoysticks--;
    
    this->rwLock.unlock();
    return true;
}

void JoystickService::PollJoysticks(void)
{
    if (!this->initialized)
        return;

    while (!this->jsPollerStop)
    {
        this->rwLock.lock();
        this->LocateJoysticks();
        
        for (auto& pair : jsMap)
        {
            int rc;
            struct libevdev *dev;

            if (!pair.second.alive)
                continue;

            JoystickData& jsData = pair.second;
            dev = (struct libevdev *) jsData.os_obj;

            // read the joystick state
            while (libevdev_has_event_pending(dev) > 0)
            {
                struct input_event ev;

                rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING, &ev);

                if (rc == LIBEVDEV_READ_STATUS_SYNC) 
                {
                    while (rc == LIBEVDEV_READ_STATUS_SYNC)
                        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
                } 
                else if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
                {
                    int button_offset;
                    POV bitset;
                    switch (ev.type)
                    {
                        case EV_KEY:
                            button_offset = ev.code - BTN_TRIGGER;
                            if (button_offset < 0 || button_offset > NUMBER_BUTTONS)
                                break;
                            jsData.buttons[button_offset] = !!ev.value;
                            break;
                        case EV_ABS:
                            switch (ev.code)
                            {
                                case ABS_X:
                                    jsData.x = NormalizeJoystickValue(ev.value, X_MIN, X_MAX);
                                    break;
                                case ABS_Y:
                                    jsData.y = -NormalizeJoystickValue(ev.value, Y_MIN, Y_MAX);
                                    break;
                                case ABS_RZ:
                                    jsData.rz = NormalizeJoystickValue(ev.value, Z_MIN, Z_MAX);
                                    break;
                                case ABS_THROTTLE:
                                    jsData.slider = 100 + (int) (((100.0 / (SLIDER_MAX - SLIDER_MIN)) * (ev.value)));
                                    break;
                                case ABS_HAT0X:
                                    switch (ev.value)
                                    {
                                        case -1:
                                            bitset = POV::POV_WEST;
                                            break;
                                        case 1:
                                            bitset = POV::POV_EAST;
                                            break;
                                        default:
                                            bitset = POV::POV_NONE;
                                            break;
                                    }

                                    // wipe lower two bits and update with bitset
                                    jsData.pov = (POV) (((int)jsData.pov & 0xC) |  (int) bitset);
                                    break;

                                case ABS_HAT0Y:
                                    switch (ev.value)
                                    {
                                        case -1:
                                            bitset = POV::POV_NORTH;
                                            break;
                                        case 1:
                                            bitset = POV::POV_SOUTH;
                                            break;
                                        default:
                                            bitset = POV::POV_NONE;
                                            break;
                                    }

                                    // wipe upper two bits and update with bitset
                                    jsData.pov = (POV) (((int) jsData.pov & 0x3) |  (int) bitset);
                                    break;
                            }
                            break;
                        default:
                            break;        
                    }
                }
            }
        }
        this->rwLock.unlock();
    }
}

void JoystickService::LocateJoysticks(void)
{
    if (this->connectedJoysticks >= this->requestedJoysticks)
        return;
    
    DIR *js_dir = opendir("/dev/input");
    if (!js_dir)
    {
        assert(false);
        return;
    }
    
enumerate_loop:
    struct dirent *devinfo;
    while ((devinfo = readdir(js_dir)) != nullptr)
    {      
        int fd;
        struct libevdev *dev;
        char buffer[30];
        const char *devUniqueID;

        if (!strstr(devinfo->d_name, "event"))
            continue;

        memset(buffer, 0, 30);
        std::snprintf(buffer, 30, "/dev/input/%s", devinfo->d_name);

        auto result = std::find(jsHandleList.begin(), jsHandleList.end(), buffer);
        if (result != jsHandleList.end())
            continue;

        if ((fd = open(buffer, O_RDONLY)) < 0)
            continue;

        if (libevdev_new_from_fd(fd, &dev) < 0)
        {
            close(fd);
            continue;
        }
        
        if (libevdev_get_id_vendor(dev) != JOYSTICK_VENDOR_ID ||
            libevdev_get_id_product(dev) != JOYSTICK_PRODUCT_ID)
        {
            libevdev_free(dev);
            close(fd);
            continue;
        }
        
        // check for device was previously connected
        devUniqueID = libevdev_get_uniq(dev);
        for (auto& pair : this->jsMap)
        {
            const char *curDevUniqueID;
            
            if (pair.second.alive)
                continue;
            
            curDevUniqueID = libevdev_get_uniq((struct libevdev *) pair.second.os_obj);
            
            if (devUniqueID && curDevUniqueID &&
                (strcmp(devUniqueID, curDevUniqueID) == 0))
            {
                // release old handle
                libevdev_free((struct libevdev *) pair.second.os_obj);
                
                pair.second.os_obj = dev;
                pair.second.alive = true;
                this->connectedJoysticks++;
                
                // continue the overall loop
                // but we're nested, so goto
                goto enumerate_loop;
            }
        }

        // configuration successful - add to map
        this->jsMap[this->nextJoystickID] = { };
        this->jsMap[this->nextJoystickID].alive = true;
        this->jsMap[this->nextJoystickID].os_obj = dev;

        jsHandleList.push_back(std::string(devinfo->d_name, strlen(devinfo->d_name)));
        this->nextJoystickID++;
        this->connectedJoysticks++;
    }
    
    closedir(js_dir);
}
