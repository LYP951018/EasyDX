#include "pch.hpp"
#include "Object.hpp"
#include "ComponentBase.hpp"

namespace dx
{
    ComponentBase* Object::GetComponent(const std::type_info& type) const
    {
        if (const auto it = std::find_if(m_components.begin(), m_components.end(),
                                         [&](const std::unique_ptr<ComponentBase>& component) {
                                             return typeid(*component) == type;
                                         });
            it != m_components.end())
        {
            return it->get();
        }
        return nullptr;
    }

    void Object::AddComponentInternal(std::unique_ptr<ComponentBase> component)
    {
        m_components.push_back(std::move(component));
    }
} // namespace dx
