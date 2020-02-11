#include "Enumerator.hpp"
#include <cstdio>
#include <iostream>

using namespace JoystickLibrary;

constexpr const char *DEVICE_PATH_TEMPLATE = "/dev/input/%s";
constexpr const char *DEVICE_ADDED = "add";
constexpr const char *DEVICE_REMOVED = "remove";


Enumerator::Enumerator()
{
    this->started = false;
    this->nextJoystickID = 0;
    this->connectedJoysticks = 0;
    this->impl = new EnumeratorImpl;
}

Enumerator::~Enumerator()
{
    this->started = false;
    if (this->impl)
        delete this->impl;
}

void Enumerator::RegisterInstance(DeviceChangeCallback callback)
{
    if (callback)
        this->callbacks.push_back(callback);

    for (auto& pair : this->impl->jsMap)
    {
        if (!pair.second.alive)
            continue;

        DeviceStateChange dsc;
        dsc.descriptor = pair.second.descriptor;
        dsc.id = pair.first;
        dsc.state = DeviceStateChange::State::ADDED;
        callback(dsc);
    }
}

bool Enumerator::Start()
{
    if (started)
    {
        this->__run_enum();
        return true;
    }

    // init udev
    this->impl->udev = udev_new();
    if (!this->impl->udev)
        return false;

    this->impl->udev_monitor = udev_monitor_new_from_netlink(this->impl->udev, "udev");
    if (!this->impl->udev_monitor)
        return false;

    // set monitors 
    udev_monitor_filter_add_match_subsystem_devtype(this->impl->udev_monitor, "hid", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(this->impl->udev_monitor, "input", NULL);
    udev_monitor_enable_receiving(this->impl->udev_monitor);
    this->impl->udev_mon_fd = udev_monitor_get_fd(this->impl->udev_monitor);

    this->started = true;

    // initial enumeration
    this->__run_enum();

    // create "self-pipe" to udev selector
    pipe(this->impl->udev_select_pipe);

    // init udev thread
    this->impl->deviceListenerThread = std::thread(&Enumerator::udev_thread, this);

    // init evdev thread
    //this->impl->evdevEventThread = std::thread(&Enumerator::evdev_thread, this);

    return true;
}

int JoystickLibrary::Enumerator::GetNumberConnected()
{
    return this->connectedJoysticks;
}

void Enumerator::__run_enum(const void *context)
{
    int fd;
    struct libevdev *dev;
    const char *devnode_path;

    if (!started || !context)
        return;

    devnode_path = (const char *) context;
    if ((fd = open(devnode_path, O_RDONLY)) < 0)
        return;
        
    if (libevdev_new_from_fd(fd, &dev) < 0)
    {
        close(fd);
        return;
    }
            
    int vendor_id = libevdev_get_id_vendor(dev);
    int product_id =  libevdev_get_id_product(dev);
    
    impl->jsMapLock.lock();
    // check for device was previously connected
    for (auto& pair : this->impl->jsMap)
    {
        bool devnode_match = strcmp(pair.second.handle.path, devnode_path) == 0; 

        bool id_match = (vendor_id == pair.second.descriptor.vendor_id)
            && (product_id == pair.second.descriptor.product_id);

        if (devnode_match && id_match)
        {
            if (pair.second.alive)
            {
                libevdev_free(dev);
                impl->jsMapLock.unlock();
                return;
            }

            // release old handle
            libevdev_free(pair.second.handle.dev);
            
            // re-enable
            pair.second.handle.fd = fd;
            pair.second.handle.dev = dev;
            pair.second.alive = true;
            this->connectedJoysticks++;

            // issue callbacks
            DeviceStateChange dsc;
            dsc.descriptor= { vendor_id, product_id };
            dsc.id = pair.first;
            dsc.state = DeviceStateChange::State::ADDED;
            for (auto callback : this->callbacks)
                callback(dsc);
            impl->jsMapLock.unlock();
            return;
        }
    }

    // new device - add to map
    JoystickHandle new_handle;
    memset(&new_handle, 0, sizeof(JoystickHandle));
    new_handle.dev = dev;
    new_handle.fd = fd;
    strncpy(new_handle.path, devnode_path, sizeof(new_handle.path));

    this->impl->jsMap[this->nextJoystickID] = { };
    this->impl->jsMap[this->nextJoystickID].alive = true;
    this->impl->jsMap[this->nextJoystickID].handle = new_handle;
    this->impl->jsMap[this->nextJoystickID].descriptor = { vendor_id, product_id };
    
    // issue callbacks
    DeviceStateChange dsc;
    dsc.descriptor= { vendor_id, product_id };
    dsc.id = this->nextJoystickID;
    dsc.state = DeviceStateChange::State::ADDED;
    for (auto callback : this->callbacks)
        callback(dsc);

    this->nextJoystickID++;
    this->connectedJoysticks++;
    impl->jsMapLock.unlock();
}

void Enumerator::__run_remove(const void *context)
{
    if (!started || context)
        return;
    
    const char *removed_name = (const char *)context;

    for (auto& pair : this->impl->jsMap)
    {
        if (!pair.second.alive)
            continue;

        if (strcmp(removed_name, pair.second.handle.path) == 0)
        {
            // found the removed device
            pair.second.alive = false;
            close(pair.second.handle.fd);
            this->connectedJoysticks--;

            // issue callbacks
            DeviceStateChange dsc;
            dsc.state = DeviceStateChange::State::REMOVED;
            dsc.id = pair.first;
            dsc.descriptor = pair.second.descriptor;
            for (auto callback : this->callbacks)
                callback(dsc);
        }
    }
    
}

void Enumerator::udev_thread()
{
    udev_enumerate *enumerate;
    udev_list_entry *devices, *dev_list_entry;
 
    // first run enumeration //
	enumerate = udev_enumerate_new(impl->udev);
	udev_enumerate_add_match_sysname(enumerate, "event[0-9]*");
    udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(dev_list_entry, devices) 
	{
		const char *path;
        udev_device *dev;
		
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(impl->udev, path);
        this->__run_enum(udev_device_get_devnode(dev));
		udev_device_unref(dev);
	}
    udev_enumerate_unref(enumerate);

    // steady state //
	while (this->started) 
    {
		fd_set fds;
		int ret;
        const char *devnode;
        const char *action;
        udev_device *dev;
		
		FD_ZERO(&fds);
        int udev_fd = this->impl->udev_mon_fd;
        int pipe_fd = this->impl->udev_select_pipe[0];
		FD_SET(udev_fd, &fds);
        FD_SET(pipe_fd, &fds);
		ret = select(std::max(udev_fd, pipe_fd) + 1, &fds, NULL, NULL, NULL);

        // if we receive something and `started` is false, it's probably
        // the "break-out-of-select" signal, so exit the loop immediately
        if (!this->started)
            break;

		if (ret <= 0 || !FD_ISSET(this->impl->udev_mon_fd, &fds))
            continue;

        dev = udev_monitor_receive_device(this->impl->udev_monitor);
        if (!dev)
            continue;
        
        devnode = udev_device_get_devnode(dev);
        if (!devnode)
            continue;

        if (strstr(devnode, "event") == NULL)
            continue;

        action = udev_device_get_action(dev);
        if (strcmp(action, DEVICE_ADDED) == 0)
            this->__run_enum(devnode);
        
        udev_device_unref(dev);
	}
}
