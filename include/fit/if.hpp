/*=============================================================================
    Copyright (c) 2015 Paul Fultz II
    if_.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef FIT_GUARD_IF_H
#define FIT_GUARD_IF_H

/// if
/// ==
/// 
/// Description
/// -----------
/// 
/// The `if_` function decorator makes the function callable if the boolean
/// condition is true. The `if_c` version can be used to give a boolean
/// condition directly(instead of relying on an integral constant).
/// 
/// Synopsis
/// --------
/// 
///     template<class IntegralConstant>
///     constexpr auto if_(IntegralConstant);
/// 
///     template<bool B, class F>
///     constexpr auto if_c(F);
/// 
/// Requirements
/// ------------
/// 
/// IntegralConstant must be:
/// 
/// * IntegralConstant
/// 
/// F must be:
/// 
/// * [ConstCallable](ConstCallable)
/// * MoveConstructible
/// 
/// Example
/// -------
/// 
///     #include <fit.hpp>
///     #include <cassert>
/// 
///     struct sum_f
///     {
///         template<class T>
///         int operator()(T x, T y) const
///         {
///             return fit::conditional(
///                 fit::if_(std::is_integral<T>())(fit::_ + fit::_),
///                 fit::always(0)
///             )(x, y);
///         }
///     };
/// 
///     int main() {
///         assert(sum_f()(1, 2) == 3);
///         assert(sum_f()("", "") == 0);
///     }
/// 
/// References
/// ----------
/// 
/// * [static_if](static_if)
/// 

#include <fit/always.hpp>
#include <fit/detail/callable_base.hpp>
#include <fit/detail/forward.hpp>
#include <fit/detail/delegate.hpp>
#include <fit/detail/move.hpp>
#include <fit/detail/static_const_var.hpp>

namespace fit {

namespace detail {

template<class C, class...>
struct if_depend
: C
{};

template<bool Cond, class F>
struct if_adaptor : detail::callable_base<F>
{
    FIT_INHERIT_CONSTRUCTOR(if_adaptor, detail::callable_base<F>)
};

template<class F>
struct if_adaptor<false, F>
{
    template<class... Ts>
    constexpr if_adaptor(Ts&&...)
    {}
};

template<bool Cond>
struct make_if_f
{
    constexpr make_if_f()
    {}
    template<class F>
    constexpr if_adaptor<Cond, F> operator()(F f) const
    {
        return if_adaptor<Cond, F>(static_cast<F&&>(f));
    }
};

struct if_f
{
    constexpr if_f()
    {}
    template<class Cond, bool B=Cond::type::value>
    constexpr make_if_f<B> operator()(Cond) const
    {
        return {};
    }
};

}
#if FIT_HAS_VARIABLE_TEMPLATES
template<bool B>
FIT_STATIC_CONSTEXPR detail::make_if_f<B> if_c = {};
#else
template<bool B, class F>
constexpr detail::if_adaptor<B, F> if_c(F f)
{
    return detail::if_adaptor<B, F>(static_cast<F&&>(f));
}
#endif

FIT_DECLARE_STATIC_VAR(if_, detail::if_f);

} // namespace fit

#endif
