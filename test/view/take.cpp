// Range v3 library
//
//  Copyright Eric Niebler 2014-present
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3

#include <list>
#include <vector>
#include <range/v3/core.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/delimit.hpp>
#include <range/v3/utility/copy.hpp>
#include "../simple_test.hpp"
#include "../test_utils.hpp"

int main()
{
    using namespace ranges;

    int rgi[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto rng0 = rgi | view::take(6);
    has_type<int &>(*begin(rng0));
    models<CommonViewConcept>(aux::copy(rng0));
    models<SizedViewConcept>(aux::copy(rng0));
    models<RandomAccessIteratorConcept>(begin(rng0));
    models<RangeConcept>(detail::as_const(rng0));
    check_equal(rng0, {0, 1, 2, 3, 4, 5});
    CHECK(size(rng0) == 6u);

    auto rng0b = rgi | view::take(20);
    has_type<int &>(*begin(rng0b));
    models<CommonViewConcept>(aux::copy(rng0b));
    models<SizedViewConcept>(aux::copy(rng0b));
    models<RandomAccessIteratorConcept>(begin(rng0b));
    models<RangeConcept>(detail::as_const(rng0b));
    check_equal(rng0b, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    CHECK(size(rng0b) == 11u);

    auto rng1 = rng0 | view::reverse;
    has_type<int &>(*begin(rng1));
    models<CommonViewConcept>(aux::copy(rng1));
    models<SizedViewConcept>(aux::copy(rng1));
    models<RandomAccessIteratorConcept>(begin(rng1));
    models<RangeConcept>(detail::as_const(rng1));
    check_equal(rng1, {5, 4, 3, 2, 1, 0});

    std::vector<int> v{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto rng2 = v | view::take(6) | view::reverse;
    has_type<int &>(*begin(rng2));
    models<CommonViewConcept>(aux::copy(rng2));
    models<SizedViewConcept>(aux::copy(rng2));
    models<RandomAccessIteratorConcept>(begin(rng2));
    models<RangeConcept>(detail::as_const(rng2));
    check_equal(rng2, {5, 4, 3, 2, 1, 0});

    std::list<int> l{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto rng3 = l | view::take(6);
    has_type<int &>(*begin(rng3));
    models<ViewConcept>(aux::copy(rng3));
    models_not<CommonViewConcept>(aux::copy(rng3));
    models<SizedViewConcept>(aux::copy(rng3));
    models<BidirectionalIteratorConcept>(begin(rng3));
    models_not<RandomAccessIteratorConcept>(begin(rng3));
    models<RangeConcept>(detail::as_const(rng3));
    ::check_equal(rng3, {0, 1, 2, 3, 4, 5});
    CHECK(size(rng3) == 6u);

    auto rng3b = l | view::take(20);
    has_type<int &>(*begin(rng3b));
    models<ViewConcept>(aux::copy(rng3b));
    models_not<CommonViewConcept>(aux::copy(rng3b));
    models<SizedViewConcept>(aux::copy(rng3b));
    models<BidirectionalIteratorConcept>(begin(rng3b));
    models_not<RandomAccessIteratorConcept>(begin(rng3b));
    models<RangeConcept>(detail::as_const(rng3b));
    check_equal(rng3b, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    CHECK(size(rng3b) == 11u);

    auto rng4 = view::iota(10) | view::take(10);
    models_not<CommonViewConcept>(aux::copy(rng4));
    models<SizedViewConcept>(aux::copy(rng4));
    models<RangeConcept>(detail::as_const(rng4));
    static_assert(!ranges::is_infinite<decltype(rng4)>::value, "");
    check_equal(rng4, {10, 11, 12, 13, 14, 15, 16, 17, 18, 19});
    CHECK(size(rng4) == 10u);

    auto rng5 = view::iota(10) | view::take(10) | view::reverse;
    models<CommonViewConcept>(aux::copy(rng5));
    models<SizedViewConcept>(aux::copy(rng5));
    models_not<RangeConcept>(detail::as_const(rng5));
    static_assert(!ranges::is_infinite<decltype(rng5)>::value, "");
    check_equal(rng5, {19, 18, 17, 16, 15, 14, 13, 12, 11, 10});
    CHECK(size(rng5) == 10u);

    auto c_str = view::delimit("hello world", '\0');
    models<RandomAccessRangeConcept>(c_str);
    models_not<SizedRangeConcept>(c_str);

    auto rng6 = c_str | view::take(5);
    models<RandomAccessRangeConcept>(rng6);
    models_not<CommonViewConcept>(aux::copy(rng6));
    models_not<SizedViewConcept>(aux::copy(rng6));
    models<RangeConcept>(detail::as_const(rng6));
    check_equal(rng6, {'h','e','l','l','o'});

    auto rng7 = c_str | view::take(20);
    check_equal(rng7, {'h','e','l','l','o',' ','w','o','r','l','d'});

    subrange<std::list<int>::iterator> rl{l.begin(), l.end()};
    models<BidirectionalRangeConcept>(rl);
    models<CommonViewConcept>(aux::copy(rl));
    models_not<SizedViewConcept>(aux::copy(rl));
    models<RangeConcept>(detail::as_const(rl));

    auto rng8 = rl | view::take(5);
    models<BidirectionalRangeConcept>(rng8);
    models_not<CommonViewConcept>(aux::copy(rng8));
    models_not<SizedViewConcept>(aux::copy(rng8));
    models<RangeConcept>(detail::as_const(rng8));
    check_equal(rng8, {0, 1, 2, 3, 4});

    auto rng9 = rl | view::take(20);
    models<BidirectionalRangeConcept>(rng9);
    models_not<CommonViewConcept>(aux::copy(rng9));
    models_not<SizedViewConcept>(aux::copy(rng9));
    models<RangeConcept>(detail::as_const(rng9));
    check_equal(rng9, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    {
        auto rng = debug_input_view<int const>{rgi} | view::take(6);
        models_not<RangeConcept>(detail::as_const(rng));
        check_equal(rng, {0, 1, 2, 3, 4, 5});
    }

    return test_result();
}
