#pragma once

namespace dx
{
    struct IComponent
    {
        static std::uint32_t GetStaticId();
        virtual std::uint32_t GetId() const = 0;
        virtual ~IComponent();
    };

    struct ComponentId
    {
        enum
        {
            kRenderable = 0,
            kCpuMesh = 1,
            kLight = 2,
            kBehavior = 3,
            kSmoothness = 4,
            kTexture = 5
        };
    };
}