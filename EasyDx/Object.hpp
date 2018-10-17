#pragma once

#include <DirectXMath.h>
#include <typeinfo>

namespace dx
{
    struct UpdateArgs;
    class Game;
    class ComponentBase;

    inline constexpr std::uint32_t kMaxComponentCount = 8;

    template<typename T>
    struct is_unique_ptr : std::false_type
    {};

    template<typename T, typename D>
    struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type
    {};

    class Object final
    {
      public:
        template<typename... ComponentsT>
        Object(std::unique_ptr<ComponentsT>... components)
        {
            AddComponents(std::move(components)...);
        }

        template<typename... ComponentsT, typename = std::enable_if_t<std::conjunction_v<
                     std::negation<std::disjunction<is_unique_ptr<ComponentsT>,
                                                    std::is_same<std::decay_t<ComponentsT>, Object>>>...>>>
        Object(ComponentsT&&... components)
        {
            AddComponents(std::make_unique<std::decay_t<ComponentsT>>(std::forward<ComponentsT>(components))...);
        }

        template<typename T>
        T* GetComponent() const
        {
            return dynamic_cast<T*>(GetComponent(typeid(T)));
        }

        ComponentBase* GetComponent(const std::type_info& type) const;

        template<typename T>
        T& AddComponent(std::unique_ptr<T> component)
        {
            auto& ret = *component;
            AddComponentInternal(std::move(component));
            return ret;
        }

        template<typename... Args>
        void AddComponents(std::unique_ptr<Args>... components)
        {
            m_components.reserve(sizeof...(components));
            (AddComponentInternal(std::move(components)), ...);
        }

      private:
        void AddComponentInternal(std::unique_ptr<ComponentBase> component);

        boost::container::static_vector<std::unique_ptr<ComponentBase>, kMaxComponentCount>
            m_components;
    };
}