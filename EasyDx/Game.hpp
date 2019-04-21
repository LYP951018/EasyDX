#pragma once

#include "Predefined.hpp"
#include "Events.hpp"
#include "DependentGraphics.hpp"

namespace dx
{
    class GameWindow;
    class SceneBase;
    class Game;
    class InputSystem;

    using SceneCreator = std::function<std::unique_ptr<SceneBase>(Game&)>;

    class SceneSwitcher
    {
      public:
        friend void RunGame(Game&, std::unique_ptr<GameWindow>, std::uint32_t);
        friend class Game;

        SceneSwitcher(Game& game);

        void AddSceneCreator(std::uint32_t index, SceneCreator creator);

        template<typename EnumT,
                 typename = std::enable_if_t<std::is_enum_v<EnumT>>>
        void AddSceneCreator(EnumT index, SceneCreator creator)
        {
            using type = std::underlying_type_t<EnumT>;
            static_assert(sizeof(type) < sizeof(std::uint32_t),
                          "The size of enum's underlying type should be "
                          "less than sizeof(std::uint32_t).");
            AddSceneCreator(static_cast<type>(index), std::move(creator));
        }

        // TODO: enum version.
        void WantToSwitchSceneTo(std::uint32_t index);

        SceneBase& MainScene() const { return *mainScene_; }

      private:
        void ReallySwitchToScene(Game& game, std::uint32_t index);
        void CheckAndSwitch();
        void Reset();

        std::unordered_map<std::uint32_t, SceneCreator> sceneCreators_;
        std::unique_ptr<SceneBase> mainScene_;
        std::optional<std::uint32_t> nextSceneIndex_;
        Game& game_;
    };

    struct UpdateArgs
    {
        Duration Delta;
    };

    class Game
    {
        friend void RunGame(Game&, std::unique_ptr<GameWindow>, std::uint32_t);

      public:
        Game(std::unique_ptr<GlobalGraphicsContext> globalGraphics,
             std::uint32_t fps);
        ~Game();

        WindowResizeEvent WindowResize;
        DpiChangedEvent DpiChanged;

        GameWindow& MainWindow() const { return *mainWindow_; }
        GlobalGraphicsContext& GlobalGraphics() { return *m_globalGraphics; }
        SceneSwitcher& Switcher() noexcept { return sceneSwitcher_; }
        const SceneSwitcher& Switcher() const noexcept
        {
            return sceneSwitcher_;
        }
        const InputSystem& GetInputSystem() const noexcept
        {
            return *m_inputSystem;
        }

      private:
        friend struct MessageDispatcher;

        void Run();
        void SetUp(std::unique_ptr<GameWindow> mainWindow);
        void UnpackMessage(WindowEventArgsPack event);
        void PrepareGraphicsForResizing(GameWindow* window, Size newSize);
        InputSystem& GetInputSystem() noexcept { return *m_inputSystem; }

        std::unique_ptr<GlobalGraphicsContext> m_globalGraphics;
        SceneSwitcher sceneSwitcher_;
        // TODO: only one window?
        std::unique_ptr<GameWindow> mainWindow_;
        std::unique_ptr<InputSystem> m_inputSystem;
        std::uint32_t fps_;
    };

    void RunGame(Game& game, std::unique_ptr<GameWindow> mainWindow,
                 std::uint32_t mainSceneIndex);
} // namespace dx