#include "pch.hpp"
#include "Resources/Shaders.hpp"
#include "ShaderCbKeyDef.hpp"
#include "GlobalShaderContext.hpp"
#include "Resources/Shaders.hpp"

namespace dx
{
    void GlobalShaderContext::Apply(
        const std::unordered_map<std::string, gsl::span<std::byte>>& bytesMap)
        const
    {
        //不想再用 boost::unordered_map 找来找去，代码太长了，先用 string
        //了……
        const auto SetIfExists = [&](std::string name, const auto& value) {
            if (const auto it = bytesMap.find(name); it != bytesMap.end())
            {
                gsl::copy(gsl::as_bytes(SingleAsSpan(value)), it->second);
            }
        };
        // SetIfExists(WORLD_MATRIX, WorldMatrix);
        SetIfExists(PROJ_MATRIX, ProjMatrix);
        SetIfExists(VIEW_MATRIX, ViewMatrix);
        SetIfExists(VIEW_PROJ_MATRIX, ViewProjMatrix);
        SetIfExists(EYE_POS, EyePos);
        SetIfExists(LIGHTS, lights);
        SetIfExists(LIGHT_COUNT, lightCount);
    }

} // namespace dx
