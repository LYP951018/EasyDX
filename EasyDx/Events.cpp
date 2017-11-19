#include "pch.hpp"
#include "Events.hpp"
#include <WinUser.h>

namespace dx
{
    bool MouseEventArgs::Control() const noexcept
    {
        return KeyStates & MK_CONTROL;
    }

    bool MouseEventArgs::Left() const noexcept
    {
        return KeyStates & MK_LBUTTON;
    }

    bool MouseEventArgs::Middle() const noexcept
    {
        return KeyStates & MK_MBUTTON;
    }

    bool MouseEventArgs::Right() const noexcept
    {
        return KeyStates & MK_RBUTTON;
    }

    bool MouseEventArgs::Shift() const noexcept
    {
        return KeyStates & MK_SHIFT;
    }

    IEventHandle::~IEventHandle()
    {
    }
}
