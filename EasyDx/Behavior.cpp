#include "pch.hpp"
#include "Behavior.hpp"

namespace dx
{
    void dx::FuncBehavior::Update(GameObject& object, const UpdateArgs& args)
    {
        func_(object, args);
    }
}