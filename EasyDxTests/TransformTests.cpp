#include "Pch.hpp"
#include <catch.hpp>

using namespace dx;

TEST_CASE("Transforms' default construction", "[Transform]")
{
    Transform defaultTransform;
    CHECK(DirectX::XMMatrixIsIdentity(defaultTransform.Matrix()));
}