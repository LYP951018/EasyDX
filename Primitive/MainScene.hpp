#pragma once

namespace data
{
    struct CbPerFrame;
}

class MainScene : public dx::Scene
{
public:
    MainScene(const dx::Game&, std::shared_ptr<void>);

protected:
    void Update(const dx::UpdateArgs& args) override;

private:
    dx::WindowResizeEventHandle resized_;
    std::shared_ptr<dx::GameObject> ball_;

    dx::Rc<dx::Cb<data::CbPerFrame>> cpuCbPerFrame_;
    wrl::ComPtr<ID3D11Buffer> gpuCbPerFrame_;

    dx::PointLight lightGlobal_;
    dx::WindowResizeEventHandle AddResize();
    
    void SetupBall(ID3D11Device&);
};