#pragma once

namespace dx
{
    struct IComponent
    {
        static std::uint32_t GetStaticId();
        virtual std::uint32_t GetId() const = 0;
        virtual ~IComponent();

        bool IsEnable;
    };

    template<typename T, typename... Args>
    Rc<T> MakeComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<IComponent, T>);
        return MakeShared<T>(std::forward<Args>(args)...);
    }

    struct ComponentId
    {
        enum
        {
            kRenderable = 0,
            kCpuMesh = 1,
            kSmoothness = 2,
            kTexture = 3
        };
    };
}