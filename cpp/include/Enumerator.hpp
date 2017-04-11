#pragma once
#include "Types.hpp"

namespace JoystickLibrary
{
    typedef std::function<void(DeviceStateChange)> DeviceChangeCallback;

    struct EnumeratorImpl
    {
        // common implementation fields //
        std::map<int, JoystickData> jsMap;

#ifdef _WIN32
        HWND enumerationhWnd;
        HDEVNOTIFY enumerationHNotify;
        HANDLE enumThread;
        LPDIRECTINPUT8 di;

        EnumeratorImpl::EnumeratorImpl()
        {
            enumerationhWnd = nullptr;
            enumerationHNotify = nullptr;
            di = nullptr;
            enumThread = nullptr;
        }

        EnumeratorImpl::~EnumeratorImpl()
        {
            if (enumerationhWnd)
                DestroyWindow(enumerationhWnd);
            if (enumerationHNotify)
                UnregisterDeviceNotification(enumerationHNotify);
            if (di)
                di->Release();
            if (enumThread)
                TerminateThread(enumThread, 0);

            enumerationhWnd = nullptr;
            enumerationHNotify = nullptr;
            di = nullptr;
            enumThread = nullptr;
        }
#elif __linux__
        struct udev *udev;
        struct udev_monitor *udev_monitor;
        int udev_mon_fd;
        std::thread deviceListenerThread;
        std::mutex jsMapLock;

        EnumeratorImpl()
        {
            udev = nullptr;
            udev_monitor = nullptr;
        }

        ~EnumeratorImpl()
        {
            if (udev)
                udev_unref(udev);
            if (udev_monitor)
                udev_monitor_unref(udev_monitor);
            if (deviceListenerThread.joinable())
                deviceListenerThread.join();
        }
#else
        #error Not currently supported!
#endif
    };

    class Enumerator
    {
        friend class JoystickService;
    public:
        static Enumerator& GetInstance()
        {
            static Enumerator instance;
            return instance;
        }

        Enumerator(Enumerator const&) = delete;
        void operator=(Enumerator const&) = delete;
        ~Enumerator();
        bool Start();
        int GetNumberConnected();
        void __run_enum(const void *context = nullptr);
        void __run_remove(const void *context = nullptr);

    private:
        Enumerator();

        void RegisterInstance(DeviceChangeCallback callback);

#ifdef __linux__
        void udev_thread();
        void evdev_thread();
#endif

        EnumeratorImpl *impl;
        std::vector<DeviceChangeCallback> callbacks;
        bool started;
        int connectedJoysticks;
        int nextJoystickID;
    };
}

