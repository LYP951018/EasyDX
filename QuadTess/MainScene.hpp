#pragma once

#include <EasyDx/Scene.hpp>
#include <EasyDx/Common.hpp>
#include <EasyDx/Predefined.hpp>

struct alignas(16) CbPerObject;
struct alignas(16) CbPerFrame;

class MainScene : public dx::Scene
{
public:
    MainScene(const dx::Game& game, dx::Rc<void> args);
    void Update(const dx::UpdateArgs& args) override;
    ~MainScene() override;

private:
    void BuildQuad(ID3D11Device&);
    dx::GpuMesh mesh_;
    dx::aligned_unique_ptr<CbPerObject> cbPerObject_;
    dx::aligned_unique_ptr<CbPerFrame> cbPerFrame_;
    wrl::ComPtr<ID3D11InputLayout> layout_;
    dx::ShaderGroup shaders_;
    dx::ConstantBuffer<CbPerObject> gpucbPerObject_;
    dx::ConstantBuffer<CbPerFrame> gpuCbPerFrame_;
};