#include "pch.hpp"
#include "Render.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Resources/Shaders.hpp"

namespace dx
{
    MaxStreamVector<ID3D11Buffer*> g_Buffers;
    MaxStreamVector<std::uint32_t> g_Strides;
    MaxStreamVector<std::uint32_t> g_Offsets;

    void ClearVbDataStructures()
    {
        g_Buffers.clear();
        g_Strides.clear();
        g_Offsets.clear();
    }

    void CheckConsistency()
    {
        const auto size = g_Buffers.size();
        assert(size == g_Strides.size() && size == g_Offsets.size());
    }

    void DrawMesh(ID3D11DeviceContext& context3D, const Mesh& mesh, const Material& material)
    {
        context3D.IASetInputLayout(&mesh.InputLayout());
        SetupIndexBuffer(context3D, mesh.GpuIb());
        const auto& vbBindData = mesh.BindData();
        mesh.FlushAll(context3D);
        const auto buffers = ComPtrsCast(mesh.GetGpuVbsWithoutFlush());
        context3D.IASetVertexBuffers(0, gsl::narrow<std::uint32_t>(buffers.size()), buffers.data(),
                                     vbBindData.Strides, vbBindData.Offsets);
        RunPasses(context3D, material, mesh);
    }

    void RunPasses(ID3D11DeviceContext& context3D, const dx::Material& material,
                   const dx::Mesh& mesh)
    {
        for (const auto& pass : material.Passes)
        {
            SetupPass(context3D, pass);
            context3D.DrawIndexed(mesh.IndexCount(),
                                  // TODO
                                  0,
                                  // TODO
                                  0);
        }
    }

    void DrawMeshInstancing(ID3D11DeviceContext& context3D, const Mesh& mesh,
                            const Material& material, std::uint32_t instancingCount,
                            gsl::span<const GpuBuffer> instancingBuffers,
                            gsl::span<const std::uint32_t> strides)
    {
        const auto& vbBindData = mesh.BindData();
        const auto buffers = ComPtrsCast(mesh.GetGpuVbsWithoutFlush());
        const auto bufferCount = buffers.size();
        // dirty here
        ClearVbDataStructures();
        Append(g_Buffers, buffers, ComPtrsCast(instancingBuffers));

        //the non-instancing part
        g_Strides.insert(g_Strides.end(), vbBindData.Strides, vbBindData.Strides + bufferCount);
        g_Offsets.insert(g_Offsets.end(), vbBindData.Offsets, vbBindData.Offsets + bufferCount);

        //the instancing part
        g_Strides.insert(g_Strides.end(), strides.begin(), strides.end());
        for (auto& buffer : instancingBuffers)
        {
            const auto desc = GetDesc(Ref(buffer));
            g_Offsets.push_back(0);
        }
        CheckConsistency();
        context3D.IASetVertexBuffers(0, gsl::narrow<std::uint32_t>(g_Buffers.size()),
                                     g_Buffers.data(), g_Strides.data(), g_Offsets.data());
        context3D.IASetInputLayout(&mesh.InputLayout());
        SetupIndexBuffer(context3D, mesh.GpuIb());
        for (const auto& pass : material.Passes)
        {
            SetupPass(context3D, pass);
            context3D.DrawIndexedInstanced(mesh.IndexCount(), instancingCount, 0, 0, 0);
        }
    }

    void SetupBlending(ID3D11DeviceContext& context3D, const Pass& pass)
    {
        const auto& blending = pass.Blending;
        if (blending.BlendState == nullptr)
        {
            context3D.OMSetBlendState(nullptr, nullptr, UINT32_MAX);
        }
        else
        {
            context3D.OMSetBlendState(blending.BlendState.Get(), blending.BlendFactor.data(),
                                      blending.SampleMask);
        }
    }

    void SetupDepthStencilStates(ID3D11DeviceContext& context3D, const Pass& pass)
    {
        const auto& depthStencilSettings = pass.DepthStencil;
        if (depthStencilSettings.StencilState == nullptr)
        {
            context3D.OMSetDepthStencilState(nullptr, 0);
        }
        else
        {
            context3D.OMSetDepthStencilState(depthStencilSettings.StencilState.Get(),
                                             depthStencilSettings.StencilRef);
        }
    }

    void SetupRasterizerState(ID3D11DeviceContext& context3D, const Pass& pass)
    {
        context3D.RSSetState(pass.RasterizerState.Get());
    }

    void SetupPass(ID3D11DeviceContext& context3D, const Pass& pass)
    {
        SetupShaders(context3D, pass.Shaders);
        SetupBlending(context3D, pass);
        SetupDepthStencilStates(context3D, pass);
        SetupRasterizerState(context3D, pass);
    }
} // namespace dx
