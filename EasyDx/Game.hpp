#pragma once

#include "Predefined.hpp"
#include "Events.hpp"
#include <optional>

namespace dx
{
    class GameWindow;
    class Scene;
    class Game;

    using SceneCreator = std::function<std::unique_ptr<Scene>(Game&, std::shared_ptr<void>)>;

    struct GraphicsResources
    {
    private:
        GraphicsResources();
        ~GraphicsResources();

        friend class Game;

        wrl::ComPtr<ID3D11Device> device3D_;
        wrl::ComPtr<ID3D11DeviceContext> context3D_;
        wrl::ComPtr<ID2D1Factory1> fanctory2D_;
        wrl::ComPtr<ID2D1Device> device2D_;
        wrl::ComPtr<ID2D1DeviceContext> context2D_;
        wrl::ComPtr<IDXGIDevice> dxgiDevice_;
        wrl::ComPtr<IDWriteFactory1> dwFactory_;
    public:

        ID3D11Device& Device3D() const { return Ref(device3D_); }
        ID3D11DeviceContext& Context3D() const { return Ref(context3D_); }
        ID2D1Factory1& Factory2D() const { return Ref(fanctory2D_); }
        ID2D1Device& Device2D() const { return Ref(device2D_); }
        ID2D1DeviceContext& Context2D() const { return Ref(context2D_); }
        IDXGIDevice& DxgiDevice() const { return Ref(dxgiDevice_); }
        IDWriteFactory1& DwFactory() const { return Ref(dwFactory_); }
    };

    class SceneSwitcher
    {
    public:
        friend void RunGame(Game&, std::unique_ptr<GameWindow>, std::uint32_t, std::shared_ptr<void> arg);
        friend class Game;

        SceneSwitcher(Game& game);

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

        Scene& MainScene() const { return *mainScene_; }

    private:
        void ReallySwitchToScene(Game& game, std::uint32_t index, std::shared_ptr<void> arg);
        void CheckAndSwitch();
        void Reset();

        std::unordered_map<std::uint32_t, SceneCreator> sceneCreators_;
        std::unique_ptr<Scene> mainScene_;
        std::optional<std::uint32_t> nextSceneIndex_;
        std::shared_ptr<void> nextSceneArg_;
        Game& game_;
    };

    class Game
    {
        friend void RunGame(Game&, std::unique_ptr<GameWindow>, std::uint32_t, std::shared_ptr<void> arg);

    public:
        Game(std::uint32_t fps);
        ~Game();

        KeyDownEvent KeyDown;
        KeyUpEvent KeyUp;
        MouseDownEvent MouseDown;
        MouseUpEvent MouseUp;
        MouseMoveEvent MouseMove;
        WindowResizeEvent WindowResize;
        DpiChangedEvent DpiChanged;

        GameWindow& MainWindow() const { return *mainWindow_; }
        const GraphicsResources& Resources() const noexcept { return grahicsResources_; }
        const PredefinedResources& Predefined() const noexcept { return predefined_; }
        SceneSwitcher& Switcher() noexcept { return sceneSwitcher_; }
        const SceneSwitcher& Switcher() const noexcept { return sceneSwitcher_; }

    private:
        void Run();
        void SetUp(std::unique_ptr<GameWindow> mainWindow);

        const GraphicsResources grahicsResources_;
        const PredefinedResources predefined_;
        SceneSwitcher sceneSwitcher_;

        std::unique_ptr<GameWindow> mainWindow_;

        std::uint32_t fps_;
    };

    void RunGame(Game& game, std::unique_ptr<GameWindow> mainWindow, std::uint32_t mainSceneIndex, std::shared_ptr<void> args = {});
}