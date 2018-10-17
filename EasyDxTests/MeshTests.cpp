#include "Pch.hpp"
#include "CommonDevices.hpp"
#include <EasyDx/FakePosVS.hpp>
#include <catch.hpp>

TEST_CASE("Mesh's single stream construction", "[Mesh]")
{
    auto [device, context] = GetDevice();
    const dx::PositionType positions[] = {
        dx::MakePosition(0.0f, 1.0f, 2.0f), dx::MakePosition(0.0f, 2.0f, 3.0f),
        dx::MakePosition(0.0f, 4.0f, 5.0f), dx::MakePosition(6.0f, 7.0f, 8.0f),
        dx::MakePosition(0.0f, 1.0f, 2.0f), dx::MakePosition(2.0f, 6.0f, 9.0f),
        dx::MakePosition(0.0f, 1.0f, 2.0f), dx::MakePosition(0.0f, 1.0f, 2.0f),
    };
    dx::InputLayoutAllocator allocator;
    const auto inputLayout =
        allocator.Register(device, dx::PosDesc, dx::AsBytes(FakePosVSByteCode));
    const dx::ShortIndex indices[] = {1, 2, 3, 4, 5, 6};
    auto mesh = dx::Mesh::CreateImmutable(device, inputLayout, gsl::make_span(indices),
                                          gsl::make_span(positions));
    CHECK(gsl::make_span(positions) == mesh->Positions());
    CHECK(mesh->IndexCount() == std::size(indices));
    CHECK(mesh->VertexCount() == std::size(positions));
    auto& bindData = mesh->BindData();
    CHECK(bindData.Strides[0] == sizeof(dx::PositionType));
    CHECK(bindData.Offsets[0] == 0);
    mesh->FlushAll(context);
    const auto vbs = mesh->GetGpuVbsWithoutFlush();
    CHECK(vbs.size() == 1);
    /*{
        auto mapped = dx::Map(context, dx::Ref(vbs[0]), dx::ResourceMapType::Read);
        auto mem = mapped.Bytes();
        CHECK(mem.size() == std::size(positions));
        CHECK(mem.size_bytes() == std::size(positions) * sizeof(DirectX::XMFLOAT3A));
    }*/
}