#include "Enumerator.hpp"

using namespace JoystickLibrary;

constexpr wchar_t *ENUMERATOR_CLASS_NAME = L"__JL_Enumerator";
constexpr wchar_t *ENUMERATOR_WND_NAME = L"__JL_Enumerator_Window";

const GUID HID_CLASS_GUID = { 0x4d1e55b2, 0xf16f, 0x11Cf, { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 }};


struct DIEnumerationContext
{
    int nextJoystickID;
    int connectedJoysticks;
    std::map<int, JoystickData> *jsMap;
    std::vector<DeviceChangeCallback> *callbacks;
    LPDIRECTINPUT8 di;
};

static LRESULT CALLBACK EnumWndMsgProc(
    _In_ HWND   hWnd,
    _In_ UINT   message,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    switch (message)
    {
        case WM_DEVICECHANGE:
        {
            auto& enumerator = Enumerator::GetInstance();

            switch (wParam)
            {
                case DBT_DEVICEARRIVAL:
                    // new HID arrived, enumerate it
                    enumerator.__run_enum();
                    break;
                case DBT_DEVICEREMOVECOMPLETE:
                    // some HID left, find it and set it to inactive
                    enumerator.__run_remove();
                    break;
            }
            break;
        }
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return TRUE;
}

static DWORD WINAPI EnumThread(
    _In_ LPVOID lpParameter
)
{
    // message pump
    MSG msg;
    int retVal;

    EnumeratorImpl *impl;
    WNDCLASSEX wx;
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    impl = (EnumeratorImpl *)lpParameter;

    // setup the background message window
    memset(&wx, 0, sizeof(WNDCLASSEX));
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = &EnumWndMsgProc;
    wx.hInstance = GetModuleHandle(nullptr);
    wx.lpszClassName = ENUMERATOR_CLASS_NAME;

    if (!RegisterClassEx(&wx))
        return false;

    impl->enumerationhWnd = CreateWindowEx(0, ENUMERATOR_CLASS_NAME, ENUMERATOR_WND_NAME, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (!impl->enumerationhWnd)
        return false;

    // tell Windows to send us the device change notification
    memset(&NotificationFilter, 0, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = HID_CLASS_GUID;
    impl->enumerationHNotify = RegisterDeviceNotification(impl->enumerationhWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

    // pump messages
    while ((retVal = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (retVal == -1)
            throw std::runtime_error("Message pump failed!");

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return TRUE;
}

static BOOL CALLBACK JoystickConfigCallback(
    LPCDIDEVICEOBJECTINSTANCE  instance,
    LPVOID context
)
{
    DIPROPRANGE propRange;
    LPDIRECTINPUTDEVICE8 joystick;

    if (!context)
        return DIENUM_STOP;

    memset(&propRange, 0, sizeof(DIPROPRANGE));
    propRange.diph.dwSize = sizeof(DIPROPRANGE);
    propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    propRange.diph.dwHow = DIPH_BYID;
    propRange.diph.dwObj = instance->dwType;
    propRange.lMin = -100;
    propRange.lMax = +100;

    // Set the range for the axis
    joystick = (LPDIRECTINPUTDEVICE8)context;
    if (FAILED(joystick->SetProperty(DIPROP_RANGE, &propRange.diph)))
        return DIENUM_STOP;

    return DIENUM_CONTINUE;
}

static BOOL CALLBACK EnumerateJoysticks(
    LPCDIDEVICEINSTANCE instance, 
    LPVOID context
)
{
    DIEnumerationContext *info = (DIEnumerationContext *)context;
    LPDIRECTINPUTDEVICE8 joystick;
    DIPROPDWORD dipdw;

    DIPROPGUIDANDPATH jsGuidPath;
    jsGuidPath.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
    jsGuidPath.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    jsGuidPath.diph.dwHow = DIPH_DEVICE;
    jsGuidPath.diph.dwObj = 0;

    if (FAILED(info->di->CreateDevice(instance->guidInstance, &joystick, nullptr)))
        return DIENUM_CONTINUE;

    if (FAILED(joystick->GetProperty(DIPROP_GUIDANDPATH, &jsGuidPath.diph)))
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

    // check if joystick was a formerly removed one
    for (auto& pair : *(info->jsMap))
    {
        DIPROPGUIDANDPATH hw_info;
        LPDIRECTINPUTDEVICE8 inactiveJoystick;

        inactiveJoystick = pair.second.handle;
        hw_info.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
        hw_info.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        hw_info.diph.dwHow = DIPH_DEVICE;
        hw_info.diph.dwObj = 0;

        if (FAILED(inactiveJoystick->GetProperty(DIPROP_GUIDANDPATH, &hw_info.diph)))
            continue;

        // path match
        if (hw_info.wszPath && jsGuidPath.wszPath && lstrcmp(hw_info.wszPath, jsGuidPath.wszPath) == 0)
        {
            // if this path is already active, don't enumerate
            if (pair.second.alive)
            {
                // no need to issue enumeration messages
                // if something is already alive, guaranteed to have been covered
                // by other code paths
                joystick->Release();
                return DIENUM_CONTINUE;
            }

            pair.second.alive = true;
            inactiveJoystick->Acquire();
            joystick->Release();
            
            DeviceStateChange dsc;
            dsc.descriptor = pair.second.descriptor;
            dsc.id = pair.first;
            dsc.state = DeviceStateChange::State::ADDED;
            for (auto callback : *(info->callbacks))
                callback(dsc);

            info->connectedJoysticks++;
            return DIENUM_CONTINUE;
        }
    }

    // get vendor and product ID
    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;

    if (FAILED(joystick->GetProperty(DIPROP_VIDPID, &dipdw.diph)))
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

    int vendor_id = LOWORD(dipdw.dwData);
    int product_id = HIWORD(dipdw.dwData);

    // create and start tracking joystick
    // use DIJOYSTATE struct for data acquisition
    if (FAILED(joystick->SetDataFormat(&c_dfDIJoystick)))
        return DIENUM_CONTINUE;

    // axis configuration to -100 -> 100
    if (FAILED(joystick->EnumObjects(&JoystickConfigCallback, joystick, DIDFT_AXIS)))
        return DIENUM_CONTINUE;

    // new joystick - add to map & acquire
    joystick->Acquire();
    (*(info->jsMap))[(info->nextJoystickID)] = {};
    (*(info->jsMap))[(info->nextJoystickID)].alive = true;
    (*(info->jsMap))[(info->nextJoystickID)].handle = joystick;
    (*(info->jsMap))[(info->nextJoystickID)].descriptor = { vendor_id, product_id };

    // issue callbacks
    DeviceStateChange dsc;
    dsc.descriptor= { vendor_id, product_id };
    dsc.id = info->nextJoystickID;
    dsc.state = DeviceStateChange::State::ADDED;
    for (auto callback : *(info->callbacks))
        callback(dsc);

    info->connectedJoysticks++;
    info->nextJoystickID++;
    return DIENUM_CONTINUE;
}

Enumerator::Enumerator()
{
    this->started = false;
    this->nextJoystickID = 0;
    this->connectedJoysticks = 0;
    this->impl = new EnumeratorImpl;
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

Enumerator::~Enumerator()
{
    this->started = false;
    if (this->impl)
        delete this->impl;
}

bool Enumerator::Start()
{
    HRESULT hr;

    if (started)
    {
        this->__run_enum();
        return true;
    }

    // start DirectInput
    hr = DirectInput8Create(
        GetModuleHandle(nullptr),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (LPVOID *)&impl->di,
        nullptr
    );

    if (FAILED(hr))
        return false;

    // start the listener
    impl->enumThread = CreateThread(nullptr, 0, &EnumThread, impl, 0, nullptr);
    if (!impl->enumThread)
        return false;

    this->started = true;

    // initial enumeration
    this->__run_enum();
    return true;
}

int JoystickLibrary::Enumerator::GetNumberConnected()
{
    return this->connectedJoysticks;
}

void Enumerator::__run_enum(const void *)
{
    DIEnumerationContext context;

    if (!started)
        return;

    context.jsMap = &(this->impl->jsMap);
    context.nextJoystickID = this->nextJoystickID;
    context.di = this->impl->di;
    context.callbacks = &(this->callbacks);
    context.connectedJoysticks = this->connectedJoysticks;

    impl->di->EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerateJoysticks, &context, DIEDFL_ATTACHEDONLY);

    this->connectedJoysticks = context.connectedJoysticks;
    this->nextJoystickID = context.nextJoystickID;
}

void Enumerator::__run_remove(const void *)
{
    if (!started)
        return;

    // potentially expensive - better way? //
    // poll every device to see who was removed //
    for (auto& pair : impl->jsMap)
    {
        HRESULT hr;
        DIJOYSTATE di_js;

        auto& jsState = pair.second;
        if (!jsState.alive)
            continue;

        auto handle = jsState.handle;
        hr = handle->GetDeviceState(sizeof(DIJOYSTATE), &di_js);
        if (SUCCEEDED(hr))
            continue;

        // joystick gone, so set to false & notify
        DeviceStateChange dsc;
        dsc.state = DeviceStateChange::State::REMOVED;
        dsc.id = pair.first;
        dsc.descriptor = jsState.descriptor;
        jsState.alive = false;

        for (auto callback : this->callbacks)
            callback(dsc);

        this->connectedJoysticks--;
    }
}
