#include "details/tagged_union.h"

#include <functional>

#define ONE_OF_CREATE_ALTERNATIVE(tag, type) struct tag{ typedef type Type; }; 

namespace one_of
{

    /**
     * Allows to visit a const OneOf instance
     * @tparam V the variant type of the OneOf instance
     * @tparam Flags a bitmap describing which alternatives have already been matched
     *               (0b100 means that the 3rd alternative has already been matched)
     */
    template<typename V, uint64_t Flags = 0>
    class ConstVisitor
    {
    public:

        explicit ConstVisitor(const V& val, const uint32_t& index, const bool& already_found = false) : 
            _val(val), 
            _index(index),
            _already_found(already_found) {}

        /**
         * Matches a const OneOf with the given Alternative
         * @tparam Alt the Alternative
         * @param cb the function that will be called if the OneOf matches the Alternative
         * @return A visitor for chained calls
         */
        template<typename Tag>
        ConstVisitor<V, Flags | (1 << details::index_of<Tag, V>::value)> match(std::function<void(const typename Tag::Type&)> cb)
        {
            static_assert((Flags & (1 << details::index_of<Tag, V>::value)) == 0, "Can not match the same alternative twice");

            if (details::index_of<Tag, V>::value == _index)
            {
                _already_found = true;
                cb(details::get<Tag>(_val));
            }
            return ConstVisitor<V, Flags | (1 << details::index_of<Tag, V>::value)>(_val, _index, _already_found);
        }

        /**
         * Defines a fallback function that handles unmatched Alternatives
         * @param cb in case of an unmatched Alternative, this function will be called with the Alternative's index
         */
        void fallback(std::function<void()> cb)
        {
            if (!_already_found) { cb(); }
        }

        /**
         * Asserts that all Alternatives have been matched
         */
        void assertMatchIsExhaustive() const
        {
            static_assert(Flags == (1 << details::size_of<V>::value) - 1, "Match is not exhaustive");
        }

    private:

        const V& _val;
        const uint32_t _index;
        bool _already_found;
    };

    /**
     * Allows to visit a non-const OneOf instance
     * @tparam V the variant type of the OneOf instance
     * @tparam Flags a bitmap describing which alternatives have already been matched
     *               (0b100 means that the 3rd alternative has already been matched)
     */
    template<typename V, uint64_t Flags = 0>
    class MutVisitor
    {
    public:

        explicit MutVisitor(V& val, const uint32_t& index, bool already_found = false) : 
            _val(val), 
            _index(index), 
            _already_found(already_found) {}

        /**
         * Matches a non-const OneOf with the given Alternative
         * @tparam Alt the Alternative
         * @param cb the function that will be called if the OneOf matches the Alternative
         * @return A visitor for chained calls
         */
        template<typename Tag>
        MutVisitor<V, Flags | (1 << details::index_of<Tag, V>::value)> match(std::function<void(typename Tag::Type&)> cb)
        {
            static_assert((Flags & (1 << details::index_of<Tag, V>::value)) == 0, "Can not match the same alternative twice");

            if (details::index_of<Tag, V>::value == _index)
            {
                _already_found = true;
                cb(details::get<Tag>(_val));
            }
            return MutVisitor<V, Flags | (1 << details::index_of<Tag, V>::value)>(_val, _index, _already_found);
        }

        /**
         * Defines a fallback function that handles unmatched Alternatives
         * @param cb in case of an unmatched Alternative, this function will be called with the Alternative's index
         */
        void fallback(std::function<void()> cb)
        {
            if (!_already_found) { cb(); }
        }

        /**
         * Asserts that all Alternatives have been matched
         */
        void assertMatchIsExhaustive() const
        {
            static_assert(Flags == (1 << details::size_of<V>::value) - 1, "Match is not exhaustive");
        }

    private:

        V& _val;
        const uint32_t _index;
        bool _already_found;
    };

    /**
     * The OneOf class
     * @tparam Tags : the tags of this oneof
     */
    template<typename... Tags>
    class OneOf
    {
    public:

        template<typename Tag>
        OneOf(Tag)
        {
            _index = details::index_of<Tag, Variant>::value;
            details::construct<Tag>(_value);
        }

        template<typename Tag, typename... Args>
        OneOf(Tag, Args&&... args)
        {
            _index = details::index_of<Tag, Variant>::value;
            details::construct<Tag>(_value, args...);
        }

        OneOf(const OneOf& other)
        {
            _index = other._index;
            copy<Tags...>(other._value, other._index);
        }

        void operator=(const OneOf& other)
        {
            destruct<Tags...>(_index);
            _index = other._index;
            copy<Tags...>(other._value, other._index);
        }

        ~OneOf()
        {
            destruct<Tags...>(_index);
        }

        template<typename Tag, typename... Args>
        void emplace(Args&&... args)
        {
            destruct<Tags...>(_index);
            _index = details::index_of<Tag, Variant>::value;
            details::construct<Tag>(_value, args...);
        }

        typedef details::TaggedUnion<Tags...> Variant;

        /**
         * Matches a const OneOf with the given Alternative
         * @tparam Alt the Alternative
         * @param cb the function that will be called if the OneOf matches the Alternative
         * @return A visitor for chained calls
         */
        template<typename Tag>
        ConstVisitor<Variant, (1 << details::index_of<Tag, Variant>::value)> match(std::function<void(const typename Tag::Type&)> cb) const
        {
            return ConstVisitor<Variant>(_value, _index).template match<Tag>(cb);
        }

        /**
         * Matches a non-const OneOf with the given Alternative
         * @tparam Alt the Alternative
         * @param cb the function that will be called if the OneOf matches the Alternative
         * @return A visitor for chained calls
         */
        template<typename Tag>
        MutVisitor<Variant, (1 << details::index_of<Tag, Variant>::value)> match(std::function<void(typename Tag::Type&)> cb)
        {
            return MutVisitor<Variant>(_value, _index).template match<Tag>(cb);
        }

    private:

        template<typename T>
        void destruct(const uint32_t& index) 
        {
            details::destruct<T>(_value);
        }

        template<typename T1, typename T2, typename... Ts> 
        void destruct(const uint32_t& index) 
        {
            if(index == 0) 
            {
                details::destruct<T1>(_value);
            }
            if(index == 1) 
            {
                details::destruct<T2>(_value);
            }
            else 
            {
                destruct<T2, Ts...>(index - 1);
            }
        }

        template<typename T>
        void copy(const Variant& other, const uint32_t& index) 
        { 
            details::construct<T>(_value, details::get<T>(other));
        }

        template<typename T1, typename T2, typename... Ts> 
        void copy(const Variant& other, const uint32_t& index) 
        {
            if(index == 0) 
            {
                details::construct<T1>(_value, details::get<T1>(other));
            }
            if(index == 1) 
            {
                details::construct<T2>(_value, details::get<T2>(other));
            }
            else 
            {
                copy<T2, Ts...>(other, index - 1);
            }
        }

        Variant _value;
        uint32_t _index = 0;
    };
}
