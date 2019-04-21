#pragma once

namespace dx
{
    class ComponentBase
    {
      public:
        virtual ~ComponentBase();
    };

#define DEF_COMPONENT_WRAPPER(type, componentName, getterName)                 \
    class componentName : public ::dx::ComponentBase                           \
    {                                                                          \
      public:                                                                  \
        template<typename =                                                    \
                     ::std::enable_if_t<::std::is_copy_constructible_v<type>>> \
        componentName(const type& data) : m_data{data}                         \
        {}                                                                     \
        componentName(type&& data) : m_data{::std::move(data)} {}              \
        componentName() = default;                                             \
                                                                               \
        type& getterName() { return m_data; }                                  \
        const type& getterName() const { return m_data; }                      \
                                                                               \
      private:                                                                 \
        type m_data;                                                           \
    };

#define DEF_COMPONENT_WRAPPER_DEFAULT(type, getterName) \
    DEF_COMPONENT_WRAPPER(type, CONCAT(type, Component), getterName)

} // namespace dx