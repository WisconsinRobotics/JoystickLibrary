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


#include <array>
#include "joystick.h"

using namespace JoystickLibrary;


struct
{
    int *nextJoystickID;
    int *requestedJoysticks;
    int *connectedJoysticks;
    int *vendor_id;
    int *product_id;
    std::map<int, JoystickData> *jsMap;
} EnumerateContext;

LPDIRECTINPUT8 di;

/**
 * Callback to configure the axis values on the joystick.
 * @param instance A pointer to the device instance
 * @param context A pointer to the joystick instance
 * @return DIENUM_CONTINUE on success, DIENUM_STOP otherwise
 */
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

/**
 * A callback function called for each found joystick.
 * @param instance A pointer to the device instance.
 * @param context (unused)
 * @return DIENUM_CONTINUE on success, DIENUM_STOP otherwise
 */
static BOOL CALLBACK EnumerateJoysticks(const DIDEVICEINSTANCE *instance, void *context)
{
    LPDIRECTINPUTDEVICE8 joystick;
    DIPROPDWORD dipdw;

    // context is not used
    UNREFERENCED_PARAMETER(context); 

    if (*EnumerateContext.connectedJoysticks >= *EnumerateContext.requestedJoysticks)
        return DIENUM_STOP;
    
    if (FAILED(di->CreateDevice(instance->guidInstance, &joystick, nullptr)))
        return DIENUM_CONTINUE;
    
    DIPROPGUIDANDPATH jsGuidPath;
    jsGuidPath.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
    jsGuidPath.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    jsGuidPath.diph.dwHow = DIPH_DEVICE;
    jsGuidPath.diph.dwObj = 0;   
    
    if (FAILED(joystick->GetProperty(DIPROP_GUIDANDPATH, &jsGuidPath.diph)))
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }
    
    // check if joystick was a formerly removed one
    for (auto& pair : *EnumerateContext.jsMap)
    {
        DIPROPGUIDANDPATH info;
        LPDIRECTINPUTDEVICE8 inactiveJoystick;
        
        inactiveJoystick = (LPDIRECTINPUTDEVICE8) pair.second.os_obj;
        info.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
        info.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        info.diph.dwHow = DIPH_DEVICE;
        info.diph.dwObj = 0;
        
        if (FAILED(inactiveJoystick->GetProperty(DIPROP_GUIDANDPATH, &info.diph)))
            continue;
        
        // path match
        if (info.wszPath && jsGuidPath.wszPath && lstrcmp(info.wszPath, jsGuidPath.wszPath) == 0)
        {
            // if this path is already active, don't enumerate
            if (pair.second.alive)
            {
                joystick->Release();
                return DIENUM_CONTINUE;
            }

            pair.second.alive = true;
            inactiveJoystick->Acquire();
            (*EnumerateContext.connectedJoysticks)++;
            joystick->Release();
            return DIENUM_CONTINUE;
        }
    }

    // check vendor and product ID
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD); 
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;

    if (FAILED(joystick->GetProperty(DIPROP_VIDPID, &dipdw.diph)))
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

    if (LOWORD(dipdw.dwData) != *EnumerateContext.vendor_id || HIWORD(dipdw.dwData) != *EnumerateContext.product_id)
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

    // create and start tracking joystick
    // use DIJOYSTATE struct for data acquisition
    if (FAILED(joystick->SetDataFormat(&c_dfDIJoystick)))
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

    // axis configuration to -100 -> 100
    if (FAILED(joystick->EnumObjects(JoystickConfigCallback, joystick, DIDFT_AXIS)))
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

    // new joystick - add to map & acquire
    joystick->Acquire();
    (*EnumerateContext.jsMap)[*(EnumerateContext.nextJoystickID)] = { };
    (*EnumerateContext.jsMap)[*(EnumerateContext.nextJoystickID)].alive = true;
    (*EnumerateContext.jsMap)[*(EnumerateContext.nextJoystickID)].os_obj = joystick;
    (*EnumerateContext.nextJoystickID)++;
    (*EnumerateContext.connectedJoysticks)++;

    return DIENUM_CONTINUE;
}

JoystickService::~JoystickService()
{
    this->jsPollerStop = true;
    
    if (this->jsPoller.joinable())
        this->jsPoller.join();  
    
    for (auto& pair : this->jsMap)
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

bool JoystickService::Initialize()
{
    HRESULT hr;
    
    if (this->initialized)
        return false;

    EnumerateContext.connectedJoysticks = &this->connectedJoysticks;
    EnumerateContext.nextJoystickID = &this->nextJoystickID;
    EnumerateContext.requestedJoysticks = &this->requestedJoysticks;
    EnumerateContext.product_id = &this->product_id;
    EnumerateContext.vendor_id = &this->vendor_id;
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

void JoystickService::PollJoysticks()
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

            JoystickData& jsData = pair.second;
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

            // joystick read complete
            this->jsMap[pair.first].state = js;
        }
        
        this->rwLock.unlock();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void JoystickService::LocateJoysticks()
{
    di->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumerateJoysticks, nullptr, DIEDFL_ATTACHEDONLY);
}
