#pragma once

#include <variant>
#include <functional>

#include "map_macro.h"

#define GENERATE_KEY(key, type) key,
#define GENERATE_KEY_FROM_OPTION(option) GENERATE_KEY option

#define GENERATE_CALLBACK(key, type) std::function<ReturnType(const type &)> key = nullptr;
#define GENERATE_CALLBACK_FROM_OPTION(option) GENERATE_CALLBACK option

#define GENERATE_CALLBACK_MUT(key, type) std::function<ReturnType(type &)> key = nullptr;
#define GENERATE_CALLBACK_MUT_FROM_OPTION(option) GENERATE_CALLBACK_MUT option

#define GENERATE_VARIANT(key, type) type,
#define GENERATE_VARIANT_FROM_OPTION(option) GENERATE_VARIANT option

#define GENERATE_VISITOR_TEMPLATE(key, type) bool key##_HANDLED = false,
#define GENERATE_VISITOR_TEMPLATE_FROM_OPTION(option) GENERATE_VISITOR_TEMPLATE option

#define GENERATE_AFTER_KEY_SET(key, type) SetKey<Key, Keys::key, key##_HANDLED>(),
#define GENERATE_AFTER_KEY_SET_FROM_OPTION(option) GENERATE_AFTER_KEY_SET option

#define GENERATE_AFTER_FALLBACK_SET(key, type) key##_HANDLED,
#define GENERATE_AFTER_FALLBACK_SET_FROM_OPTION(option) GENERATE_AFTER_FALLBACK_SET option

#define GENERATE_CONSTRUCTOR_ASSERT(key, type)                                              \
    if constexpr (Key == Keys::key)                                                         \
    {                                                                                       \
        static_assert(std::is_same<T, type>::value || std::is_convertible<T, type>::value); \
    }
#define GENERATE_CONSTRUCTOR_ASSERT_FROM_OPTION(option) GENERATE_CONSTRUCTOR_ASSERT option

#define GENERATE_ASSERT(key, type) key##_HANDLED ||
#define GENERATE_ASSERT_FROM_OPTION(option) GENERATE_ASSERT option

