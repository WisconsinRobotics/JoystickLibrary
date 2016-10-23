#pragma once

#include "joystick.h"
#include <iostream>
#include <string>

using JoystickLibrary::Xbox360Service;
using JoystickLibrary::Xbox360Button;
using JoystickLibrary::Extreme3DProService;
using JoystickLibrary::Extreme3DProButton;
using JoystickLibrary::POV;

std::map<POV, std::string> povNameMap = {
    { POV::POV_NONE, "POV_NONE" },
    { POV::POV_WEST, "POV_WEST" },
    { POV::POV_EAST, "POV_EAST" },
    { POV::POV_NORTH, "POV_NORTH" },
    { POV::POV_SOUTH, "POV_SOUTH" },
    { POV::POV_NORTHWEST, "POV_NORTHWEST" },
    { POV::POV_NORTHEAST, "POV_NORTHEAST" },
    { POV::POV_SOUTHWEST, "POV_SOUTHWEST" },
    { POV::POV_SOUTHEAST, "POV_SOUTHEAST" }
};

class Xbox360ServiceTester
{
public:
    Xbox360ServiceTester(int num_js)
    {
        this->service = new Xbox360Service(num_js);
    }

    ~Xbox360ServiceTester()
    {
        if (this->service)
            delete this->service;
    }

    bool Start()
    {
        return this->service->Initialize() && this->service->Start();
    }

    void PrintAbsoluteAxes(int id)
    {
        int lx, ly, rx, ry, ltrigger, rtrigger;

        if (!this->service->GetLeftX(id, lx))
            lx = 0;
        if (!this->service->GetLeftY(id, ly))
            ly = 0;
        if (!this->service->GetRightX(id, rx))
            rx = 0;
        if (!this->service->GetRightY(id, ry))
            ry = 0;
        if (!this->service->GetLeftTrigger(id, ltrigger))
            ltrigger = 0;
        if (!this->service->GetRightTrigger(id, rtrigger))
            rtrigger = 0;

        std::cout << "LX: " << lx << " | "
            << "LY: " << ly << " | "
            << "RX: " << rx << " | "
            << "RY: " << ry << " | "
            << "LT: " << ltrigger << " | "
            << "RT: " << rtrigger << " | "
            << std::endl;
    }

    void PrintButtons(int id)
    {
        std::map<Xbox360Button, bool> buttons;
        if (!this->service->GetButtons(id, buttons))
            return;

        for (auto pair : buttons)
            std::cout << static_cast<int>(pair.first) << ": " << pair.second << " | ";
        std::cout << std::endl;
    }

    void PrintDpad(int id)
    {
        POV pov;
        if (!this->service->GetDpad(id, pov))
            pov = POV::POV_NONE;

        std::cout << "D-PAD: " << povNameMap[pov] << std::endl;
    }

    Xbox360Service *service;    
};


class Extreme3DProServiceTester
{
public:
    Extreme3DProServiceTester(int num_js)
    {
        this->service = new Extreme3DProService(num_js);
    }

    ~Extreme3DProServiceTester()
    {
        if (this->service)
            delete this->service;
    }

    bool Start()
    {
        return this->service->Initialize() && this->service->Start();
    }

    void PrintAbsoluteAxes(int id)
    {
        int x, y, z, slider;

        if (!this->service->GetX(id, x))
            x = 0;
        if (!this->service->GetY(id, y))
            y = 0;
        if (!this->service->GetZRot(id, z))
            z = 0;
        if (!this->service->GetSlider(id, slider))
            slider = 0;

        std::cout << "X: " << x << " | Y: " << y << " | Z: " << z << " | Slider: " << slider << std::endl;
    }

    void PrintButtons(int id)
    {
        std::map<Extreme3DProButton, bool> buttons;
        this->service->GetButtons(id, buttons);

        for (auto pair : buttons)
            std::cout << static_cast<int>(pair.first) << ": " << pair.second << " | ";
        std::cout << std::endl;
    }

    void PrintPOV(int id)
    {
        POV pov;
        if (!this->service->GetPOV(id, pov))
            pov = POV::POV_NONE;

        std::cout << "POV: " << povNameMap[pov] << std::endl;
    }

    Extreme3DProService *service;
};



