//#pragma once
//
//#include <DirectXMath.h>
//
//struct alignas(16) MatrixCb
//{
//    DirectX::XMMATRIX ViewProj;
//};
//
//struct Pipeline
//{
//    enum : std::size_t
//    {
//        kPos, kNormal, kTexCoord, kInstancing
//    };
//
//    wrl::ComPtr<ID3D11InputLayout> Layout;
//    std::array<dx::VertexBuffer, 4> Vbs;
//    dx::IndexBuffer Ib;
//    dx::VertexShader<MatrixCb> VS;
//    dx::SimpleLightingPS PS;
//    wrl::ComPtr<ID3D11ShaderResourceView> Tex;
//    wrl::ComPtr<ID3D11SamplerState> Sampler;
//
//    dx::VertexBuffer& PosVb() { return Vbs[kPos]; }
//    dx::VertexBuffer& NormalVb() { return Vbs[kNormal]; }
//    dx::VertexBuffer& TexCoordVb() { return Vbs[kTexCoord]; }
//    dx::VertexBuffer& InstancingVb() { return Vbs[kInstancing]; }
//};
//
//struct Ball : dx::ObjectBase
//{
//public:
//    Ball(const dx::IndependentGraphics& independent, const dx::PredefinedResources& predefined);
//    //void Update(const dx::UpdateArgs&, const dx::Game&) override;
//    void Render(ID3D11DeviceContext& context3D, const dx::Game&) override;
//
//private:
//    Pipeline m_pipeline;
//    dx::Smoothness m_material;
//};