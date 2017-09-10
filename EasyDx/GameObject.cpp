#include "pch.hpp"
#include "GameObject.hpp"
#include "Component.hpp"

namespace dx
{
    void GameObject::AddComponent(std::shared_ptr<IComponent> component)
    {
        components_.push_back(std::move(component));
    }
}