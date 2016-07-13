#include <dinput.h>
#include <windows.h>

#include "joystick.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

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

struct
{
    int *nextJoystickID;
    int *connectedJoysticks;
    int *requestedJoysticks;
    std::map<int, JoystickData> *jsMap;
} EnumerateContext;

LPDIRECTINPUT8 di;

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
    joystick = (LPDIRECTINPUTDEVICE8) context;
    if (FAILED(joystick->SetProperty(DIPROP_RANGE, &propRange.diph)))
        return DIENUM_STOP;

    return DIENUM_CONTINUE;
}

static BOOL CALLBACK EnumerateJoysticks(const DIDEVICEINSTANCE *instance, void *context)
{
    LPDIRECTINPUTDEVICE8 joystick;

    if (*EnumerateContext.connectedJoysticks >= *EnumerateContext.requestedJoysticks)
        return DIENUM_STOP;

    // check if joystick was a formerly removed one
    for (auto& pair : *EnumerateContext.jsMap)
    {
        DIDEVICEINSTANCE info;
        if (pair.second.alive)
            continue;

        LPDIRECTINPUTDEVICE8 inactiveJoystick = (LPDIRECTINPUTDEVICE8) pair.second.os_obj;
        info.dwSize = sizeof(DIDEVICEINSTANCE);
        auto hr = inactiveJoystick->GetDeviceInfo(&info);

        if (info.guidInstance == instance->guidInstance)
        {
            pair.second.alive = true;
            inactiveJoystick->Acquire();
            (*EnumerateContext.connectedJoysticks)++;
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
    (*EnumerateContext.jsMap)[*(EnumerateContext.nextJoystickID)] = { };
    (*EnumerateContext.jsMap)[*(EnumerateContext.nextJoystickID)].alive = true;
    (*EnumerateContext.jsMap)[*(EnumerateContext.nextJoystickID)].os_obj = joystick;
    (*EnumerateContext.nextJoystickID)++;
    (*EnumerateContext.connectedJoysticks)++;

    return DIENUM_CONTINUE;
}

JoystickService::~JoystickService(void)
{
    this->jsPollerStop = true;
    this->jsPoller.join();
    
    for (auto pair : this->jsMap)
    {
        LPDIRECTINPUTDEVICE8 js = (LPDIRECTINPUTDEVICE8) pair.second.os_obj;
        
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

    EnumerateContext.connectedJoysticks = &this->connectedJoysticks;
    EnumerateContext.nextJoystickID = &this->nextJoystickID;
    EnumerateContext.requestedJoysticks = &this->requestedJoysticks;
    EnumerateContext.jsMap = &this->jsMap;
    
    // initialize directinput
    hr = DirectInput8Create(
            GetModuleHandle(nullptr),
            DIRECTINPUT_VERSION,
            IID_IDirectInput8, 
            (void **) &di,
            nullptr
    );
    
    if (FAILED(hr))
        return false;
    
    this->initialized = true;
    return true;
}

bool JoystickService::RemoveJoystick(int joystickID)
{
    if (!this->initialized || !this->IsValidJoystickID(joystickID))
        return false;
    
    this->rwLock.lock();

    LPDIRECTINPUTDEVICE8 joystick = (LPDIRECTINPUTDEVICE8) this->jsMap[joystickID].os_obj;
    joystick->Unacquire();

    this->jsMap[joystickID].alive = false;
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
        HRESULT hr;
        DIJOYSTATE js;
        LPDIRECTINPUTDEVICE8 joystick;
        
        this->rwLock.lock();
        this->LocateJoysticks();
        
        for (auto& pair : this->jsMap)
        {
            if (!pair.second.alive)
                continue;

            JoystickData jsData = pair.second;
            joystick = (LPDIRECTINPUTDEVICE8) jsData.os_obj;
            
            hr = joystick->Poll();
            if (FAILED(hr))
            {
                do 
                {
                    hr = joystick->Acquire();
                }
                while (hr == DIERR_INPUTLOST);

                // joystick fatal error
                if (hr == DIERR_INVALIDPARAM ||
                    hr == DIERR_NOTINITIALIZED ||
                    hr == DIERR_OTHERAPPHASPRIO)
                {
                    // release and invalidate joystick - not found!
                    this->rwLock.unlock();
                    this->RemoveJoystick(pair.first);
                    this->rwLock.lock();
                    continue;
                }
            }

            hr = joystick->GetDeviceState(sizeof(DIJOYSTATE), &js);
            if (FAILED(hr))
            {
                // release and invalidate joystick - not found!
                this->rwLock.unlock();
                this->RemoveJoystick(pair.first);
                this->rwLock.lock();
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
            this->jsMap[pair.first] = jsData;
        }
        
        this->rwLock.unlock();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void JoystickService::LocateJoysticks(void)
{
    di->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumerateJoysticks, nullptr, DIEDFL_ATTACHEDONLY);
}
