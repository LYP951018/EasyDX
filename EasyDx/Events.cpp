#include "pch.hpp"
#include "Events.hpp"
#include <WinUser.h>

namespace dx
{
    bool KeyStates::Control() const noexcept { return KeyStateBits & MK_CONTROL; }

    bool KeyStates::Left() const noexcept { return KeyStateBits & MK_LBUTTON; }

    bool KeyStates::Middle() const noexcept { return KeyStateBits & MK_MBUTTON; }

    bool KeyStates::Right() const noexcept { return KeyStateBits & MK_RBUTTON; }

    bool KeyStates::Shift() const noexcept { return KeyStateBits & MK_SHIFT; }

    IEventHandle::~IEventHandle() {}
} // namespace dx
