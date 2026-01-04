#include <stdint.h>
#include <utility>

namespace one_of::details
{
    
    // -----------------------------------------------
    // Tagged Union Definition
    // -----------------------------------------------

    template<typename HeadTag, typename... TailTags>
    struct TaggedUnion
    {
        union
        {
            typename HeadTag::Type head;
            TaggedUnion<TailTags...> tail;
        };

        TaggedUnion() {}
        ~TaggedUnion() {}
    };

    template<typename Tag>
    struct TaggedUnion<Tag>
    {
        union
        {
            typename Tag::Type head;
            uint8_t dummy;
        };
        
        TaggedUnion() {}
        ~TaggedUnion() {}
    };

    // -----------------------------------------------
    // Construct functions
    // -----------------------------------------------

    template<typename T, typename... Args> 
    void construct(TaggedUnion<T>& var, Args&&... args);

    template <typename U, typename T1, typename T2, typename... Ts, typename... Args> 
    void construct(TaggedUnion<T1, T2, Ts...>& var, Args&&... args);

    template<typename T1, typename T2, typename... Ts>
    class HeadConstructor
    {
    public:
        template<typename U, typename... Args>
        static void construct(TaggedUnion<T1, T2, Ts...>& var, Args&&... args)
        {
            new (&var.head) (typename U::Type)(std::forward<Args>(args)...);
        }
    };

    template<typename T1, typename T2, typename... Ts>
    class TailConstructor
    {
    public:
        template<typename U, typename... Args>
        static void construct(TaggedUnion<T1, T2, Ts...>& var, Args&&... args)
        {
            ::one_of::details::construct<U>(var.tail, args...);
        }
    };

    template<typename T, typename... Args> void construct(TaggedUnion<T>& var, Args&&... args)
    {
        using Type = typename T::Type;
        new (&var.head) Type(std::forward<Args>(args)...);
    }
        
    template <typename U, typename T1, typename T2, typename... Ts, typename... Args> void construct(TaggedUnion<T1, T2, Ts...>& var, Args&&... args)
    {
        std::conditional<   
            std::is_same<U,T1>::value, 
            HeadConstructor<T1, T2, Ts...>, 
            TailConstructor<T1, T2, Ts...>
        >::type::template construct<U>(var, args...);
    }

    // -----------------------------------------------
    // Destruct Functions
    // -----------------------------------------------

    template<typename U, typename T, typename... Ts> void destruct(TaggedUnion<T, Ts...>& var)
    {
        if (std::is_same<U, T>::value)
        {
            using Type = typename T::Type;
            var.head.~Type();
        }
        else
        {
            destruct<U>(var.tail);
        }
    }

    template<typename U, typename T> void destruct(TaggedUnion<T>& var)
    {
        using Type = typename T::Type;
        var.head.~Type();
    }

    // -----------------------------------------------
    // Getters
    // -----------------------------------------------

    template<typename T, typename... Ts>    typename T::Type& get(TaggedUnion<T, Ts...>& var)                { return var.head; }
    template<typename T, typename... Ts>    typename T::Type& get(TaggedUnion<Ts...>& var)                   { return get<T>(var.tail); }

    template<typename T, typename... Ts>    const typename T::Type& get(const TaggedUnion<T, Ts...>& var)    { return var.head; }
    template<typename T, typename... Ts>    const typename T::Type& get(const TaggedUnion<Ts...>& var)       { return get<T>(var.tail); }

    // -----------------------------------------------
    // Utils
    // -----------------------------------------------

    template<typename V> struct size_of{};
    template<typename T>                    struct size_of<TaggedUnion<T>>        { static constexpr uint32_t value = 1; };
    template<typename T, typename... Ts>    struct size_of<TaggedUnion<T, Ts...>> { static constexpr uint32_t value = 1 + size_of<TaggedUnion<Ts...>>::value; };

    template<typename T, typename V>                    struct index_of {};
    template<typename T>                                struct index_of<T, TaggedUnion<T>>            { static constexpr uint32_t value = 0; };
    template<typename T, typename... Ts>                struct index_of<T, TaggedUnion<T, Ts...>>     { static constexpr uint32_t value = 0; };
    template<typename U, typename T, typename... Ts>    struct index_of<U, TaggedUnion<T, Ts...>>     { static constexpr uint32_t value =  1 + index_of<U, TaggedUnion<Ts...>>::value; };
}