#include "Pch.hpp"
#include "Ball.hpp"
#include "InstancingVS.hpp"
#include <d3d11.h>

void Bind(ID3D11DeviceContext& context3D, const Pipeline& pipeline)
{
    dx::Bind(context3D, dx::Ref(pipeline.Layout), dx::MakeVbBinder(pipeline.Vbs), pipeline.Ib, pipeline.VS, pipeline.PS);
}

struct alignas(16) InstancingVertex
{
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX InvTransWorld;
};

constexpr std::uint32_t N = 10;

void GenerateInstancingData(dx::AlignedVec<InstancingVertex>& instancingData, std::int32_t n)
{
    instancingData.resize(n * n * n);
    const float dist = 1.0f;
    for (std::int32_t i = 0; i < n; ++i)
    {
        for (std::int32_t j = 0; j < n; ++j)
        {
            for (std::int32_t k = 0; k < n; ++k)
            {
                auto& data = instancingData[i * n * n + j * n + k];
                data.World = DirectX::XMMatrixTranslation(static_cast<float>(i) * dist, static_cast<float>(j) * dist, static_cast<float>(k) * dist);
                data.InvTransWorld = DirectX::XMMatrixInverse({}, DirectX::XMMatrixTranspose(data.World));
            }
        }
    }
}

Pipeline MakePipeline(const dx::IndependentGraphics& independent, const dx::PredefinedResources& predefined)
{
    using namespace dx;
    Pipeline po;
    auto& device = independent.Device3D();
    po.VS = VertexShader<MatrixCb>{ device, InstancingVSByteCode };
    po.PS = predefined.GetBasicPS();
    ModelResultUnit mesh;
    dx::MakeUVSphere(0.2f, 10, 10, mesh);
    dx::AlignedVec<InstancingVertex> instancingData;
    GenerateInstancingData(instancingData, N);
    FillConstVbs(device, gsl::span{ po.Vbs }, mesh.Positions, mesh.Normals, mesh.TexCoords, instancingData);
    po.Ib = { device, mesh.Indices };
    po.Tex = predefined.GetWhite();
    po.Sampler = predefined.GetDefaultSampler();
    constexpr auto units = std::array{
        MakeVertex(VSSemantics::kPosition, 0),
        MakeVertex(VSSemantics::kNormal, 1),
        MakeVertex(VSSemantics::kTexCoord, 2),
        MATRIX_VERTEX_UNITS("WORLDMATRIX", 3),
        MATRIX_VERTEX_UNITS("INVTRANSWORLDMATRIX", 3)
    };
    po.Layout = MakeLayout(device, MakeDescArray(gsl::span(units)), po.VS.ByteCode());
    return po;
}


Ball::Ball(const dx::IndependentGraphics & independent, const dx::PredefinedResources & predefined)
    : m_pipeline{ MakePipeline(independent, predefined) },
    m_material{
        DirectX::XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f },
        DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f },
        DirectX::XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f },
        DirectX::XMFLOAT4{},
        16.0f
}
{}

void Ball::Render(ID3D11DeviceContext& context3D, const dx::Game& game)
{
    const auto& scene = game.Switcher().MainScene();
    context3D.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Bind(context3D, m_pipeline);
    const auto drawContext = dx::BasicDrawContext{
        context3D, scene.MainCamera(), gsl::make_span(scene.Lights())
    };
    SetupBasicLighting(drawContext, m_pipeline.PS, m_material);
    m_pipeline.PS.SetSampler(context3D, m_pipeline.Sampler.Get());
    m_pipeline.PS.SetResource(context3D, m_pipeline.Tex.Get());
    auto& vs = m_pipeline.VS;
    auto& camera = scene.MainCamera();
    auto viewProj = camera.GetView() * camera.GetProjection();
    vs.UpdateCb(context3D, MatrixCb{
        viewProj
        });
    context3D.DrawIndexedInstanced(m_pipeline.Ib.CountOfIndices(), N * N * N, 0, 0, 0);
}
