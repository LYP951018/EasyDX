#pragma once

#include <unordered_map>
#include <functional>
#include <type_traits>

namespace stlext
{
    template<typename T>
    T& Identity(T& t) noexcept { return t; }

    template<typename Key, typename Value, typename MapFun = decltype(Identity<const Value>),
        typename RetT = decltype(std::invoke(std::declval<MapFun>(), std::declval<Value&>()))>
    RetT FindOr(const std::unordered_map<Key, Value>& table, const Key& key, RetT value = {}, 
        MapFun fun = Identity<const Value>)
    {
        const auto it = table.find(key);
        if (it != table.end())
        {
            return fun(it->second);
        }
        else
        {
            return value;
        }
    }
}