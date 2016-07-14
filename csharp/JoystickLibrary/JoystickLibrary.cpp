#include "JoystickLibrary.h"

#include <map>
#include <array>

using namespace JoystickLibrary;

const std::array<POV, 8> povList = {
    POV::POV_NORTH,
    POV::POV_NORTHEAST,
    POV::POV_EAST,
    POV::POV_SOUTHEAST,
    POV::POV_SOUTH,
    POV::POV_SOUTHWEST,
    POV::POV_WEST,
    POV::POV_NORTHWEST,
};

struct JoystickData
{
    bool alive;
    int x;
    int y;
    int rz;
    int slider;
    std::array<bool, NUMBER_BUTTONS> buttons;
    POV pov;

    LPDIRECTINPUTDEVICE8 os_obj;
};

LPDIRECTINPUT8 di;
std::map<int, JoystickData> jsMap;
int requestedJoysticks;
int connectedJoysticks;
int nextJoystickID;

static BOOL CALLBACK JoystickConfigCallback(const DIDEVICEOBJECTINSTANCE *instance, void *context)
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

static BOOL CALLBACK EnumerateJoysticks(const DIDEVICEINSTANCE *instance, void *context)
{
    LPDIRECTINPUTDEVICE8 joystick;

    if (connectedJoysticks >= requestedJoysticks)
        return DIENUM_STOP;

    // check if joystick was a formerly removed one
    for (auto& pair : jsMap)
    {
        DIDEVICEINSTANCE info;
        if (pair.second.alive)
            continue;

        LPDIRECTINPUTDEVICE8 inactiveJoystick = (LPDIRECTINPUTDEVICE8)pair.second.os_obj;
        info.dwSize = sizeof(DIDEVICEINSTANCE);
        auto hr = inactiveJoystick->GetDeviceInfo(&info);

        if (info.guidInstance == instance->guidInstance)
        {
            pair.second.alive = true;
            inactiveJoystick->Acquire();
            connectedJoysticks++;
            return DIENUM_CONTINUE;
        }
    }

    if (FAILED(di->CreateDevice(instance->guidInstance, &joystick, nullptr)))
        return DIENUM_CONTINUE;

    // create and start tracking joystick
    // use DIJOYSTATE struct for data acquisition
    if (FAILED(joystick->SetDataFormat(&c_dfDIJoystick)))
        return DIENUM_CONTINUE;

    // axis configuration to -100 -> 100
    if (FAILED(joystick->EnumObjects(JoystickConfigCallback, joystick, DIDFT_AXIS)))
        return DIENUM_CONTINUE;

    // new joystick - add to map & acquire
    joystick->Acquire();
    jsMap[nextJoystickID] = { };
    jsMap[nextJoystickID].alive = true;
    jsMap[nextJoystickID].os_obj = joystick;
    nextJoystickID++;
    connectedJoysticks++;

    return DIENUM_CONTINUE;
}

JoystickService::JoystickService(int number_joysticks)
{
    requestedJoysticks = number_joysticks;
    connectedJoysticks = 0;
    nextJoystickID = 1;
    this->jsPollerStop = false;
    this->initialized = false;

    this->m_lock = gcnew Object();
}

JoystickService::~JoystickService(void)
{
    this->jsPollerStop = true;
    this->jsPoller->Join();

    for (auto& pair : jsMap)
    {
        LPDIRECTINPUTDEVICE8 js = (LPDIRECTINPUTDEVICE8)pair.second.os_obj;

        if (js)
        {
            js->Unacquire();
            js->Release();
        }
    }
    if (di)
        di->Release();
}

bool JoystickService::Initialize(void)
{
    HRESULT hr;

    if (this->initialized)
        return false;

    // initialize directinput
    hr = DirectInput8Create(
        GetModuleHandle(nullptr),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (void **)&di,
        nullptr
    );

    if (FAILED(hr))
        return false;

    this->initialized = true;
    return true;
}

bool JoystickService::Start(void)
{
    if (!this->initialized)
        return false;

    jsPollerStop = false;
    jsPoller = gcnew Thread(gcnew ThreadStart(this, &JoystickService::PollJoysticks));
    jsPoller->Start();
    return true;
}

bool JoystickService::IsValidJoystickID(int joystickID)
{
    return (jsMap.find(joystickID) != jsMap.end()) && jsMap[joystickID].alive;
}

