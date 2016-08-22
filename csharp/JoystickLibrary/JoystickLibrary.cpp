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
    std::array<bool, JoystickService::NUMBER_BUTTONS> buttons;
    POV pov;

    LPDIRECTINPUTDEVICE8 os_obj;
};

LPDIRECTINPUT8 di;
std::map<int, JoystickData> jsMap;
int requestedJoysticks;
int connectedJoysticks;
int nextJoystickID;

/**
 Callback to configure the axis values on the joystick.
 @param instance A pointer to the device instance
 @param context A pointer to the joystick instance
 @return DIENUM_CONTINUE on success, DIENUM_STOP otherwise
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
    joystick = (LPDIRECTINPUTDEVICE8)context;
    if (FAILED(joystick->SetProperty(DIPROP_RANGE, &propRange.diph)))
        return DIENUM_STOP;

    return DIENUM_CONTINUE;
}

/**
  A callback function called for each found joystick.
  @param instance A pointer to the device instance.
  @param context (unused)
  @return DIENUM_CONTINUE on success, DIENUM_STOP otherwise
 */
static BOOL CALLBACK EnumerateJoysticks(const DIDEVICEINSTANCE *instance, void *context)
{
    LPDIRECTINPUTDEVICE8 joystick;
    DIPROPDWORD dipdw;

    if (connectedJoysticks >= requestedJoysticks)
        return DIENUM_STOP;

    DIPROPGUIDANDPATH jsGuidPath;
    jsGuidPath.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
    jsGuidPath.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    jsGuidPath.diph.dwHow = DIPH_DEVICE;
    jsGuidPath.diph.dwObj = 0;

    if (FAILED(di->CreateDevice(instance->guidInstance, &joystick, nullptr)))
        return DIENUM_CONTINUE;

    if (FAILED(joystick->GetProperty(DIPROP_GUIDANDPATH, &jsGuidPath.diph)))
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

    // check if joystick was a formerly removed one
    for (auto& pair : jsMap)
    {
        DIPROPGUIDANDPATH info;
        LPDIRECTINPUTDEVICE8 inactiveJoystick;

        inactiveJoystick = (LPDIRECTINPUTDEVICE8)pair.second.os_obj;
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
            connectedJoysticks++;
            joystick->Release();
            return DIENUM_CONTINUE;
        }
    }

    // check vendor and product ID
    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;

    if (FAILED(joystick->GetProperty(DIPROP_VIDPID, &dipdw.diph)))
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

    if (LOWORD(dipdw.dwData) != JoystickService::JOYSTICK_VENDOR_ID || HIWORD(dipdw.dwData) != JoystickService::JOYSTICK_PRODUCT_ID)
    {
        joystick->Release();
        return DIENUM_CONTINUE;
    }

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

    if (this->jsPoller->IsAlive)
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

bool JoystickService::GetButton(int joystickID, Button button, Boolean% buttonVal)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    buttonVal = jsMap[joystickID].buttons[(int) button];
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

bool JoystickService::GetPOV(int joystickID, POV% pov)
{
    if (!this->initialized || !IsValidJoystickID(joystickID))
        return false;

    pov = jsMap[joystickID].pov;
    return true;
}
