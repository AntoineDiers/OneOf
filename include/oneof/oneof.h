#pragma once

#include <variant>
#include <functional>

#define ONE_OF_CREATE_ALTERNATIVE(name, type) struct name##_tag{}; typedef oneof::Alternative<name##_tag, type> name;

namespace oneof
{
    /**
     * A OneOf alternative
     * @tparam Tag The tag of the alternative (allows two alternatives with the same type to be different)
     * @tparam Type The type of the alternative
     */
    template<typename Tag, typename Type>
    class Alternative
    {
    public:
        typedef Type InternalType;
        Alternative() = default;
        explicit Alternative(const Type& v) : val(v){}
        const Type& get() const { return val; }
        Type& get() { return val; }

    private:
        Type val;
    };

    /**
     * Returns the index of type T in variant V
     * @tparam V the variant
     * @tparam T the type
     * @tparam index the current index
     * @return the index of T in V
     */
    template<typename V, typename T, std::size_t index = 0>
    inline constexpr size_t variant_index() {
        static_assert(std::variant_size_v<V> > index, "Type not found in variant");
        if constexpr (index == std::variant_size_v<V>) {
            return index;
        } else if constexpr (std::is_same_v<std::variant_alternative_t<index, V>, T>) {
            return index;
        } else {
            return variant_index<V, T, index + 1>();
        }
    }

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

        explicit ConstVisitor(const V& val, const bool& already_found = false) : _val(val), _already_found(already_found) {}

        template<typename Alt> using InternalType = typename Alt::InternalType;

        /**
         * Matches a const OneOf with the given Alternative
         * @tparam Alt the Alternative
         * @param cb the function that will be called if the OneOf matches the Alternative
         * @return A visitor for chained calls
         */
        template<typename Alt>
        ConstVisitor<V, Flags | (1 << variant_index<V, Alt>())> match(const std::function<void(const InternalType<Alt>&)>& cb)
        {
            static_assert((Flags & (1 << variant_index<V, Alt>())) == 0, "Can not match the same alternative twice");

            if (variant_index<V, Alt>() == _val.index())
            {
                _already_found = true;
                cb(std::get<Alt>(_val).get());
            }
            return ConstVisitor<V, Flags | (1 << variant_index<V, Alt>())>(_val, _already_found);
        }

        /**
         * Defines a fallback function that handles unmatched Alternatives
         * @param cb in case of an unmatched Alternative, this function will be called with the Alternative's index
         */
        void fallback(const std::function<void(const size_t&)>& cb)
        {
            if (!_already_found) { cb(_val.index()); }
        }

        /**
         * Asserts that all Alternatives have been matched
         */
        void assertMatchIsExhaustive() const
        {
            static_assert(Flags == (1 << std::variant_size_v<V>) - 1, "Match is not exhaustive");
        }

    private:

        const V& _val;
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

        explicit MutVisitor(const V& val, bool already_found = false) : _val(val), _already_found(already_found) {}

        template<typename Alt> using InternalType = typename Alt::InternalType;

        /**
         * Matches a non-const OneOf with the given Alternative
         * @tparam Alt the Alternative
         * @param cb the function that will be called if the OneOf matches the Alternative
         * @return A visitor for chained calls
         */
        template<typename Alt>
        MutVisitor<V, Flags | (1 << variant_index<V, Alt>())> match(const std::function<void(InternalType<Alt>&)>& cb)
        {
            static_assert((Flags & (1 << variant_index<V, Alt>())) == 0, "Can not match the same alternative twice");

            if (variant_index<V, Alt>() == _val.index())
            {
                _already_found = true;
                cb(std::get<Alt>(_val).get());
            }
            return MutVisitor<V, Flags | (1 << variant_index<V, Alt>())>(_val, _already_found);
        }

        /**
         * Defines a fallback function that handles unmatched Alternatives
         * @param cb in case of an unmatched Alternative, this function will be called with the Alternative's index
         */
        void fallback(const std::function<void(const size_t& index)>& cb)
        {
            if (!_already_found) { cb(_val.index()); }
        }

        /**
         * Asserts that all Alternatives have been matched
         */
        void assertMatchIsExhaustive() const
        {
            static_assert(Flags == (1 << std::variant_size_v<V>) - 1, "Match is not exhaustive");
        }

    private:

        V& _val;
        bool _already_found;
    };

    /**
     * The OneOf class
     * @tparam Alts the Alternative of this OneOf
     */
    template<typename... Alts>
    class OneOf
    {
    public:
        /**
         * Default constructs the first Alternative
         */
        OneOf() = default;

        /**
         * Constructs the OneOf with the given Alternative
         * @tparam Alt
         * @param val
         */
        template<typename Alt>
        OneOf(const Alt& val) : _value(val){}

        typedef std::variant<Alts...> Variant;
        template<typename Alt> using InternalType = typename Alt::InternalType;

        /**
         * Matches a const OneOf with the given Alternative
         * @tparam Alt the Alternative
         * @param cb the function that will be called if the OneOf matches the Alternative
         * @return A visitor for chained calls
         */
        template<typename Alt>
        ConstVisitor<Variant, (1 << variant_index<Variant, Alt>())> match(const std::function<void(const InternalType<Alt>&)>& cb) const
        {
            return ConstVisitor<Variant>(_value).template match<Alt>(cb);
        }

        /**
         * Matches a non-const OneOf with the given Alternative
         * @tparam Alt the Alternative
         * @param cb the function that will be called if the OneOf matches the Alternative
         * @return A visitor for chained calls
         */
        template<typename Alt>
        MutVisitor<Variant, (1 << variant_index<Variant, Alt>())> match(std::function<void(InternalType<Alt>&)>& cb)
        {
            return MutVisitor<Variant>(_value).template match<Alt>(cb);
        }

    private:
        Variant _value;
    };

}