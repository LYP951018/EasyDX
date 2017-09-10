#include "pch.hpp"
#include "Component.hpp"

namespace dx
{
    std::uint32_t IComponent::GetStaticId()
    {
        assert(false);
        return 0;
    }

    IComponent::~IComponent()
    {
    }
}

