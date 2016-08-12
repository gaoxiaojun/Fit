/*=============================================================================
    Copyright (c) 2015 Paul Fultz II
    construct.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef FIT_GUARD_CONSTRUCT_H
#define FIT_GUARD_CONSTRUCT_H

/// construct
/// =========
/// 
/// Description
/// -----------
/// 
/// The `construct` function returns a function object that will construct the
/// object when the called. A template can also be givien, which it will deduce
/// the parameters to the template. The `construct_meta` can be used to
/// construct the object from a metafunction.
/// 
/// Synopsis
/// --------
/// 
///     template<class T>
///     constexpr auto construct();
/// 
///     template<template<class...> class Template>
///     constexpr auto construct();
/// 
///     template<class MetafunctionClass>
///     constexpr auto construct_meta();
/// 
///     template<template<class...> class MetafunctionTemplate>
///     constexpr auto construct_meta();
/// 
/// Semantics
/// ---------
/// 
///     assert(construct<T>()(xs...) == T(xs...));
///     assert(construct<Template>()(xs...) == Template<decltype(xs)...>(xs...));
///     assert(construct_meta<MetafunctionClass>()(xs...) == MetafunctionClass::apply<decltype(xs)...>(xs...));
///     assert(construct_meta<MetafunctionTemplate>()(xs...) == MetafunctionTemplate<decltype(xs)...>::type(xs...));
/// 
/// Requirements
/// ------------
/// 
/// MetafunctionClass must be a:
/// 
/// * [MetafunctionClass](MetafunctionClass)
/// 
/// MetafunctionTemplate<Ts...> must be a:
/// 
/// * [Metafunction](Metafunction)
/// 
/// T, Template<Ts..>, MetafunctionClass::apply<Ts...>, and
/// MetafunctionTemplate<Ts...>::type must be:
/// 
/// * MoveConstructible
/// 
/// Example
/// -------
/// 
///     #include <fit.hpp>
///     #include <cassert>
///     #include <vector>
/// 
///     int main() {
///         auto v = fit::construct<std::vector<int>>()(5, 5);
///         assert(v.size() == 5);
///     }
/// 

#include <fit/detail/forward.hpp>
#include <fit/detail/move.hpp>
#include <fit/detail/delegate.hpp>
#include <fit/detail/join.hpp>
#include <fit/detail/remove_rvalue_reference.hpp>
#include <fit/decay.hpp>

#include <initializer_list>

namespace fit { 

namespace detail {

template<class T, class=void>
struct construct_f
{
    typedef typename std::aligned_storage<sizeof(T)>::type storage;

    struct storage_holder
    {
        storage * s;
        storage_holder(storage* x) : s(x)
        {}

        T& data()
        {
            return *reinterpret_cast<T*>(s);
        }

        ~storage_holder()
        {
            this->data().~T();
        }
    };

    constexpr construct_f()
    {}
    template<class... Ts, FIT_ENABLE_IF_CONSTRUCTIBLE(T, Ts...)>
    T operator()(Ts&&... xs) const
    {
        storage buffer{};
        new(&buffer) T(FIT_FORWARD(Ts)(xs)...);
        storage_holder h(&buffer);
        return h.data();
    }

    template<class X, FIT_ENABLE_IF_CONSTRUCTIBLE(T, std::initializer_list<X>&&)>
    T operator()(std::initializer_list<X>&& x) const
    {
        storage buffer{};
        new(&buffer) T(static_cast<std::initializer_list<X>&&>(x));
        storage_holder h(&buffer);
        return h.data();
    }

    template<class X, FIT_ENABLE_IF_CONSTRUCTIBLE(T, std::initializer_list<X>&)>
    T operator()(std::initializer_list<X>& x) const
    {
        storage buffer{};
        new(&buffer) T(x);
        storage_holder h(&buffer);
        return h.data();
    }

    template<class X, FIT_ENABLE_IF_CONSTRUCTIBLE(T, const std::initializer_list<X>&)>
    T operator()(const std::initializer_list<X>& x) const
    {
        storage buffer{};
        new(&buffer) T(x);
        storage_holder h(&buffer);
        return h.data();
    }
};

template<class T>
struct construct_f<T, typename std::enable_if<FIT_IS_LITERAL(T)>::type>
{
    constexpr construct_f()
    {}
    template<class... Ts, FIT_ENABLE_IF_CONSTRUCTIBLE(T, Ts...)>
    constexpr T operator()(Ts&&... xs) const
    {
        return T(FIT_FORWARD(Ts)(xs)...);
    }

    template<class X, FIT_ENABLE_IF_CONSTRUCTIBLE(T, std::initializer_list<X>&&)>
    constexpr T operator()(std::initializer_list<X>&& x) const
    {
        return T(static_cast<std::initializer_list<X>&&>(x));
    }

    template<class X, FIT_ENABLE_IF_CONSTRUCTIBLE(T, std::initializer_list<X>&)>
    constexpr T operator()(std::initializer_list<X>& x) const
    {
        return T(x);
    }

    template<class X, FIT_ENABLE_IF_CONSTRUCTIBLE(T, const std::initializer_list<X>&)>
    constexpr T operator()(const std::initializer_list<X>& x) const
    {
        return T(x);
    }
};

template<template<class...> class Template, template<class...> class D>
struct construct_template_f
{
    constexpr construct_template_f()
    {}
    template<class... Ts, class Result=FIT_JOIN(Template, typename D<Ts>::type...), 
        FIT_ENABLE_IF_CONSTRUCTIBLE(Result, Ts...)>
    constexpr Result operator()(Ts&&... xs) const
    {
        return construct_f<Result>()(FIT_FORWARD(Ts)(xs)...);
    }
};

template<class MetafunctionClass>
struct construct_meta_f
{
    constexpr construct_meta_f()
    {}

    template<class... Ts>
    struct apply
    : MetafunctionClass::template apply<Ts...>
    {};

    template<class... Ts, 
        class Metafunction=FIT_JOIN(apply, Ts...), 
        class Result=typename Metafunction::type, 
        FIT_ENABLE_IF_CONSTRUCTIBLE(Result, Ts...)>
    constexpr Result operator()(Ts&&... xs) const
    {
        return construct_f<Result>()(FIT_FORWARD(Ts)(xs)...);
    }
};

template<template<class...> class MetafunctionTemplate>
struct construct_meta_template_f
{
    constexpr construct_meta_template_f()
    {}
    template<class... Ts, 
        class Metafunction=FIT_JOIN(MetafunctionTemplate, Ts...), 
        class Result=typename Metafunction::type, 
        FIT_ENABLE_IF_CONSTRUCTIBLE(Result, Ts...)>
    constexpr Result operator()(Ts&&... xs) const
    {
        return construct_f<Result>()(FIT_FORWARD(Ts)(xs)...);
    }
};


template<class T>
struct construct_id
{
    typedef T type;
};

}

template<class T>
constexpr detail::construct_f<T> construct()
{
    return {};
}
// These overloads are provide for consistency
template<class T>
constexpr detail::construct_f<T> construct_forward()
{
    return {};
}

template<class T>
constexpr detail::construct_f<T> construct_basic()
{
    return {};
}

template<template<class...> class Template>
constexpr detail::construct_template_f<Template, detail::decay_mf> construct()
{
    return {};
}

template<template<class...> class Template>
constexpr detail::construct_template_f<Template, detail::construct_id> construct_forward()
{
    return {};
}

template<template<class...> class Template>
constexpr detail::construct_template_f<Template, detail::remove_rvalue_reference> construct_basic()
{
    return {};
}

template<class T>
constexpr detail::construct_meta_f<T> construct_meta()
{
    return {};
}

template<template<class...> class Template>
constexpr detail::construct_meta_template_f<Template> construct_meta()
{
    return {};
}

} // namespace fit

#endif
