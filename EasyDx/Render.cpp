#include "pch.hpp"
#include "Render.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Resources/Shaders.hpp"
#include "Resources/InputLayout.hpp"
#include "Vertex.hpp"

namespace dx
{
    MaxStreamVector<ID3D11Buffer*> g_Buffers;
    MaxStreamVector<std::uint32_t> g_Strides;
    MaxStreamVector<std::uint32_t> g_Offsets;
    std::vector<D3D11_INPUT_ELEMENT_DESC> g_InputElementDescs;
    MaxStreamVector<VSSemantics> g_VSSemantics;

    void UpdateGlobalMeshData(const dx::Mesh& mesh, VSSemantics& mask);

    void ClearVbDataStructures()
    {
        g_Buffers.clear();
        g_Strides.clear();
        g_Offsets.clear();
        g_VSSemantics.clear();
        g_InputElementDescs.clear();
    }

    void CheckConsistency()
    {
        const auto size = g_Buffers.size();
        assert(size == g_Strides.size() && size == g_Offsets.size());
    }

    void DrawMesh(ID3D11DeviceContext& context3D, const Mesh& mesh,
                  const Material& material, ID3D11Device* deviceToCreateInputLayout)
    {
        DrawMesh(context3D, mesh, *material.mainPass.pass, deviceToCreateInputLayout);
    }

    void DrawMesh(ID3D11DeviceContext& context3D, const Mesh& mesh,
                  const Pass& pass, ID3D11Device* deviceToCreateInputLayout)
    {
        context3D.IASetPrimitiveTopology(mesh.GetPrimitiveTopology());
        SetupIndexBuffer(context3D, mesh.GetGpuIndexBuffer());
        VSSemantics mask = pass.Shaders.GetMask();
        UpdateGlobalMeshData(mesh, mask);
        // FIXME: .Get
        ID3D11InputLayout* existingLayout =
            InputLayoutAllocator::Query(gsl::make_span(g_InputElementDescs))
                .Get();
        if (existingLayout == nullptr)
        {
            if (deviceToCreateInputLayout == nullptr)
            {
                throw std::out_of_range{ "input layout does not exist" };
            }
            else
            {
                existingLayout = InputLayoutAllocator::Register(*deviceToCreateInputLayout,
                    gsl::make_span(g_InputElementDescs), pass.Shaders.GetVertexShader().GetByteCode()).Get();
            }
        }
        context3D.IASetInputLayout(existingLayout);
        mesh.FlushAll(context3D);
        context3D.IASetVertexBuffers(0, g_Buffers.size(), g_Buffers.data(),
                                     g_Strides.data(), g_Offsets.data());
        SetupPass(context3D, pass);
        context3D.DrawIndexed(mesh.GetIndexCount(), 0, 0);
    }

    void UpdateGlobalMeshData(const dx::Mesh& mesh, dx::VSSemantics& mask)
    {
        const gsl::span<const VSSemantics> semanticses = mesh.GetChannelMasks();
        const gsl::span<const StreamInfo> streamsInfo = mesh.GetStreamsInfo();
        const gsl::span<ID3D11Buffer* const> buffers =
            ComPtrsCast(mesh.GetGpuVbsWithoutFlush());
        const gsl::span<const D3D11_INPUT_ELEMENT_DESC> fullInputElementDesces =
            mesh.GetFullInputElementDesces();
        ClearVbDataStructures();
        std::uint32_t inputLayoutIndex = 0;
        std::uint32_t inputSlot = 0;
        for (std::ptrdiff_t i = 0; i < semanticses.size(); ++i)
        {
            const VSSemantics semantics = semanticses[i];
            const std::uint32_t semanticsCount =
                __popcnt(static_cast<unsigned int>(semantics));
            if ((semantics & mask) != VSSemantics::kNone)
            {
                g_Buffers.push_back(buffers[i]);
                g_Strides.push_back(streamsInfo[i].GetStride());
                g_Offsets.push_back(0);
                g_VSSemantics.push_back(semantics);
                for (std::uint32_t i = 0; i < semanticsCount; ++i)
                {
                    g_InputElementDescs.push_back(
                        fullInputElementDesces[inputLayoutIndex + i]);
                    g_InputElementDescs.back().InputSlot = inputSlot;
                }
                mask &= ~semantics;
                inputSlot += 1;
            }
            inputLayoutIndex += semanticsCount;
        }
        assert(mask == VSSemantics::kNone);
    }

    // FIXME: instancing input layout
    void DrawMeshInstancing(ID3D11DeviceContext& context3D, const Mesh& mesh,
                            const Pass& pass, std::uint32_t instancingCount,
                            gsl::span<const GpuBuffer> instancingBuffers,
                            gsl::span<const std::uint32_t> strides)
    {
        const auto buffers = ComPtrsCast(mesh.GetGpuVbsWithoutFlush());
        const auto bufferCount = buffers.size();
        // dirty here
        ClearVbDataStructures();
        VSSemantics mask = pass.Shaders.GetMask();
        UpdateGlobalMeshData(mesh, mask);
        // the instancing part
        Append(g_Buffers, buffers, ComPtrsCast(instancingBuffers));
        g_Strides.insert(g_Strides.end(), strides.begin(), strides.end());
        for (auto& buffer : instancingBuffers)
        {
            const auto desc = GetDesc(Ref(buffer));
            g_Offsets.push_back(0);
        }
        CheckConsistency();
        context3D.IASetVertexBuffers(
            0, gsl::narrow<std::uint32_t>(g_Buffers.size()), g_Buffers.data(),
            g_Strides.data(), g_Offsets.data());
        context3D.IASetInputLayout(
            InputLayoutAllocator::Query(gsl::make_span(g_InputElementDescs))
                .Get());
        context3D.IASetPrimitiveTopology(mesh.GetPrimitiveTopology());
        SetupIndexBuffer(context3D, mesh.GetGpuIndexBuffer());
        SetupPass(context3D, pass);
        context3D.DrawIndexedInstanced(mesh.GetIndexCount(), instancingCount, 0,
                                       0, 0);
    }

} // namespace dx
