#pragma once

#include <unordered_map>
#include <climits>
#include <d3d11.h>
#include "Predefined.hpp"

namespace dx
{
    class GameWindow;
    class Scene;
    class Game;

    using SceneCreator = std::function<std::unique_ptr<Scene>(const Game&, std::shared_ptr<void>)>;

    class Game
    {
        friend void RunGame(Game&, std::unique_ptr<GameWindow>, std::uint32_t, std::shared_ptr<void> arg);

    public:
        Game();
        ~Game();

        void AddSceneCreator(std::uint32_t index, SceneCreator creator);

        template<typename EnumT, typename = std::enable_if_t<std::is_enum_v<EnumT>>>
        void AddSceneCreator(EnumT index, SceneCreator creator)
        {
            using type = std::underlying_type_t<EnumT>;
            static_assert(sizeof(type) < sizeof(std::uint32_t),
                "The size of enum's underlying type should be less than sizeof(std::uint32_t).");
            AddSceneCreator(static_cast<type>(index), std::move(creator));
        }

        //TODO: enum version.
        void WantToSwitchSceneTo(std::uint32_t index, std::shared_ptr<void> arg);

        Scene& GetMainScene() const;
        GameWindow* GetMainWindow() const;
        const Predefined& GetPredefined() const;

        ID3D11Device& GetDevice3D() const;
        ID3D11DeviceContext& GetContext3D() const;
        IDXGIDevice& GetDxgiDevice() const;

        ID2D1Device& GetDevice2D() const;
        ID2D1Factory1& GetFactory2D() const;
        ID2D1DeviceContext& GetContext2D() const;

        IDWriteFactory1& GetDWriteFactory() const;

    private:
        friend struct VertexShader;
        friend struct PixelShader;

        static constexpr std::uint32_t InvalidSceneIndex = UINT32_MAX;

        void InitializeDevices();
        void Run();
        void SetUp(std::unique_ptr<GameWindow> mainWindow);
        void ReallySwitchToScene(std::uint32_t index, std::shared_ptr<void> arg);
        void CheckAndSwitch();

        wrl::ComPtr<ID3D11Device> device3D_;
        wrl::ComPtr<ID3D11DeviceContext> context3D_;
        wrl::ComPtr<ID2D1Factory1> d2dFactory_;
        wrl::ComPtr<ID2D1Device> device2D_;
        wrl::ComPtr<ID2D1DeviceContext> context2D_;
        wrl::ComPtr<IDXGIDevice> dxgiDevice_;
        std::unique_ptr<Predefined> predefined_;

        wrl::ComPtr<IDWriteFactory1> dwFactory_;
        std::unordered_map<std::uint32_t, SceneCreator> sceneCreators_;
        std::unique_ptr<GameWindow> mainWindow_;
        std::unique_ptr<Scene> mainScene_;
        std::uint32_t fps_;
        std::uint32_t nextSceneIndex_;
        std::shared_ptr<void> nextSceneArg_;
    };

    void RunGame(Game& game, std::unique_ptr<GameWindow> mainWindow, std::uint32_t mainSceneIndex, std::shared_ptr<void> args = {});
}