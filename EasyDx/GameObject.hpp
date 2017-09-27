#pragma once

#include "Component.hpp"
#include "Behavior.hpp"
#include "Transformation.hpp"

namespace dx
{
    struct UpdateArgs;

    class GameObject
    {
    public:
        template<typename... ComponentsT>
        GameObject(Rc<ComponentsT>... components)
        {
            this->AddComponents(std::move(components)...);
        }

        void AddComponent(std::shared_ptr<IComponent>);

        template<typename... ComponentsT>
        void AddComponents(std::shared_ptr<ComponentsT>... components)
        {
            static_assert(std::conjunction_v<std::is_base_of<IComponent, ComponentsT>...>);
            Swallow(AddComponentFake(std::move(components))...);
        }

        template<typename T, typename... Args>
        void EmplaceComponent(Args&&... args)
        {
            this->AddComponent(MakeShared<T>(std::forward<Args>(args)...));
        }

        template<typename... BehaviorT>
        void AddBehaviors(Rc<BehaviorT>... behavior)
        {
            static_assert(std::conjunction_v<std::is_base_of<Behavior, BehaviorT>...>);
            Swallow(AddBehaviorFake(std::move(behavior))...);
        }

        auto GetComponents() const noexcept
        {
            using namespace ranges;
            return view::transform(components_, 
                [](const Rc<IComponent>& pc) noexcept -> IComponent& {
                return *pc;
            });
        }

        auto GetBehaviors() const noexcept
        {
            using namespace ranges;
            return view::transform(behaviors_,
                [](const Rc<Behavior>& behavior) noexcept -> Behavior& {
                return *behavior;
            });
        }

        Transformation Transform;

    private:
        template<typename T>
        friend Rc<T> GetComponent(const GameObject& object);

        int AddComponentFake(std::shared_ptr<IComponent> component)
        {
            AddComponent(std::move(component));
            return 0;
        }

        int AddBehaviorFake(Rc<Behavior> behavior)
        {
            behaviors_.push_back(std::move(behavior));
            std::push_heap(behaviors_.begin(), behaviors_.end(), BehaviorComparer{});
            return 0;
        }

        struct BehaviorComparer
        {
            bool operator ()(const Rc<Behavior>& lhs, const Rc<Behavior>& rhs) const noexcept
            {
                return lhs->GetExeTime() > rhs->GetExeTime();
            }
        };

        Vec<Rc<IComponent>> components_;
        Vec<Rc<Behavior>> behaviors_;
    };

    template<typename ComponentT>
    bool IsComponent(const IComponent& component) noexcept
    {
        return component.GetId() == ComponentT::GetStaticId();
    }

    template<typename T>
    Rc<T> GetComponent(const GameObject& object)
    {
        const auto& components = object.components_;
        const auto it = std::find_if(components.begin(), components.end(), 
            [](const std::shared_ptr<IComponent>& component) {
            return IsComponent<T>(*component);
        });
        return it == components.end() ? Rc<T>{nullptr} : std::static_pointer_cast<T>(*it);
    }

    template<typename T>
    auto GetComponentsOfType(const GameObject& object)
    {
        using namespace ranges;
        return view::transform(view::filter(object.GetComponents(), [](const IComponent& c) { return IsComponent<T>(c); }),
            [](IComponent& c)
        {
            return static_cast<const T&>(c);
        });
    }
}