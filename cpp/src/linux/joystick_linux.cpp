/*
 * Copyright (c) 2016, Wisconsin Robotics
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of Wisconsin Robotics nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *   
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL WISCONSIN ROBOTICS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <cstring>
#include "joystick.h"

using namespace JoystickLibrary;


std::thread enumerateThread;

JoystickService::~JoystickService(void)
{
    this->jsPollerStop = true;
    
    if (this->jsPoller.joinable())
        this->jsPoller.join();
    
    if (enumerateThread.joinable())
        enumerateThread.join();

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

    enumerateThread = std::thread(&JoystickService::LocateJoysticks, this);

    while (!this->jsPollerStop)
    {
        this->rwLock.lock();
        
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
                    switch (ev.type)
                    {
                        case EV_KEY:
                            jsData.state.buttons[ev.code] = !!ev.value;
                            break;
                        case EV_ABS:
                            jsData.state.axes[ev.code] = ev.value;
                            break;
                        default:
                            break;        
                    }
                }
                else
                {
                    this->rwLock.unlock();
                    this->RemoveJoystick(pair.first);
                    this->rwLock.lock();
                    break;
                }
            }
        }
        this->rwLock.unlock();
    }
}

void JoystickService::LocateJoysticks(void)
{
    while (!this->jsPollerStop) 
    {
        if (this->connectedJoysticks >= this->requestedJoysticks)
            continue;
        
        DIR *js_dir = opendir("/dev/input");
        if (!js_dir)
            return;
        
    enumerate_loop:
        // This loop is the equivalent of doing:
        // ls  /dev/input/event* and then checking each one
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

            if ((fd = open(buffer, O_RDONLY)) < 0)
                continue;

            if (libevdev_new_from_fd(fd, &dev) < 0)
            {
                close(fd);
                continue;
            }
            
            if (libevdev_get_id_vendor(dev) != this->vendor_id ||
                libevdev_get_id_product(dev) != this->product_id)
            {
                libevdev_free(dev);
                close(fd);
                continue;
            }
            
            // check for device was previously connected
            devUniqueID = libevdev_get_phys(dev);
            for (auto& pair : this->jsMap)
            {
                const char *curDevUniqueID;
                
                curDevUniqueID = libevdev_get_phys((struct libevdev *) pair.second.os_obj);
                
                if (devUniqueID && curDevUniqueID &&
                    (strcmp(devUniqueID, curDevUniqueID) == 0))
                {
                    if (pair.second.alive)
                    {
                        libevdev_free(dev);
                        goto enumerate_loop;
                    }

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

            this->nextJoystickID++;
            this->connectedJoysticks++;
        }
        
        closedir(js_dir);
    }
}
