#include "Pch.hpp"
#include "CommonDevices.hpp"
#include <EasyDx/FakePosVS.hpp>
#include <catch.hpp>

TEST_CASE("Input element descs' Creation", "[InputLayout]")
{
    constexpr auto desc = dx::PosDesc[0];
}

TEST_CASE("Input element descs' ElementEquality", "[InputLayout]")
{
    D3D11_INPUT_ELEMENT_DESC desc1{}, desc2{};
    desc1.SemanticName = "";
    desc2.SemanticName = "";
    CHECK(desc1 == desc2);
    desc2.SemanticName = "Whatever";
    CHECK(desc1 != desc2);
    desc1.SemanticName = "Whatever";
    CHECK(desc1 == desc2);
    desc1.InputSlot = 2;
    CHECK(desc1 != desc2);
    // TODO: more cases
}

TEST_CASE("Inputlayouts' creation", "[InputLayout]")
{
    auto [device, context] = GetDevice();
    dx::InputLayoutAllocator allocator;
    const auto inputLayout =
        allocator.Register(device, dx::PosDesc, dx::AsBytes(FakePosVSByteCode));
    CHECK(inputLayout == allocator.Query(dx::PosDesc));
}