bool JoystickService::RemoveJoystick(int joystickID)
{
    if (!this->initialized || !this->IsValidJoystickID(joystickID))
        return false;

    msclr::lock l(m_lock);

    jsMap[joystickID].os_obj->Unacquire();
    jsMap[joystickID].alive = false;
    connectedJoysticks--;

    return true;
}

void JoystickService::PollJoysticks(void)
{
    if (!this->initialized)
        return;

    while (!this->jsPollerStop)
    {
        HRESULT hr;
        DIJOYSTATE js;
        LPDIRECTINPUTDEVICE8 joystick;

        msclr::lock l(m_lock);
        this->LocateJoysticks();

        for (auto& pair : jsMap)
        {
            if (!pair.second.alive)
                continue;

            JoystickData& jsData = pair.second;
            joystick = (LPDIRECTINPUTDEVICE8)jsData.os_obj;

            hr = joystick->Poll();
            if (FAILED(hr))
            {
                do
                {
                    hr = joystick->Acquire();
                } while (hr == DIERR_INPUTLOST);

                // joystick fatal error
                if (hr == DIERR_INVALIDPARAM ||
                    hr == DIERR_NOTINITIALIZED ||
                    hr == DIERR_OTHERAPPHASPRIO)
                {
                    // release and invalidate joystick - not found!
                    jsData.os_obj->Unacquire();
                    jsData.alive = false;
                    connectedJoysticks--;
                    continue;
                }
            }

            hr = joystick->GetDeviceState(sizeof(DIJOYSTATE), &js);
            if (FAILED(hr))
            {
                // release and invalidate joystick - not found!
                jsData.os_obj->Unacquire();
                jsData.alive = false;
                connectedJoysticks--;
                continue;
            }

            jsData.x = js.lX;
            jsData.y = -js.lY; // y is backwards for some reason
            jsData.rz = js.lRz;

            // slider goes from -100 to 100, but it makes more sense from 0 -> 100.
            // it's also backwards. The bottom part of the slider was 100, and the top -100.
            // the formula below corrects this.
            jsData.slider = (100 - js.rglSlider[0]) / 2;

            // POV values are done in 45 deg segments (0 is up) * 100.
            // i.e. straight right = 90 deg = 9000.
            // determine POV value by way of lookup table (divide by 4500)
            unsigned int povListIndex = js.rgdwPOV[0] / 4500;
            jsData.pov = (povListIndex < povList.size()) ? povList[povListIndex] : POV::POV_NONE;

            for (int i = 0; i < NUMBER_BUTTONS; i++)
                jsData.buttons[i] = !!js.rgbButtons[i];

            // joystick read complete
            jsMap[pair.first] = jsData;
        }

        Thread::Sleep(100);
    }
}

void JoystickService::LocateJoysticks(void)
{
    di->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumerateJoysticks, nullptr, DIEDFL_ATTACHEDONLY);
}

List<int>^ JoystickService::GetJoystickIDs(void)
{
    List<int>^ ids = gcnew List<int>();

    for (auto& pair : jsMap)
        if (pair.second.alive)
            ids->Add(pair.first);

    return ids;
}

int JoystickService::GetConnectedJoysticksCount(void)
{
    return connectedJoysticks;
}

bool JoystickService::GetX(int joystickID, Int32% x)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    x = jsMap[joystickID].x;
    return true;
}

bool JoystickService::GetY(int joystickID, Int32% y)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    y = jsMap[joystickID].y;
    return true;
}

bool JoystickService::GetZRot(int joystickID, Int32% zRot)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    zRot = jsMap[joystickID].rz;
    return true;
}

bool JoystickService::GetSlider(int joystickID, Int32% slider)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    slider = jsMap[joystickID].slider;
    return true;
}

bool JoystickService::GetButtons(int joystickID, array<bool>^% buttons)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    buttons = gcnew array<bool>(NUMBER_BUTTONS);
    for (int i = 0; i < NUMBER_BUTTONS; i++)
        buttons[i] = jsMap[joystickID].alive && jsMap[joystickID].buttons[i];

    return true;
}

bool JoystickService::GetPOV(int joystickID, POV^% pov)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    pov = jsMap[joystickID].pov;
    return true;
}
