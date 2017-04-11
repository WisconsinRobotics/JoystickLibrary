#pragma once

#include "Enumerator.hpp"
#include <array>

namespace JoystickLibrary
{
    class JoystickService
    {
    public:
        JoystickService();
        virtual ~JoystickService();
        bool Initialize();
        int GetNumberConnected() const;
        const std::vector<int>& GetIDs() const;

    protected:
#ifdef _WIN32
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
#endif

        virtual void OnDeviceChanged(DeviceStateChange ds) = 0;
        void ProcessDeviceChange(std::vector<JoystickDescriptor> id_list, DeviceStateChange dsc);
        bool IsValidJoystickID(int id) const;
        JoystickState GetState(int id) const;

        Enumerator& enumerator = Enumerator::GetInstance();
        std::vector<int> ids;
        bool initialized;
        
    };
}
