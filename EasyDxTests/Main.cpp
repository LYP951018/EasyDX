#include <EasyDx/EasyDx.Common/Common.hpp>
#include <EasyDx/EasyDx.Common/StaticVector.hpp>

#define CATCH_CONFIG_MAIN
#include <catch.hpp>


TEST_CASE("ws2s", "Common") 
{
    REQUIRE(dx::ws2s(L"23333") == std::string{ u8"23333" });
}

//TEST_CASE("static_vector", "basic")
//{
//    using dx::static_vector;
//    static_vector<int, 10> vec;
//
//    REQUIRE(vec.begin() == vec.end());
//    REQUIRE(vec.size() == 0);
//
//    vec.push_back(1);
//    REQUIRE(vec.end() == vec.begin() + 1);
//    REQUIRE(vec.size() == 1);
//    REQUIRE(vec[0] == 1);
//}