#define GENERATE_MATCH(key, type)                                                 \
    else if constexpr (Key == Keys::key)                                          \
    {                                                                             \
        static_assert(!key##_HANDLED, #key " visitor already defined for " #key); \
        _callbacks.key = cb;                                                      \
        return AfterKeySet<Keys::key>(_callbacks, _fallback);                     \
    }
#define GENERATE_MATCH_FROM_OPTION(option) GENERATE_MATCH option

#define GENERATE_VISIT(key, type)                                                     \
    if (key##_HANDLED && one_of._key == Keys::key)                                    \
    {                                                                                 \
        RETURN_MAYBE_VOID(_callbacks.key(std::get<(size_t)Keys::key>(one_of._value))) \
    }
#define GENERATE_VISIT_FROM_OPTION(option) GENERATE_VISIT option

#define RETURN_MAYBE_VOID(expr)                      \
    if constexpr (!std::is_same_v<void, ReturnType>) \
    {                                                \
        return expr;                                 \
    }                                                \
    else                                             \
    {                                                \
        expr;                                        \
        return;                                      \
    }

#define GENERATE_ONE_OF(class_name, ...)                                                                                                \
    class class_name                                                                                                                    \
    {                                                                                                                                   \
                                                                                                                                        \
    public:                                                                                                                             \
        class Empty                                                                                                                     \
        {                                                                                                                               \
        };                                                                                                                              \
                                                                                                                                        \
        enum class Keys                                                                                                                 \
        {                                                                                                                               \
            MAP(GENERATE_KEY_FROM_OPTION, __VA_ARGS__)                                                                                  \
        };                                                                                                                              \
                                                                                                                                        \
        template <Keys Key, typename T>                                                                                                 \
        static class_name create(const T &value)                                                                                        \
        {                                                                                                                               \
            MAP(GENERATE_CONSTRUCTOR_ASSERT_FROM_OPTION, __VA_ARGS__)                                                                   \
                                                                                                                                        \
            class_name res;                                                                                                             \
            res._key = Key;                                                                                                             \
            res._value.emplace<(size_t)Key>(value);                                                                                     \
                                                                                                                                        \
            return res;                                                                                                                 \
        }                                                                                                                               \
                                                                                                                                        \
    private:                                                                                                                            \
        template <typename ReturnType>                                                                                                  \
        struct Callbacks                                                                                                                \
        {                                                                                                                               \
            MAP(GENERATE_CALLBACK_FROM_OPTION, __VA_ARGS__)                                                                             \
        };                                                                                                                              \
        template <typename ReturnType>                                                                                                  \
        struct CallbacksMut                                                                                                             \
        {                                                                                                                               \
            MAP(GENERATE_CALLBACK_MUT_FROM_OPTION, __VA_ARGS__)                                                                         \
        };                                                                                                                              \
                                                                                                                                        \
        Keys _key;                                                                                                                      \
        std::variant<MAP(GENERATE_VARIANT_FROM_OPTION, __VA_ARGS__) Empty> _value;                                                      \
        class_name() {};                                                                                                                \
                                                                                                                                        \
    public:                                                                                                                             \
        template <typename ReturnType,                                                                                                  \
                  MAP(GENERATE_VISITOR_TEMPLATE_FROM_OPTION, __VA_ARGS__) bool HAS_FALLBACK = false>                                    \
        class Visitor                                                                                                                   \
        {                                                                                                                               \
        private:                                                                                                                        \
            template <Keys KeyToSet, Keys GivenKey, bool IsSet>                                                                         \
            static constexpr bool SetKey() { return GivenKey == KeyToSet ? true : IsSet; }                                              \
                                                                                                                                        \
            Callbacks<ReturnType> _callbacks;                                                                                           \
            std::function<ReturnType(const Keys &)> _fallback = nullptr;                                                                \
                                                                                                                                        \
        public:                                                                                                                         \
            Visitor(Callbacks<ReturnType> callbacks, std::function<ReturnType(const Keys &)> fallback)                                  \
            {                                                                                                                           \
                _callbacks = callbacks;                                                                                                 \
                _fallback = fallback;                                                                                                   \
            }                                                                                                                           \
                                                                                                                                        \
            template <Keys Key>                                                                                                         \
            using AfterKeySet = Visitor<ReturnType,                                                                                     \
                                        MAP(GENERATE_AFTER_KEY_SET_FROM_OPTION, __VA_ARGS__)                                            \
                                            HAS_FALLBACK>;                                                                              \
                                                                                                                                        \
            using AfterFallbackSet = Visitor<ReturnType,                                                                                \
                                             MAP(GENERATE_AFTER_FALLBACK_SET_FROM_OPTION, __VA_ARGS__) true>;                           \
                                                                                                                                        \
            Visitor()                                                                                                                   \
            {                                                                                                                           \
                static_assert(!(MAP(GENERATE_ASSERT_FROM_OPTION, __VA_ARGS__) HAS_FALLBACK),                                            \
                              "Visitor must be initialized with default template parameters");                                          \
            }                                                                                                                           \
                                                                                                                                        \
            template <Keys Key, typename Cb>                                                                                            \
            AfterKeySet<Key> match(Cb cb)                                                                                               \
            {                                                                                                                           \
                if constexpr (false)                                                                                                    \
                {                                                                                                                       \
                }                                                                                                                       \
                MAP(GENERATE_MATCH_FROM_OPTION, __VA_ARGS__)                                                                            \
                else                                                                                                                    \
                {                                                                                                                       \
                    static_assert(false, "Invalid key type for match");                                                                 \
                }                                                                                                                       \
            }                                                                                                                           \
                                                                                                                                        \
            AfterFallbackSet fallback(std::function<ReturnType(const Keys &)> cb)                                                       \
            {                                                                                                                           \
                static_assert(!HAS_FALLBACK, "Fallback visitor already defined");                                                       \
                _fallback = cb;                                                                                                         \
                return AfterFallbackSet(_callbacks, _fallback);                                                                         \
            }                                                                                                                           \
                                                                                                                                        \
            AfterFallbackSet defaultFallback()                                                                                          \
            {                                                                                                                           \
                static_assert(!HAS_FALLBACK, "Fallback visitor already defined");                                                       \
                if constexpr (std::is_same<void, ReturnType>::value)                                                                    \
                {                                                                                                                       \
                    _fallback = [](const Keys &) {};                                                                                    \
                }                                                                                                                       \
                else                                                                                                                    \
                {                                                                                                                       \
                    _fallback = [](const Keys &) { return ReturnType(); };                                                              \
                }                                                                                                                       \
                return AfterFallbackSet(_callbacks, _fallback);                                                                         \
            }                                                                                                                           \
                                                                                                                                        \
            ReturnType visit(const class_name &one_of)                                                                                  \
            {                                                                                                                           \
                static_assert(HAS_FALLBACK || (MAP(GENERATE_ASSERT_FROM_OPTION, __VA_ARGS__) true),                                     \
                              "Some visitors are missing, either implement all visitors or implement a fallback with the ELSE method"); \
                MAP(GENERATE_VISIT_FROM_OPTION, __VA_ARGS__)                                                                            \
                RETURN_MAYBE_VOID(_fallback(one_of._key));                                                                              \
            }                                                                                                                           \
        };                                                                                                                              \
                                                                                                                                        \
        template <typename ReturnType,                                                                                                  \
                  MAP(GENERATE_VISITOR_TEMPLATE_FROM_OPTION, __VA_ARGS__) bool HAS_FALLBACK = false>                                    \
        class VisitorMut                                                                                                                \
        {                                                                                                                               \
        private:                                                                                                                        \
            template <Keys KeyToSet, Keys GivenKey, bool IsSet>                                                                         \
            static constexpr bool SetKey() { return GivenKey == KeyToSet ? true : IsSet; }                                              \
                                                                                                                                        \
            CallbacksMut<ReturnType> _callbacks;                                                                                        \
            std::function<ReturnType(const Keys &)> _fallback = nullptr;                                                                \
                                                                                                                                        \
        public:                                                                                                                         \
            VisitorMut(CallbacksMut<ReturnType> callbacks, std::function<ReturnType(const Keys &)> fallback)                            \
            {                                                                                                                           \
                _callbacks = callbacks;                                                                                                 \
                _fallback = fallback;                                                                                                   \
            }                                                                                                                           \
                                                                                                                                        \
            template <Keys Key>                                                                                                         \
            using AfterKeySet = VisitorMut<ReturnType,                                                                                  \
                                           MAP(GENERATE_AFTER_KEY_SET_FROM_OPTION, __VA_ARGS__)                                         \
                                               HAS_FALLBACK>;                                                                           \
                                                                                                                                        \
            using AfterFallbackSet = VisitorMut<ReturnType,                                                                             \
                                                MAP(GENERATE_AFTER_FALLBACK_SET_FROM_OPTION, __VA_ARGS__) true>;                        \
                                                                                                                                        \
            VisitorMut()                                                                                                                \
            {                                                                                                                           \
                static_assert(!(MAP(GENERATE_ASSERT_FROM_OPTION, __VA_ARGS__) HAS_FALLBACK),                                            \
                              "Visitor must be initialized with default template parameters");                                          \
            }                                                                                                                           \
                                                                                                                                        \
            template <Keys Key, typename Cb>                                                                                            \
            AfterKeySet<Key> match(Cb cb)                                                                                               \
            {                                                                                                                           \
                if constexpr (false)                                                                                                    \
                {                                                                                                                       \
                }                                                                                                                       \
                MAP(GENERATE_MATCH_FROM_OPTION, __VA_ARGS__)                                                                            \
                else                                                                                                                    \
                {                                                                                                                       \
                    static_assert(false, "Invalid key type for match");                                                                 \
                }                                                                                                                       \
            }                                                                                                                           \
                                                                                                                                        \
            AfterFallbackSet fallback(std::function<ReturnType(const Keys &)> cb)                                                       \
            {                                                                                                                           \
                static_assert(!HAS_FALLBACK, "Fallback visitor already defined");                                                       \
                _fallback = cb;                                                                                                         \
                return AfterFallbackSet(_callbacks, _fallback);                                                                         \
            }                                                                                                                           \
                                                                                                                                        \
            AfterFallbackSet defaultFallback()                                                                                          \
            {                                                                                                                           \
                static_assert(!HAS_FALLBACK, "Fallback visitor already defined");                                                       \
                if constexpr (std::is_same<void, ReturnType>::value)                                                                    \
                {                                                                                                                       \
                    _fallback = [](const Keys &) {};                                                                                    \
                }                                                                                                                       \
                else                                                                                                                    \
                {                                                                                                                       \
                    _fallback = [](const Keys &) { return ReturnType(); };                                                              \
                }                                                                                                                       \
                return AfterFallbackSet(_callbacks, _fallback);                                                                         \
            }                                                                                                                           \
                                                                                                                                        \
            ReturnType visit(class_name &one_of)                                                                                        \
            {                                                                                                                           \
                static_assert(HAS_FALLBACK || (MAP(GENERATE_ASSERT_FROM_OPTION, __VA_ARGS__) true),                                     \
                              "Some visitors are missing, either implement all visitors or implement a fallback with the ELSE method"); \
                MAP(GENERATE_VISIT_FROM_OPTION, __VA_ARGS__)                                                                            \
                RETURN_MAYBE_VOID(_fallback(one_of._key));                                                                              \
            }                                                                                                                           \
        };                                                                                                                              \
    };
