#include "JoystickService.hpp"
#include <iostream>

using namespace JoystickLibrary;


JoystickService::JoystickService()
{
    this->initialized = false;   
}

JoystickService::~JoystickService()
{
}

bool JoystickLibrary::JoystickService::Initialize()
{
    if (this->initialized)
        return true;

    enumerator.RegisterInstance(std::bind(&JoystickService::OnDeviceChanged, this, std::placeholders::_1));
    bool success = enumerator.Start();
    this->initialized = success;
    return success;
}

int JoystickLibrary::JoystickService::GetNumberConnected() const
{
    return this->ids.size();
}

const std::vector<int>& JoystickService::GetIDs() const
{
    return this->ids;
}

bool JoystickService::IsValidJoystickID(int id) const
{
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

JoystickState JoystickLibrary::JoystickService::GetState(int id) const
{
#ifdef _WIN32
    DIJOYSTATE js;
    HRESULT hr;
    hr = enumerator.impl->jsMap[id].handle->GetDeviceState(sizeof(JoystickState), &js);
    if (FAILED(hr))
        memset(&js, 0, sizeof(DIJOYSTATE));
    enumerator.impl->jsMap[id].state = js;
    return js;
#else
    enumerator.impl->jsMapLock.lock();
    JoystickData& jsData = enumerator.impl->jsMap[id];
    struct libevdev *dev = jsData.handle.dev;

    // read the joystick state
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(jsData.handle.fd, &fds);
    struct timeval tv = { 0, 0 };

    while (select(jsData.handle.fd + 1, &fds, nullptr, nullptr, &tv) > 0)
    {
        struct input_event ev;
        int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING, &ev);

        if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
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
        else if (rc == LIBEVDEV_READ_STATUS_SYNC)
        {
            // joy state became unsync'd, so perform a resync
            while (true)
            {
                rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC | LIBEVDEV_READ_FLAG_BLOCKING, &ev);
                if (rc != LIBEVDEV_READ_STATUS_SYNC)
                    break;
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
        }
        else
        {
            // set this one to inactive
            jsData.alive = false;
            close(jsData.handle.fd);
            enumerator.connectedJoysticks--;

            // issue callbacks
            DeviceStateChange dsc;
            dsc.state = DeviceStateChange::State::REMOVED;
            dsc.id = id;
            dsc.descriptor = jsData.descriptor;
            for (auto callback : enumerator.callbacks)
                callback(dsc);
            enumerator.impl->jsMapLock.unlock();
            return JoystickState();
        }

        FD_ZERO(&fds);
        FD_SET(jsData.handle.fd, &fds);
        tv = { 0, 100 };
    }
    enumerator.impl->jsMapLock.unlock();

    return enumerator.impl->jsMap[id].state;
#endif
}

void JoystickService::ProcessDeviceChange(std::vector<JoystickDescriptor> id_list, DeviceStateChange dsc)
{
    auto it = std::find(id_list.begin(), id_list.end(), dsc.descriptor);
    if (it == id_list.end())
        return;

    auto id_itr = std::find(ids.begin(), ids.end(), dsc.id);

    if (dsc.state == DeviceStateChange::State::ADDED)
    {
        if (id_itr == ids.end())
            this->ids.push_back(dsc.id);
    }
    else
    {
        if (id_itr != ids.end())
            ids.erase(id_itr);
    }
}

#ifndef _WIN32
// on linux, use ioctl to get initial states for joysticks
int JoystickLibrary::JoystickService::GetAxis(int id, int axisId) const
{
    std::map<int, int> axes = this->GetState(id).axes;
    // look up a cached value
    std::map<int, int>::iterator axisEntry = axes.find(axisId);
    if (axisEntry != axes.end())
    {
        // cache hit!
        return axisEntry->second;
    }
    // if there's nothing, retrieve the current value
    enumerator.impl->jsMapLock.lock();
    int axisValue = libevdev_get_event_value(enumerator.impl->jsMap[id].handle.dev, EV_ABS, axisId);
    axes[axisId] = axisValue;
    enumerator.impl->jsMapLock.unlock();
    return axisValue;
}
#endif
