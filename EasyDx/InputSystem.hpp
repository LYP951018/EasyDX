#pragma once

#include "Misc.hpp"

namespace dx
{
    enum class VirtualKey : std::uint32_t
    {
        kLeftButton = 1,
        kRightButton = 1,
        kBack = 8,
        kTab = 9,
        kReturn = 0xD,
        kShift = 0x10,
        kControl = 0x11,
        kEsc = 0x1B,
        kSpace = 0x20,
        kLeft = 0x25,
        kUp = 0x26,
        kRight = 0x27,
        kDown = 0x28,
        k0 = 0x30,
        k1 = 0x31, k2, k3, k4, k5, k6, k7, k8, k9,
        kA = 0x41, kB, kC, kD, kE, kF, kG, kH, kI, kJ, kK, kL, kM, kN, kO, kP, kQ,
        kR, kS, kT, kU, kV, kW, kX, kY, kZ,
        kOemClear = 0xFE //the last key
    };

    enum class ElementState
    {
        Released,
        Pressed,
    };

    class InputSystem : Noncopyable
    {
    public:
        InputSystem()
            : m_keyStates{}
        {}

        bool IsPressing(VirtualKey key) const;
        Point MouseMoved() const;
        ElementState GetKeyState(VirtualKey key) const;

    private:
        //��Ӧ�ð� MessageDispatcher ���� TU ���ڲ�����Ϊ��Ԫ������Ϊ�˷��㡣
        friend struct MessageDispatcher;
        friend class Game;

        std::array<ElementState, 0xFF> m_keyStates;
        std::optional<std::pair<Point, Point>> m_mousePosPair;

        void OnKeyDown(std::uint32_t keyCode);
        void OnKeyUp(std::uint32_t keyCode);
        void OnInitialized();
        void OnFrameDone();
        void OnFrameStart();
    };
}