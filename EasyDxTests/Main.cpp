#include "Pch.hpp"

#include <EasyDx/LinkWithDirectX.hpp>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

TEST_CASE("ws2s", "Common")
{
    REQUIRE(dx::ws2s(L"23333") == std::string{u8"23333"});
}