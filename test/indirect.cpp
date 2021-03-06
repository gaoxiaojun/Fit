#include <fit/indirect.hpp>
#include "test.hpp"

FIT_TEST_CASE()
{
    FIT_TEST_CHECK(3 == fit::indirect(std::unique_ptr<binary_class>(new binary_class()))(1, 2));
    FIT_TEST_CHECK(3 == fit::reveal(fit::indirect(std::unique_ptr<binary_class>(new binary_class())))(1, 2));

    binary_class f;

    FIT_TEST_CHECK(3 == fit::indirect(&f)(1, 2));
    FIT_TEST_CHECK(3 == fit::reveal(fit::indirect(&f))(1, 2));
}

struct mutable_function
{
    mutable_function() : value(0) {}
    void operator()(int a) { value += a; }
    int value;
};

FIT_TEST_CASE()
{
    auto mf = mutable_function{};
    fit::indirect(&mf)(15);
    fit::indirect(&mf)(2);
    FIT_TEST_CHECK(mf.value == 17);
}


FIT_TEST_CASE()
{
    auto mf = std::make_shared<mutable_function>();
    fit::indirect(mf)(15);
    fit::indirect(mf)(2);
    FIT_TEST_CHECK(mf->value == 17);
}


