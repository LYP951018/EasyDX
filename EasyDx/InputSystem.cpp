#include "pch.hpp"
#include "InputSystem.hpp"
#include <Windows.h>

namespace dx
{
    dx::Point GetMousePos()
    {
        ::POINT point{};
        ::GetCursorPos(&point);
        return {
            point.x, point.y
        };
    }

    bool InputSystem::IsPressing(VirtualKey key) const
    {
        return GetKeyState(key) == ElementState::Pressed;
    }

    Point InputSystem::MouseMoved() const
    {
        assert(m_mousePosPair.has_value());
        auto[orig, now] = *m_mousePosPair;
        return Point{
            now.X - orig.X,
            now.Y - orig.Y
        };
    }

    ElementState InputSystem::GetKeyState(VirtualKey key) const
    {
        return m_keyStates[static_cast<std::uint32_t>(key)];
    }

    void InputSystem::OnKeyDown(std::uint32_t keyCode)
    {
        m_keyStates[keyCode] = ElementState::Pressed;
    }

    void InputSystem::OnKeyUp(std::uint32_t keyCode)
    {
        m_keyStates[keyCode] = ElementState::Released;
    }

    void InputSystem::OnInitialized()
    {
        const auto point = GetMousePos();
        m_mousePosPair = std::pair{ point, point };
    }

    void InputSystem::OnFrameDone()
    {
        m_mousePosPair->first = m_mousePosPair->second;
    }

    void InputSystem::OnFrameStart()
    {
        const auto point = GetMousePos();
        m_mousePosPair->second = point;
    }
}
