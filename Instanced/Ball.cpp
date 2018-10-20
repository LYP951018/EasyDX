#include "Pch.hpp"
//#include "Ball.hpp"
//#include "InstancingVS.hpp"
//#include <d3d11.h>
//
//using namespace DirectX;
//
//void Bind(ID3D11DeviceContext& context3D, const Pipeline& pipeline)
//{
//    dx::Bind(context3D, dx::Ref(pipeline.Layout), dx::MakeVbBinder(pipeline.Vbs), pipeline.Ib, pipeline.VS, pipeline.PS);
//}
//
//
//
//constexpr std::uint32_t N = 10;
//
//
//
//void Culling(const dx::Camera& camera,
//    gsl::span<const XMFLOAT3> position,
//    gsl::span<const InstancingVertex> transforms,
//    std::uint32_t vertexCountPerBall,
//    ID3D11DeviceContext& context3D,
//    dx::VertexBuffer& vertexBuffer)
//{
//    const auto& frustum = camera.Frustum();
//    std::vector<InstancingVertex> visibleTransforms;
//    BoundingBox aabb;
//    std::copy_if(transforms.begin(), transforms.end(), std::back_inserter(visibleTransforms), [&](const InstancingVertex& transform) {
//        GetAabb(position, transform.World, aabb);
//        return frustum.Contains(aabb);
//    });
//    vertexBuffer.UpdateWithDiscard(context3D, gsl::make_span(visibleTransforms));
//}
//
//void GetAabb(gsl::span<const XMFLOAT3> mesh, const XMMATRIX& transform, BoundingBox& aabb)
//{
//    std::vector<XMFLOAT3> positions;
//    positions.reserve(mesh.size());
//    std::transform(mesh.begin(), mesh.end(), std::back_inserter(positions), [&](const XMFLOAT3& point) {
//        const auto pointVec = XMLoadFloat3(&point);
//        return XMVector3Transform(pointVec, transform);
//    });
//    BoundingBox::CreateFromPoints(aabb, positions.size(), positions.data(), sizeof(XMFLOAT3));
//}
//
//Pipeline MakePipeline(const dx::IndependentGraphics& independent, 
//    const dx::PredefinedResources& predefined,
//    std::uint32_t ballCount)
//{
//    using namespace dx;
//    Pipeline po;
//    auto& device = independent.Device3D();
//    po.VS = VertexShader<MatrixCb>{ device, InstancingVSByteCode };
//    po.PS = predefined.GetBasicPS();
//    ModelResultUnit mesh;
//    dx::MakeUVSphere(0.2f, 10, 10, mesh);
//    dx::AlignedVec<InstancingVertex> instancingData;
//    GenerateInstancingData(instancingData, N);
//    FillConstVbs(device, gsl::span{ po.Vbs }, mesh.Positions, mesh.Normals, mesh.TexCoords, instancingData);
//    po.Ib = { device, mesh.Indices };
//    po.Tex = predefined.GetWhite();
//    po.Sampler = predefined.GetDefaultSampler();
//    constexpr auto units = std::array{
//        MakeVertex(VSSemantics::kPosition, 0),
//        MakeVertex(VSSemantics::kNormal, 1),
//        MakeVertex(VSSemantics::kTexCoord, 2),
//        MATRIX_VERTEX_UNITS("WORLDMATRIX", 3),
//        MATRIX_VERTEX_UNITS("INVTRANSWORLDMATRIX", 3)
//    };
//    po.Layout = AddLayout(device, MakeDescArray(gsl::span(units)), po.VS.ByteCode());
//    return po;
//}
//
//
//Ball::Ball(const dx::IndependentGraphics & independent, const dx::PredefinedResources & predefined)
//    : m_pipeline{ MakePipeline(independent, predefined) },
//    m_material{
//        DirectX::XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f },
//        DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f },
//        DirectX::XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f },
//        DirectX::XMFLOAT4{},
//        16.0f
//}
//{}
//
//void Ball::Render(ID3D11DeviceContext& context3D, const dx::Game& game)
//{
//    const auto& scene = game.Switcher().MainScene();
//    context3D.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//    Bind(context3D, m_pipeline);
//    const auto drawContext = dx::BasicDrawContext{
//        context3D, scene.MainCamera(), gsl::make_span(scene.Lights())
//    };
//    SetupBasicLighting(drawContext, m_pipeline.PS, m_material);
//    m_pipeline.PS.SetSampler(context3D, m_pipeline.Sampler.Get());
//    m_pipeline.PS.SetResource(context3D, m_pipeline.Tex.Get());
//    auto& vs = m_pipeline.VS;
//    auto& camera = scene.MainCamera();
//    auto viewProj = camera.GetView() * camera.GetProjection();
//    vs.UpdateCb(context3D, MatrixCb{
//        viewProj
//        });
//    context3D.DrawIndexedInstanced(m_pipeline.Ib.CountOfIndices(), N * N * N, 0, 0, 0);
//}
