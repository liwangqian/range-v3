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

#include <string>
#include <vector>
#include <iterator>
#include <functional>
#include <range/v3/core.hpp>
#include <range/v3/algorithm/move.hpp>
#include <range/v3/functional/overload.hpp>
#include <range/v3/iterator/insert_iterators.hpp>
#include <range/v3/utility/copy.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/counted.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/span.hpp>
#include <range/v3/view/zip.hpp>
#include "../simple_test.hpp"
#include "../test_utils.hpp"

struct is_odd
{
    bool operator()(int i) const
    {
        return (i % 2) == 1;
    }
};

// https://github.com/ericniebler/range-v3/issues/996
void bug_996()
{
    std::vector<int> buff(12, -1);
    ::ranges::span<int> sp(buff.data(), 12);

    auto x = ::ranges::view::transform(sp, [](int a) { return a > 3 ? a : 42; });
    auto y = ::ranges::view::transform(x, sp, [](int a, int b) { return a + b; });
    auto rng = ::ranges::view::transform(y, [](int a) { return a + 1; });
    (void)rng;
}

int main()
{
    using namespace ranges;

    int rgi[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto && rng = rgi | view::transform(is_odd());
    has_type<int &>(*begin(rgi));
    has_type<bool>(*begin(rng));
    models<SizedViewConcept>(aux::copy(rng));
    models<RandomAccessViewConcept>(aux::copy(rng));
    ::check_equal(rng, {true, false, true, false, true, false, true, false, true, false});

    std::pair<int, int> rgp[] = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}, {7,7}, {8,8}, {9,9}, {10,10}};
    auto && rng2 = rgp | view::transform(&std::pair<int,int>::first);
    has_type<int &>(*begin(rng2));
    CPP_assert(Same<range_value_t<decltype(rng2)>, int>);
    CPP_assert(Same<decltype(iter_move(begin(rng2))), int &&>);
    models<CommonViewConcept>(aux::copy(rng2));
    models<SizedViewConcept>(aux::copy(rng2));
    models<RandomAccessViewConcept>(aux::copy(rng2));
    ::check_equal(rng2, {1,2,3,4,5,6,7,8,9,10});
    ::check_equal(rng2 | view::reverse, {10,9,8,7,6,5,4,3,2,1});
    CHECK(&*begin(rng2) == &rgp[0].first);
    CHECK(rng2.size() == 10u);

    auto && rng3 = view::counted(rgp, 10) | view::transform(&std::pair<int,int>::first);
    has_type<int &>(*begin(rng3));
    models<CommonViewConcept>(aux::copy(rng3));
    models<SizedViewConcept>(aux::copy(rng3));
    models<RandomAccessViewConcept>(aux::copy(rng3));
    ::check_equal(rng3, {1,2,3,4,5,6,7,8,9,10});
    CHECK(&*begin(rng3) == &rgp[0].first);
    CHECK(rng3.size() == 10u);

    auto && rng4 = view::counted(forward_iterator<std::pair<int, int>*>{rgp}, 10)
                      | view::transform(&std::pair<int,int>::first);
    has_type<int &>(*begin(rng4));
    models_not<CommonViewConcept>(aux::copy(rng4));
    models<SizedViewConcept>(aux::copy(rng4));
    models<ForwardViewConcept>(aux::copy(rng4));
    models_not<BidirectionalViewConcept>(aux::copy(rng4));
    ::check_equal(rng4, {1,2,3,4,5,6,7,8,9,10});
    CHECK(&*begin(rng4) == &rgp[0].first);
    CHECK(rng4.size() == 10u);

    counted_iterator<forward_iterator<std::pair<int, int>*>> i = begin(rng4).base();
    (void)i;

    // Test transform with a mutable lambda
    int cnt = 100;
    auto mutable_rng = view::transform(rgi, [cnt](int) mutable { return cnt++;});
    ::check_equal(mutable_rng, {100,101,102,103,104,105,106,107,108,109});
    CHECK(cnt == 100);
    CPP_assert(View<decltype(mutable_rng)>);
    CPP_assert(!View<decltype(mutable_rng) const>);

    // Test iter_transform by transforming a zip view to select one element.
    {
        auto v0 = to<std::vector<MoveOnlyString>>({"a","b","c"});
        auto v1 = to<std::vector<MoveOnlyString>>({"x","y","z"});

        auto rng = view::zip(v0, v1);
        ::models<RandomAccessRangeConcept>(rng);

        std::vector<MoveOnlyString> res;
        using R = decltype(rng);
        using I = iterator_t<R>;
        // Needlessly verbose -- a simple transform would do the same, but this
        // is an interesting test.
        auto proj = overload(
            [](I i) -> MoveOnlyString& {return (*i).first;},
            [](copy_tag, I) -> MoveOnlyString {return {};},
            [](move_tag, I i) -> MoveOnlyString&& {return std::move((*i).first);}
        );
        auto rng2 = rng | view::iter_transform(proj);
        move(rng2, ranges::back_inserter(res));
        ::check_equal(res, {"a","b","c"});
        ::check_equal(v0, {"","",""});
        ::check_equal(v1, {"x","y","z"});
        using R2 = decltype(rng2);
        CPP_assert(Same<range_value_t<R2>, MoveOnlyString>);
        CPP_assert(Same<range_reference_t<R2>, MoveOnlyString &>);
        CPP_assert(Same<range_rvalue_reference_t<R2>, MoveOnlyString &&>);
    }

    // two range transform
    {
        auto v0 = to<std::vector<std::string>>({"a","b","c"});
        auto v1 = to<std::vector<std::string>>({"x","y","z"});

        auto rng = view::transform(v0, v1, [](std::string& s0, std::string& s1){return std::tie(s0, s1);});
        using R = decltype(rng);
        CPP_assert(Same<range_value_t<R>, std::tuple<std::string&, std::string&>>);
        CPP_assert(Same<range_reference_t<R>, std::tuple<std::string&, std::string&>>);
        CPP_assert(Same<range_rvalue_reference_t<R>, std::tuple<std::string&, std::string&>>);

        using T = std::tuple<std::string, std::string>;
        ::check_equal(rng, {T{"a","x"}, T{"b","y"}, T{"c","z"}});
    }

    // two range indirect transform
    {
        auto v0 = to<std::vector<std::string>>({"a","b","c"});
        auto v1 = to<std::vector<std::string>>({"x","y","z"});
        using I = std::vector<std::string>::iterator;

        auto fun = overload(
            [](I i, I j)           { return std::tie(*i, *j); },
            [](copy_tag, I, I)     { return std::tuple<std::string, std::string>{}; },
            [](move_tag, I i, I j) { return common_tuple<std::string&&, std::string&&>{
                std::move(*i), std::move(*j)}; } );

        auto rng = view::iter_transform(v0, v1, fun);
        using R = decltype(rng);
        CPP_assert(Same<range_value_t<R>, std::tuple<std::string, std::string>>);
        CPP_assert(Same<range_reference_t<R>, std::tuple<std::string&, std::string&>>);
        CPP_assert(Same<range_rvalue_reference_t<R>, common_tuple<std::string&&, std::string&&>>);

        using T = std::tuple<std::string, std::string>;
        ::check_equal(rng, {T{"a","x"}, T{"b","y"}, T{"c","z"}});
    }

    {
        auto rng = debug_input_view<int const>{rgi} | view::transform(is_odd{});
        ::check_equal(rng, {true, false, true, false, true, false, true, false, true, false});
    }

    {
        auto v0 = to<std::vector<std::string>>({"a","b","c"});
        auto v1 = to<std::vector<std::string>>({"x","y","z"});

        auto r0 = debug_input_view<std::string>{v0.data(), distance(v0)};
        auto r1 = debug_input_view<std::string>{v1.data(), distance(v1)};
        auto rng = view::transform(std::move(r0), std::move(r1),
            [](std::string &s0, std::string &s1){ return std::tie(s0, s1); });
        using R = decltype(rng);
        CPP_assert(Same<range_value_t<R>, std::tuple<std::string &, std::string &>>);
        CPP_assert(Same<range_reference_t<R>, std::tuple<std::string &, std::string &>>);
        CPP_assert(Same<range_rvalue_reference_t<R>, std::tuple<std::string &, std::string &>>);

        using T = std::tuple<std::string, std::string>;
        ::check_equal(rng, {T{"a","x"}, T{"b","y"}, T{"c","z"}});
    }

    {
#if RANGES_CXX_DEDUCTION_GUIDES >= RANGES_CXX_DEDUCTION_GUIDES_17
#if defined(__clang__) && __clang_major__ < 6
        // Workaround https://bugs.llvm.org/show_bug.cgi?id=33314
        RANGES_DIAGNOSTIC_PUSH
        RANGES_DIAGNOSTIC_IGNORE_UNDEFINED_FUNC_TEMPLATE
#endif
        std::vector<int> vi = {1, 2, 3};
        ranges::transform_view times_ten{vi, [](int i) { return i * 10; }};
        ::check_equal(times_ten, {10, 20, 30});
#if defined(__clang__) && __clang_major__ < 6
        RANGES_DIAGNOSTIC_POP
#endif // clang bug workaround
#endif // use deduction guides
    }

    return test_result();
}
