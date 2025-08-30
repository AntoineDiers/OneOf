#pragma once

#include <variant>
#include <functional>
#include <string>

#include "map_macro.h"

#define GENERATE_KEY(key, type) key,
#define GENERATE_KEY_FROM_OPTION(option)  GENERATE_KEY option

#define GENERATE_ASSERT(key, type) if constexpr(Key == Keys::key) { static_assert(std::is_same<T, type>::value || std::is_convertible<T, type>::value); }
#define GENERATE_ASSERT_FROM_OPTION(option)  GENERATE_ASSERT option

#define GENERATE_VISITOR(key, type) std::function<T(const type&)> key = nullptr;
#define GENERATE_VISITOR_FROM_OPTION(option)  GENERATE_VISITOR option

#define GENERATE_VISITOR_MUT(key, type) std::function<T(type&)> key = nullptr;
#define GENERATE_VISITOR_MUT_FROM_OPTION(option)  GENERATE_VISITOR_MUT option

#define GENERATE_VISIT(key, type) case Keys::key: if(visitor.key){RETURN_MAYBE_VOID(visitor.key(std::get<(size_t)Keys::key>(_value)))} break;
#define GENERATE_VISIT_FROM_OPTION(option)  GENERATE_VISIT option

#define GENERATE_VARIANT(key, type) type,
#define GENERATE_VARIANT_FROM_OPTION(option)  GENERATE_VARIANT option

#define RETURN_MAYBE_VOID(expr) if constexpr(!std::is_same_v<void, T>) { return expr; } else { expr; return; } 

#define GENERATE_ONE_OF(class_name, ...)                                        \
class class_name                                                                \
{                                                                               \
public:                                                                         \
    class Empty{};                                                              \
    enum class Keys                                                             \
    {                                                                           \
        MAP(GENERATE_KEY_FROM_OPTION, __VA_ARGS__)                              \
    };                                                                          \
                                                                                \
    template<Keys Key, typename T>                                              \
    static class_name create(const T& value)                                    \
    {                                                                           \
        MAP(GENERATE_ASSERT_FROM_OPTION, __VA_ARGS__)                           \
                                                                                \
        class_name res;                                                         \
        res._key = Key;                                                         \
        res._value.emplace<(size_t)Key>(value);                                 \
                                                                                \
        return res;                                                             \
    }                                                                           \
                                                                                \
    template<typename T>                                                        \
    class Visitor                                                               \
    {                                                                           \
    public:                                                                     \
        typedef std::function<T(const Keys& key)> Fallback;                     \
        Visitor(Fallback fallback) : _fallback(fallback) {};                    \
        MAP(GENERATE_VISITOR_FROM_OPTION, __VA_ARGS__)                          \
        T fallback(const Keys& key) const { RETURN_MAYBE_VOID(_fallback(key)) } \
    private:                                                                    \
        Fallback _fallback;                                                     \
    };                                                                          \
                                                                                \
    template<typename T>                                                        \
    class VisitorMut                                                            \
    {                                                                           \
    public:                                                                     \
        typedef std::function<T(const Keys& key)> Fallback;                     \
        VisitorMut(Fallback fallback) : _fallback(fallback) {};                 \
        MAP(GENERATE_VISITOR_MUT_FROM_OPTION, __VA_ARGS__)                      \
        T fallback(const Keys& key) const { RETURN_MAYBE_VOID(_fallback(key))}  \
    private:                                                                    \
        Fallback _fallback;                                                     \
    };                                                                          \
                                                                                \
    template<typename T>                                                        \
    T visit(const Visitor<T>& visitor) const                                    \
    {                                                                           \
        switch (_key)                                                           \
        {                                                                       \
        MAP(GENERATE_VISIT_FROM_OPTION, __VA_ARGS__)                            \
        }                                                                       \
        RETURN_MAYBE_VOID(visitor.fallback(_key))                               \
    }                                                                           \
                                                                                \
    template<typename T>                                                        \
    T visit_mut(const VisitorMut<T>& visitor)                                   \
    {                                                                           \
        switch (_key)                                                           \
        {                                                                       \
        MAP(GENERATE_VISIT_FROM_OPTION, __VA_ARGS__)                            \
        }                                                                       \
        RETURN_MAYBE_VOID(visitor.fallback(_key))                               \
    }                                                                           \
                                                                                \
private:                                                                        \
                                                                                \
    class_name(){};                                                             \
                                                                                \
    Keys _key;                                                                  \
    std::variant<MAP(GENERATE_VARIANT_FROM_OPTION, __VA_ARGS__) Empty> _value;  \
};