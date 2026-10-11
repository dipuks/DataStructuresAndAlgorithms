#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dsa/binary_search.hpp"
#include "dsa/intervals.hpp"
#include "dsa/sliding_window.hpp"
#include "dsa/two_pointers.hpp"

namespace {

std::size_t checks = 0;

void check(bool condition, std::string_view message)
{
    ++checks;
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

template <typename Exception, typename Function>
void checkThrows(Function&& function, std::string_view message)
{
    ++checks;
    try {
        function();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(std::string(message));
}

using Ints = std::vector<int>;
using Size = std::size_t;
constexpr int kMax = std::numeric_limits<int>::max();
constexpr int kMin = std::numeric_limits<int>::min();

void testTwoPointers()
{
    using dsa::two_pointers::TwoPointers;

    check(TwoPointers::validPalindrome("A man, a plan, a canal: Panama"), "palindrome phrase");
    check(!TwoPointers::validPalindrome("race a car"), "not a palindrome");
    check(TwoPointers::validPalindrome(""), "empty palindrome");
    check(TwoPointers::validPalindrome(" .,;"), "all-ignored palindrome");
    check(!TwoPointers::validPalindrome("0P"), "digits and letters differ");
    check(TwoPointers::validPalindrome("a"), "single char palindrome");

    using Triples = std::vector<std::array<int, 3>>;
    const Ints nums{-1, 0, 1, 2, -1, -4};
    check(TwoPointers::threeSum(nums) == (Triples{{-1, -1, 2}, {-1, 0, 1}}), "threeSum unique");
    check(TwoPointers::threeSum(Ints{0, 0, 0, 0}) == (Triples{{0, 0, 0}}), "threeSum zeros once");
    check(TwoPointers::threeSum(Ints{1, 2}).empty(), "threeSum too short");
    check(TwoPointers::threeSum(Ints{}).empty(), "threeSum empty");
    check(TwoPointers::threeSum(Ints{kMax, kMax, kMax}).empty(), "threeSum overflow positive");
    check(TwoPointers::threeSum(Ints{kMin, kMin, kMin}).empty(), "threeSum overflow negative");
    // kMin + kMax + 1 == 0
    check(TwoPointers::threeSum(Ints{kMax, 1, kMin}) == (Triples{{kMin, 1, kMax}}),
          "threeSum extreme triplet found");

    using Quads = std::vector<std::array<int, 4>>;
    const Ints four{1, 0, -1, 0, -2, 2};
    check(TwoPointers::fourSum(four, 0) ==
              (Quads{{-2, -1, 1, 2}, {-2, 0, 0, 2}, {-1, 0, 0, 1}}),
          "fourSum classic");
    check(TwoPointers::fourSum(Ints{2, 2, 2, 2, 2}, 8) == (Quads{{2, 2, 2, 2}}),
          "fourSum duplicates once");
    check(TwoPointers::fourSum(Ints{1, 2, 3}, 6).empty(), "fourSum too short");
    check(TwoPointers::fourSum(Ints{kMax, kMax, kMax, kMax}, 4LL * kMax) ==
              (Quads{{kMax, kMax, kMax, kMax}}),
          "fourSum overflow-wide target");
    check(TwoPointers::fourSum(Ints{kMin, kMin, kMin, kMin}, 4LL * kMin) ==
              (Quads{{kMin, kMin, kMin, kMin}}),
          "fourSum negative wide target");
    check(TwoPointers::fourSum(Ints{kMax, kMax, kMax, kMax}, 0).empty(), "fourSum no match");

    check(TwoPointers::maxArea(Ints{1, 8, 6, 2, 5, 4, 8, 3, 7}) == 49, "maxArea classic");
    check(TwoPointers::maxArea(Ints{1, 1}) == 1, "maxArea pair");
    check(TwoPointers::maxArea(Ints{5}) == 0, "maxArea single");
    check(TwoPointers::maxArea(Ints{}) == 0, "maxArea empty");
    check(TwoPointers::maxArea(Ints{0, 0, 0}) == 0, "maxArea zero heights");
    check(TwoPointers::maxArea(Ints{kMax, 0, 0, 0, kMax}) == 4LL * kMax, "maxArea wide product");
    checkThrows<std::invalid_argument>([] { (void)TwoPointers::maxArea(Ints{1, -1}); },
                                       "maxArea negative");

    check(TwoPointers::trapRainWater(Ints{0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1}) == 6, "trap classic");
    check(TwoPointers::trapRainWater(Ints{4, 2, 0, 3, 2, 5}) == 9, "trap bowl");
    check(TwoPointers::trapRainWater(Ints{1, 2, 3}) == 0, "trap ascending");
    check(TwoPointers::trapRainWater(Ints{2, 0}) == 0, "trap short");
    check(TwoPointers::trapRainWater(Ints{kMax, 0, kMax}) == kMax, "trap large");
    check(TwoPointers::trapRainWater(Ints{kMax, 0, kMax, 0, kMax}) == 2LL * kMax, "trap sum widens");
    checkThrows<std::invalid_argument>([] { (void)TwoPointers::trapRainWater(Ints{1, -1, 1}); },
                                       "trap negative");

    Ints colors{2, 0, 2, 1, 1, 0};
    TwoPointers::sortColors(colors);
    check(colors == (Ints{0, 0, 1, 1, 2, 2}), "sortColors sorts");
    Ints sorted{0, 1, 2};
    TwoPointers::sortColors(sorted);
    check(sorted == (Ints{0, 1, 2}), "sortColors sorted input");
    Ints empty;
    TwoPointers::sortColors(empty);
    check(empty.empty(), "sortColors empty");
    Ints reversed{2, 2, 1, 0, 0};
    TwoPointers::sortColors(reversed);
    check(reversed == (Ints{0, 0, 1, 2, 2}), "sortColors reversed");
    Ints invalid{2, 1, 3, 0};
    checkThrows<std::invalid_argument>([&] { TwoPointers::sortColors(invalid); },
                                       "sortColors invalid throws");
    check(invalid == (Ints{2, 1, 3, 0}), "sortColors invalid leaves input untouched");

    Ints dup{0, 0, 1, 1, 1, 2, 2, 3, 3, 4};
    const Size k = TwoPointers::removeDuplicates(dup);
    check(k == 5 && std::equal(dup.begin(), dup.begin() + 5, Ints{0, 1, 2, 3, 4}.begin()),
          "removeDuplicates logical length");
    Ints allSame{7, 7, 7};
    check(TwoPointers::removeDuplicates(allSame) == 1 && allSame[0] == 7, "removeDuplicates all same");
    Ints none;
    check(TwoPointers::removeDuplicates(none) == 0, "removeDuplicates empty");
    Ints unsorted{1, 3, 2, 2};
    checkThrows<std::invalid_argument>([&] { (void)TwoPointers::removeDuplicates(unsorted); },
                                       "removeDuplicates unsorted throws");
    check(unsorted == (Ints{1, 3, 2, 2}), "removeDuplicates unsorted untouched");

    check(TwoPointers::sortedSquares(Ints{-4, -1, 0, 3, 10}) ==
              (std::vector<long long>{0, 1, 9, 16, 100}),
          "sortedSquares mixed");
    check(TwoPointers::sortedSquares(Ints{-7, -3, 2, 3, 11}) ==
              (std::vector<long long>{4, 9, 9, 49, 121}),
          "sortedSquares ties");
    check(TwoPointers::sortedSquares(Ints{}).empty(), "sortedSquares empty");
    check(TwoPointers::sortedSquares(Ints{kMin, 0}) ==
              (std::vector<long long>{0, static_cast<long long>(kMin) * kMin}),
          "sortedSquares int min widens");
    checkThrows<std::invalid_argument>([] { (void)TwoPointers::sortedSquares(Ints{2, 1}); },
                                       "sortedSquares unsorted");
}

void testSlidingWindow()
{
    using dsa::sliding_window::SlidingWindow;

    check(SlidingWindow::maxSumSubarrayOfSizeK(Ints{2, 1, 5, 1, 3, 2}, 3) == 9, "fixed window sum");
    check(SlidingWindow::maxSumSubarrayOfSizeK(Ints{-3, -1, -2}, 1) == -1, "fixed window negatives");
    check(SlidingWindow::maxSumSubarrayOfSizeK(Ints{1, 2}, 2) == 3, "fixed window whole array");
    check(!SlidingWindow::maxSumSubarrayOfSizeK(Ints{1, 2}, 3), "fixed window too large");
    check(SlidingWindow::maxSumSubarrayOfSizeK(Ints{kMax, kMax}, 2) == 2LL * kMax, "fixed window widens");
    checkThrows<std::invalid_argument>(
        [] { (void)SlidingWindow::maxSumSubarrayOfSizeK(Ints{1}, 0); }, "fixed window k=0");

    check(SlidingWindow::longestSubstringWithoutRepeating("abcabcbb") == 3, "no repeat abc");
    check(SlidingWindow::longestSubstringWithoutRepeating("bbbbb") == 1, "no repeat b");
    check(SlidingWindow::longestSubstringWithoutRepeating("pwwkew") == 3, "no repeat pwwkew");
    check(SlidingWindow::longestSubstringWithoutRepeating("abba") == 2, "no repeat abba stale index");
    check(SlidingWindow::longestSubstringWithoutRepeating("") == 0, "no repeat empty");
    check(SlidingWindow::longestSubstringWithoutRepeating("a b!A") == 5, "no repeat bytes");

    check(SlidingWindow::longestRepeatingCharacterReplacement("ABAB", 2) == 4, "replacement ABAB");
    check(SlidingWindow::longestRepeatingCharacterReplacement("AABABBA", 1) == 4, "replacement AABABBA");
    check(SlidingWindow::longestRepeatingCharacterReplacement("ABC", 0) == 1, "replacement k=0");
    check(SlidingWindow::longestRepeatingCharacterReplacement("", 5) == 0, "replacement empty");
    check(SlidingWindow::longestRepeatingCharacterReplacement("ABC", 10) == 3, "replacement big k");

    check(SlidingWindow::minimumWindowSubstring("ADOBECODEBANC", "ABC") == std::string("BANC"),
          "min window classic");
    check(SlidingWindow::minimumWindowSubstring("a", "a") == std::string("a"), "min window single");
    check(!SlidingWindow::minimumWindowSubstring("a", "aa"), "min window multiplicity");
    check(!SlidingWindow::minimumWindowSubstring("abc", "d"), "min window absent");
    check(SlidingWindow::minimumWindowSubstring("abc", "") == std::string(), "min window empty pattern");
    check(SlidingWindow::minimumWindowSubstring("aabdec", "abc") == std::string("abdec"),
          "min window duplicates in text");
    check(SlidingWindow::minimumWindowSubstring("bba", "ab") == std::string("ba"), "min window ba");
    check(!SlidingWindow::minimumWindowSubstring("", "a"), "min window empty text");

    check(SlidingWindow::permutationInString("ab", "eidbaooo"), "permutation present");
    check(!SlidingWindow::permutationInString("ab", "eidboaoo"), "permutation absent");
    check(!SlidingWindow::permutationInString("abc", "ab"), "permutation longer pattern");
    check(SlidingWindow::permutationInString("", "x"), "permutation empty pattern");
    check(SlidingWindow::permutationInString("a", "a"), "permutation single");
    check(!SlidingWindow::permutationInString("aa", "ab"), "permutation multiplicity");

    check(SlidingWindow::findAllAnagrams("cbaebabacd", "abc") == (std::vector<Size>{0, 6}),
          "anagrams classic");
    check(SlidingWindow::findAllAnagrams("abab", "ab") == (std::vector<Size>{0, 1, 2}),
          "anagrams overlap");
    check(SlidingWindow::findAllAnagrams("a", "ab").empty(), "anagrams pattern longer");
    check(SlidingWindow::findAllAnagrams("ab", "") == (std::vector<Size>{0, 1, 2}),
          "anagrams empty pattern");
    check(SlidingWindow::findAllAnagrams("", "a").empty(), "anagrams empty text");

    check(SlidingWindow::minSubArrayLen(7, Ints{2, 3, 1, 2, 4, 3}) == 2, "min subarray len");
    check(SlidingWindow::minSubArrayLen(4, Ints{1, 4, 4}) == 1, "min subarray len single");
    check(SlidingWindow::minSubArrayLen(11, Ints{1, 1, 1}) == 0, "min subarray len none");
    check(SlidingWindow::minSubArrayLen(3, Ints{0, 0, 3, 0}) == 1, "min subarray len zeros");
    check(SlidingWindow::minSubArrayLen(5, Ints{}) == 0, "min subarray len empty");
    check(SlidingWindow::minSubArrayLen(2LL * kMax, Ints{kMax, kMax}) == 2, "min subarray len wide");
    checkThrows<std::invalid_argument>([] { (void)SlidingWindow::minSubArrayLen(0, Ints{1}); },
                                       "min subarray len target 0");
    checkThrows<std::invalid_argument>([] { (void)SlidingWindow::minSubArrayLen(1, Ints{-1}); },
                                       "min subarray len negative");

    check(SlidingWindow::longestOnes(Ints{1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0}, 2) == 6, "ones classic");
    check(SlidingWindow::longestOnes(Ints{0, 0, 0}, 0) == 0, "ones none");
    check(SlidingWindow::longestOnes(Ints{0, 0, 0}, 5) == 3, "ones big k");
    check(SlidingWindow::longestOnes(Ints{}, 1) == 0, "ones empty");
    checkThrows<std::invalid_argument>([] { (void)SlidingWindow::longestOnes(Ints{1, 2}, 1); },
                                       "ones invalid bit");

    check(SlidingWindow::longestSubstringAtMostKDistinct("eceba", 2) == 3, "k distinct eceba");
    check(SlidingWindow::longestSubstringAtMostKDistinct("aa", 1) == 2, "k distinct aa");
    check(SlidingWindow::longestSubstringAtMostKDistinct("abc", 0) == 0, "k distinct zero");
    check(SlidingWindow::longestSubstringAtMostKDistinct("abc", 9) == 3, "k distinct large");
    check(SlidingWindow::longestSubstringAtMostKDistinct("", 2) == 0, "k distinct empty");

    check(SlidingWindow::slidingWindowMaximum(Ints{1, 3, -1, -3, 5, 3, 6, 7}, 3) ==
              (Ints{3, 3, 5, 5, 6, 7}),
          "window max classic");
    check(SlidingWindow::slidingWindowMaximum(Ints{1}, 1) == Ints{1}, "window max single");
    check(SlidingWindow::slidingWindowMaximum(Ints{4, 3, 2, 1}, 2) == (Ints{4, 3, 2}),
          "window max decreasing");
    check(SlidingWindow::slidingWindowMaximum(Ints{2, 2, 2}, 2) == (Ints{2, 2}),
          "window max equal values");
    check(SlidingWindow::slidingWindowMaximum(Ints{1, 2, 3}, 3) == Ints{3}, "window max whole");
    check(SlidingWindow::slidingWindowMaximum(Ints{}, 3).empty(), "window max empty");
    checkThrows<std::invalid_argument>(
        [] { (void)SlidingWindow::slidingWindowMaximum(Ints{1, 2}, 3); }, "window max k too big");
    checkThrows<std::invalid_argument>(
        [] { (void)SlidingWindow::slidingWindowMaximum(Ints{1, 2}, 0); }, "window max k zero");

    check(SlidingWindow::numSubarrayProductLessThanK(Ints{10, 5, 2, 6}, 100) == 8, "product classic");
    check(SlidingWindow::numSubarrayProductLessThanK(Ints{1, 2, 3}, 0) == 0, "product k=0");
    check(SlidingWindow::numSubarrayProductLessThanK(Ints{1, 1, 1}, 1) == 0, "product k=1");
    check(SlidingWindow::numSubarrayProductLessThanK(Ints{1, 1, 1}, 2) == 6, "product all ones");
    check(SlidingWindow::numSubarrayProductLessThanK(Ints{}, 5) == 0, "product empty");
    check(SlidingWindow::numSubarrayProductLessThanK(Ints{5, 1, 5}, 5) == 1, "product skips big");
    check(SlidingWindow::numSubarrayProductLessThanK(
              Ints{kMax, kMax, kMax}, std::numeric_limits<long long>::max()) == 5,
          "product no overflow near LLONG_MAX");
    checkThrows<std::invalid_argument>(
        [] { (void)SlidingWindow::numSubarrayProductLessThanK(Ints{0}, 5); }, "product non-positive");
}


void testSlidingWindowExactCases()
{
    using dsa::sliding_window::SlidingWindow;

    check(SlidingWindow::subarraysWithKDistinct(Ints{1, 2, 1, 2, 3}, 2) == 7, "exact k=2 classic");
    check(SlidingWindow::subarraysWithKDistinct(Ints{1, 2, 1, 3, 4}, 3) == 3, "exact k=3 classic");
    check(SlidingWindow::subarraysWithKDistinct(Ints{1, 2, 3}, 1) == 3, "exact k=1 singles");
    check(SlidingWindow::subarraysWithKDistinct(Ints{5, 5, 5}, 1) == 6, "exact k=1 all equal");
    check(SlidingWindow::subarraysWithKDistinct(Ints{5, 5, 5}, 2) == 0, "exact k above distinct");
    check(SlidingWindow::subarraysWithKDistinct(Ints{1, 2, 3}, 3) == 1, "exact k whole array");
    check(SlidingWindow::subarraysWithKDistinct(Ints{1, 2, 3}, 0) == 0, "exact k=0");
    check(SlidingWindow::subarraysWithKDistinct(Ints{}, 1) == 0, "exact empty");
    check(SlidingWindow::subarraysWithKDistinct(Ints{kMin, kMax, kMin}, 2) == 3, "exact extremes");
    {
        std::mt19937 rng(7);
        for (int trial = 0; trial < 300; ++trial) {
            Ints a(static_cast<Size>(rng() % 9));
            for (int& v : a) v = static_cast<int>(rng() % 4);
            for (Size k = 0; k <= 5; ++k) {
                long long brute = 0;
                for (Size i = 0; i < a.size(); ++i) {
                    std::vector<int> seen;
                    for (Size j = i; j < a.size(); ++j) {
                        if (std::ranges::find(seen, a[j]) == seen.end()) seen.push_back(a[j]);
                        brute += (seen.size() == k);
                    }
                }
                check(SlidingWindow::subarraysWithKDistinct(a, k) == brute, "exact k matches brute force");
            }
        }
    }

    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{1, 1, 0, 1}) == 3, "delete one classic");
    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{0, 1, 1, 1, 0, 1, 1, 0, 1}) == 5,
          "delete one merges runs");
    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{1, 1, 1}) == 2, "delete one all ones");
    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{0, 0, 0}) == 0, "delete one all zeros");
    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{1}) == 0, "delete one single one");
    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{0}) == 0, "delete one single zero");
    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{}) == 0, "delete one empty");
    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{1, 0}) == 1, "delete one trailing zero");
    check(SlidingWindow::longestSubarrayAfterDeletingOne(Ints{1, 0, 0, 1, 1}) == 2, "delete one gap of zeros");
    checkThrows<std::invalid_argument>(
        [] { (void)SlidingWindow::longestSubarrayAfterDeletingOne(Ints{1, 2}); }, "delete one invalid bit");
}
void testBinarySearch()
{
    using dsa::binary_search::BinarySearch;
    using Range = std::optional<std::pair<Size, Size>>;

    const Ints sorted{-5, -1, 0, 3, 8, 12};
    check(BinarySearch::binarySearch(sorted, 8) == Size{4}, "bsearch found");
    check(BinarySearch::binarySearch(sorted, -5) == Size{0}, "bsearch first");
    check(BinarySearch::binarySearch(sorted, 12) == Size{5}, "bsearch last");
    check(!BinarySearch::binarySearch(sorted, 4), "bsearch missing");
    check(!BinarySearch::binarySearch(Ints{}, 1), "bsearch empty");
    check(BinarySearch::binarySearch(Ints{1, 2, 2, 2, 3}, 2) == Size{1}, "bsearch leftmost duplicate");
    check(BinarySearch::binarySearch(Ints{kMin, kMax}, kMax) == Size{1}, "bsearch extremes");

    const Ints insert{1, 3, 5, 6};
    check(BinarySearch::searchInsertPosition(insert, 5) == 2, "insert existing");
    check(BinarySearch::searchInsertPosition(insert, 2) == 1, "insert middle");
    check(BinarySearch::searchInsertPosition(insert, 7) == 4, "insert end");
    check(BinarySearch::searchInsertPosition(insert, 0) == 0, "insert start");
    check(BinarySearch::searchInsertPosition(Ints{}, 1) == 0, "insert empty");
    check(BinarySearch::searchInsertPosition(Ints{1, 2, 2, 3}, 2) == 1, "insert duplicate first");

    const Ints range{5, 7, 7, 8, 8, 10};
    check(BinarySearch::searchRange(range, 8) == Range{{3, 4}}, "range duplicates");
    check(BinarySearch::searchRange(range, 5) == Range{{0, 0}}, "range single first");
    check(BinarySearch::searchRange(range, 10) == Range{{5, 5}}, "range single last");
    check(!BinarySearch::searchRange(range, 6), "range missing");
    check(!BinarySearch::searchRange(Ints{}, 6), "range empty");
    check(BinarySearch::searchRange(Ints{2, 2, 2}, 2) == Range{{0, 2}}, "range all equal");

    const Ints rotated{4, 5, 6, 7, 0, 1, 2};
    check(BinarySearch::searchRotated(rotated, 0) == Size{4}, "rotated find 0");
    check(BinarySearch::searchRotated(rotated, 4) == Size{0}, "rotated find first");
    check(BinarySearch::searchRotated(rotated, 2) == Size{6}, "rotated find last");
    check(!BinarySearch::searchRotated(rotated, 3), "rotated missing");
    check(BinarySearch::searchRotated(Ints{1, 2, 3}, 3) == Size{2}, "rotated by zero");
    check(BinarySearch::searchRotated(Ints{1}, 1) == Size{0}, "rotated single");
    check(!BinarySearch::searchRotated(Ints{}, 1), "rotated empty");
    check(BinarySearch::searchRotated(Ints{3, 1}, 1) == Size{1}, "rotated pair");

    const Ints dupRotated{2, 5, 6, 0, 0, 1, 2};
    check(BinarySearch::searchRotatedWithDuplicates(dupRotated, 0).has_value(), "dup rotated found");
    check(!BinarySearch::searchRotatedWithDuplicates(dupRotated, 3), "dup rotated missing");
    check(BinarySearch::searchRotatedWithDuplicates(Ints{1, 1, 1, 3, 1}, 3) == Size{3},
          "dup rotated ambiguous ends");
    check(BinarySearch::searchRotatedWithDuplicates(Ints{1, 0, 1, 1, 1}, 0) == Size{1},
          "dup rotated pivot hidden");
    check(!BinarySearch::searchRotatedWithDuplicates(Ints{1, 1, 1, 1}, 2), "dup rotated all equal missing");
    check(!BinarySearch::searchRotatedWithDuplicates(Ints{}, 2), "dup rotated empty");
    {
        const auto hit = BinarySearch::searchRotatedWithDuplicates(dupRotated, 2);
        check(hit && dupRotated[*hit] == 2, "dup rotated returns a matching index");
    }

    check(BinarySearch::findMinRotated(rotated) == 0, "min rotated");
    check(BinarySearch::findMinRotated(Ints{11, 13, 15, 17}) == 11, "min unrotated");
    check(BinarySearch::findMinRotated(Ints{2, 1}) == 1, "min pair");
    check(BinarySearch::findMinRotated(Ints{7}) == 7, "min single");
    check(!BinarySearch::findMinRotated(Ints{}), "min empty");
    check(BinarySearch::findMinRotatedWithDuplicates(Ints{2, 2, 2, 0, 1}) == 0, "min dup basic");
    check(BinarySearch::findMinRotatedWithDuplicates(Ints{1, 3, 3}) == 1, "min dup unrotated");
    check(BinarySearch::findMinRotatedWithDuplicates(Ints{3, 3, 1, 3}) == 1, "min dup hidden pivot");
    check(BinarySearch::findMinRotatedWithDuplicates(Ints{1, 1, 1, 0, 1}) == 0, "min dup ambiguous");
    check(BinarySearch::findMinRotatedWithDuplicates(Ints{5, 5, 5}) == 5, "min dup all equal");
    check(!BinarySearch::findMinRotatedWithDuplicates(Ints{}), "min dup empty");

    {
        const auto peak = BinarySearch::findPeakElement(Ints{1, 2, 1, 3, 5, 6, 4});
        check(peak && (*peak == 1 || *peak == 5), "peak found");
    }
    check(BinarySearch::findPeakElement(Ints{1}) == Size{0}, "peak single");
    check(BinarySearch::findPeakElement(Ints{1, 2, 3}) == Size{2}, "peak ascending end");
    check(BinarySearch::findPeakElement(Ints{3, 2, 1}) == Size{0}, "peak descending start");
    check(!BinarySearch::findPeakElement(Ints{}), "peak empty");

    check(BinarySearch::integerSquareRoot(0) == 0, "sqrt 0");
    check(BinarySearch::integerSquareRoot(1) == 1, "sqrt 1");
    check(BinarySearch::integerSquareRoot(8) == 2, "sqrt 8");
    check(BinarySearch::integerSquareRoot(16) == 4, "sqrt 16");
    check(BinarySearch::integerSquareRoot(2147395599) == 46339, "sqrt near int max");
    check(BinarySearch::integerSquareRoot(std::numeric_limits<long long>::max()) == 3037000499LL,
          "sqrt long long max");
    checkThrows<std::invalid_argument>([] { (void)BinarySearch::integerSquareRoot(-1); },
                                       "sqrt negative");

    check(BinarySearch::minEatingSpeed(Ints{3, 6, 7, 11}, 8) == 4, "koko classic");
    check(BinarySearch::minEatingSpeed(Ints{30, 11, 23, 4, 20}, 5) == 30, "koko hours==piles");
    check(BinarySearch::minEatingSpeed(Ints{30, 11, 23, 4, 20}, 6) == 23, "koko hours 6");
    check(!BinarySearch::minEatingSpeed(Ints{3, 6, 7, 11}, 3), "koko infeasible");
    check(BinarySearch::minEatingSpeed(Ints{}, 0) == 1, "koko empty");
    check(BinarySearch::minEatingSpeed(Ints{0, 0}, 0) == 1, "koko zero piles");
    check(!BinarySearch::minEatingSpeed(Ints{1}, 0), "koko zero hours");
    check(BinarySearch::minEatingSpeed(Ints{kMax}, 1) == kMax, "koko huge pile");
    check(BinarySearch::minEatingSpeed(Ints{kMax, kMax}, 4) == (kMax + 1LL) / 2, "koko huge piles");
    checkThrows<std::invalid_argument>([] { (void)BinarySearch::minEatingSpeed(Ints{-1}, 1); },
                                       "koko negative pile");

    check(BinarySearch::shipWithinDays(Ints{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 5) == 15, "ship classic");
    check(BinarySearch::shipWithinDays(Ints{3, 2, 2, 4, 1, 4}, 3) == 6, "ship second");
    check(BinarySearch::shipWithinDays(Ints{1, 2, 3, 1, 1}, 4) == 3, "ship third");
    check(BinarySearch::shipWithinDays(Ints{5, 5}, 1) == 10, "ship one day");
    check(BinarySearch::shipWithinDays(Ints{5, 5}, 100) == 5, "ship many days");
    check(BinarySearch::shipWithinDays(Ints{}, 3) == 0, "ship empty");
    check(!BinarySearch::shipWithinDays(Ints{1}, 0), "ship zero days infeasible");
    check(BinarySearch::shipWithinDays(Ints{kMax, kMax}, 1) == 2LL * kMax, "ship wide sum");
    checkThrows<std::invalid_argument>([] { (void)BinarySearch::shipWithinDays(Ints{-1}, 1); },
                                       "ship negative");

    check(BinarySearch::splitArrayLargestSum(Ints{7, 2, 5, 10, 8}, 2) == 18, "split classic");
    check(BinarySearch::splitArrayLargestSum(Ints{1, 2, 3, 4, 5}, 2) == 9, "split second");
    check(BinarySearch::splitArrayLargestSum(Ints{1, 4, 4}, 3) == 4, "split into singles");
    check(BinarySearch::splitArrayLargestSum(Ints{1, 4, 4}, 1) == 9, "split one part");
    check(BinarySearch::splitArrayLargestSum(Ints{0, 0, 0}, 2) == 0, "split zeros");
    check(!BinarySearch::splitArrayLargestSum(Ints{1, 2}, 3), "split too many parts");
    check(!BinarySearch::splitArrayLargestSum(Ints{1, 2}, 0), "split zero parts");
    check(!BinarySearch::splitArrayLargestSum(Ints{}, 1), "split empty");
    check(BinarySearch::splitArrayLargestSum(Ints{kMax, kMax}, 1) == 2LL * kMax, "split wide sum");
    checkThrows<std::invalid_argument>(
        [] { (void)BinarySearch::splitArrayLargestSum(Ints{1, -2}, 1); }, "split negative");

    check(BinarySearch::minDaysToMakeBouquets(Ints{1, 10, 3, 10, 2}, 3, 1) == 3, "bouquets m=3 k=1");
    check(!BinarySearch::minDaysToMakeBouquets(Ints{1, 10, 3, 10, 2}, 3, 2), "bouquets not enough flowers");
    check(BinarySearch::minDaysToMakeBouquets(Ints{7, 7, 7, 7, 12, 7, 7}, 2, 3) == 12, "bouquets adjacency");
    check(BinarySearch::minDaysToMakeBouquets(Ints{1, 10, 2, 9, 3, 8, 4, 7, 5, 6}, 4, 2) == 9,
          "bouquets interleaved");
    check(BinarySearch::minDaysToMakeBouquets(Ints{5}, 1, 1) == 5, "bouquets single flower");
    check(BinarySearch::minDaysToMakeBouquets(Ints{4, 4, 4}, 1, 3) == 4, "bouquets all flowers one");
    check(!BinarySearch::minDaysToMakeBouquets(Ints{}, 1, 1), "bouquets empty");
    check(BinarySearch::minDaysToMakeBouquets(Ints{-3, -1, -2}, 1, 3) == -1, "bouquets negative days");
    check(BinarySearch::minDaysToMakeBouquets(Ints{kMin, kMax}, 2, 1) == kMax, "bouquets extreme days");
    check(!BinarySearch::minDaysToMakeBouquets(Ints{1, 2}, std::numeric_limits<long long>::max(), 2),
          "bouquets m*k overflow guarded");
    checkThrows<std::invalid_argument>(
        [] { (void)BinarySearch::minDaysToMakeBouquets(Ints{1}, 0, 1); }, "bouquets m zero");
    checkThrows<std::invalid_argument>(
        [] { (void)BinarySearch::minDaysToMakeBouquets(Ints{1}, 1, 0); }, "bouquets k zero");

    check(BinarySearch::maxMinMagneticForce(Ints{1, 2, 3, 4, 7}, 3) == 3, "magnetic classic");
    check(BinarySearch::maxMinMagneticForce(Ints{5, 4, 3, 2, 1, 1000000000}, 2) == 999999999,
          "magnetic two balls unsorted");
    check(BinarySearch::maxMinMagneticForce(Ints{1, 2, 3}, 3) == 1, "magnetic every basket");
    check(BinarySearch::maxMinMagneticForce(Ints{3, 3, 3}, 2) == 0, "magnetic equal positions");
    check(BinarySearch::maxMinMagneticForce(Ints{1, 10}, 2) == 9, "magnetic two baskets");
    check(!BinarySearch::maxMinMagneticForce(Ints{1, 2}, 3), "magnetic too few baskets");
    check(!BinarySearch::maxMinMagneticForce(Ints{}, 2), "magnetic empty");
    check(BinarySearch::maxMinMagneticForce(Ints{kMin, 0, kMax}, 2) == 2LL * kMax + 1,
          "magnetic extreme span widens");
    checkThrows<std::invalid_argument>(
        [] { (void)BinarySearch::maxMinMagneticForce(Ints{1, 2}, 1); }, "magnetic one ball");
    {
        std::mt19937 rng(99);
        for (int trial = 0; trial < 300; ++trial) {
            Ints a(static_cast<Size>(2 + rng() % 6));
            for (int& v : a) v = static_cast<int>(rng() % 20);
            for (Size balls = 2; balls <= a.size(); ++balls) {
                long long brute = -1;
                for (unsigned mask = 0; mask < (1u << a.size()); ++mask) {
                    if (static_cast<Size>(std::popcount(mask)) != balls) continue;
                    Ints chosen;
                    for (Size i = 0; i < a.size(); ++i) if (mask >> i & 1u) chosen.push_back(a[i]);
                    std::ranges::sort(chosen);
                    long long minGap = std::numeric_limits<long long>::max();
                    for (Size i = 1; i < chosen.size(); ++i) minGap = std::min<long long>(minGap, chosen[i] - chosen[i - 1]);
                    brute = std::max(brute, minGap);
                }
                check(BinarySearch::maxMinMagneticForce(a, balls) == brute, "magnetic matches brute force");
            }
        }
    }
    const auto median = [](const Ints& a, const Ints& b) {
        return BinarySearch::findMedianSortedArrays(a, b);
    };
    check(median({1, 3}, {2}) == 2.0, "median odd");
    check(median({1, 2}, {3, 4}) == 2.5, "median even");
    check(median({}, {1}) == 1.0, "median one empty");
    check(median({2}, {}) == 2.0, "median other empty");
    check(median({}, {1, 2, 3, 4}) == 2.5, "median empty even");
    check(!median({}, {}), "median both empty");
    check(median({1, 1}, {1, 1}) == 1.0, "median equal");
    check(median({1, 2, 3}, {10, 11, 12}) == 6.5, "median disjoint");
    check(median({kMax}, {kMax}) == static_cast<double>(kMax), "median int max no overflow");
    check(median({kMin}, {kMax}) == -0.5, "median extremes");
    {
        std::mt19937 rng(12345);
        std::uniform_int_distribution<int> value(-20, 20);
        std::uniform_int_distribution<int> length(0, 8);
        for (int trial = 0; trial < 2000; ++trial) {
            Ints a(static_cast<Size>(length(rng)));
            Ints b(static_cast<Size>(length(rng)));
            for (int& v : a) v = value(rng);
            for (int& v : b) v = value(rng);
            std::ranges::sort(a);
            std::ranges::sort(b);
            Ints merged;
            std::ranges::merge(a, b, std::back_inserter(merged));
            const auto got = median(a, b);
            if (merged.empty()) {
                check(!got, "median random empty");
            } else {
                const Size n = merged.size();
                const double expected = n % 2 ? merged[n / 2]
                                              : (merged[n / 2 - 1] + merged[n / 2]) / 2.0;
                check(got && *got == expected, "median random matches merge");
            }
        }
    }

    using Pos = std::optional<std::pair<Size, Size>>;
    const std::vector<Ints> matrix{{1, 3, 5, 7}, {10, 11, 16, 20}, {23, 30, 34, 60}};
    check(BinarySearch::searchMatrix(matrix, 3) == Pos{{0, 1}}, "matrix found");
    check(BinarySearch::searchMatrix(matrix, 60) == Pos{{2, 3}}, "matrix last");
    check(BinarySearch::searchMatrix(matrix, 1) == Pos{{0, 0}}, "matrix first");
    check(BinarySearch::searchMatrix(matrix, 10) == Pos{{1, 0}}, "matrix row start");
    check(!BinarySearch::searchMatrix(matrix, 13), "matrix missing");
    check(!BinarySearch::searchMatrix(matrix, 100), "matrix above");
    const std::vector<Ints> tall{{1}, {3}, {5}, {7}};
    check(BinarySearch::searchMatrix(tall, 5) == Pos{{2, 0}}, "matrix tall single column");
    const std::vector<Ints> wide{{1, 2, 3, 4, 5}};
    check(BinarySearch::searchMatrix(wide, 4) == Pos{{0, 3}}, "matrix wide single row");
    check(!BinarySearch::searchMatrix(std::vector<Ints>{}, 1), "matrix no rows");
    check(!BinarySearch::searchMatrix(std::vector<Ints>{{}, {}}, 1), "matrix no columns");
    checkThrows<std::invalid_argument>(
        [] { (void)BinarySearch::searchMatrix(std::vector<Ints>{{1, 2}, {3}}, 1); }, "matrix jagged");
}

void testIntervals()
{
    using dsa::intervals::Interval;
    using dsa::intervals::Intervals;
    using Intervs = std::vector<Interval>;

    check(Intervals::mergeIntervals(Intervs{{1, 3}, {8, 10}, {2, 6}, {15, 18}}) ==
              (Intervs{{1, 6}, {8, 10}, {15, 18}}),
          "merge unsorted");
    check(Intervals::mergeIntervals(Intervs{{1, 4}, {4, 5}}) == (Intervs{{1, 5}}),
          "merge touching closed");
    check(Intervals::mergeIntervals(Intervs{{1, 10}, {2, 3}, {4, 5}}) == (Intervs{{1, 10}}),
          "merge nested");
    check(Intervals::mergeIntervals(Intervs{{1, 2}, {3, 4}}) == (Intervs{{1, 2}, {3, 4}}),
          "merge gap stays");
    check(Intervals::mergeIntervals(Intervs{{5, 5}, {5, 5}}) == (Intervs{{5, 5}}), "merge points");
    check(Intervals::mergeIntervals(Intervs{}).empty(), "merge empty");
    check(Intervals::mergeIntervals(Intervs{{kMin, kMax}, {0, 1}}) == (Intervs{{kMin, kMax}}),
          "merge extremes");
    const Intervs untouched{{3, 4}, {1, 2}};
    (void)Intervals::mergeIntervals(untouched);
    check(untouched == (Intervs{{3, 4}, {1, 2}}), "merge does not mutate input");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::mergeIntervals(Intervs{{2, 1}}); }, "merge malformed");

    check(Intervals::insertInterval(Intervs{{1, 3}, {6, 9}}, {2, 5}) == (Intervs{{1, 5}, {6, 9}}),
          "insert merge one");
    check(Intervals::insertInterval(Intervs{{1, 2}, {3, 5}, {6, 7}, {8, 10}, {12, 16}}, {4, 8}) ==
              (Intervs{{1, 2}, {3, 10}, {12, 16}}),
          "insert merge many");
    check(Intervals::insertInterval(Intervs{}, {5, 7}) == (Intervs{{5, 7}}), "insert into empty");
    check(Intervals::insertInterval(Intervs{{3, 4}}, {1, 2}) == (Intervs{{1, 2}, {3, 4}}),
          "insert before");
    check(Intervals::insertInterval(Intervs{{1, 2}}, {3, 4}) == (Intervs{{1, 2}, {3, 4}}),
          "insert after");
    check(Intervals::insertInterval(Intervs{{1, 2}}, {2, 4}) == (Intervs{{1, 4}}),
          "insert touching start merges");
    check(Intervals::insertInterval(Intervs{{3, 4}}, {1, 3}) == (Intervs{{1, 4}}),
          "insert touching end merges");
    check(Intervals::insertInterval(Intervs{{3, 4}, {6, 7}}, {0, 10}) == (Intervs{{0, 10}}),
          "insert covers all");
    check(Intervals::insertInterval(Intervs{{1, 5}}, {2, 3}) == (Intervs{{1, 5}}),
          "insert inside existing");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::insertInterval(Intervs{{1, 3}, {2, 4}}, {5, 6}); },
        "insert overlapping input");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::insertInterval(Intervs{{3, 4}, {1, 2}}, {5, 6}); },
        "insert unsorted input");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::insertInterval(Intervs{}, {2, 1}); }, "insert malformed new");

    check(Intervals::eraseOverlapIntervals(Intervs{{1, 2}, {2, 3}, {3, 4}, {1, 3}}) == 1,
          "erase classic");
    check(Intervals::eraseOverlapIntervals(Intervs{{1, 2}, {1, 2}, {1, 2}}) == 2, "erase identical");
    check(Intervals::eraseOverlapIntervals(Intervs{{1, 2}, {2, 3}}) == 0, "erase touching ok");
    check(Intervals::eraseOverlapIntervals(Intervs{}) == 0, "erase empty");
    check(Intervals::eraseOverlapIntervals(Intervs{{1, 100}, {11, 22}, {1, 11}, {2, 12}}) == 2,
          "erase long interval");
    check(Intervals::eraseOverlapIntervals(Intervs{{-5, -3}, {kMin, kMax}}) == 1, "erase extremes");
    check(Intervals::eraseOverlapIntervals(Intervs{{0, 2}, {1, 3}, {2, 4}, {3, 5}, {4, 6}}) == 2,
          "erase chain");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::eraseOverlapIntervals(Intervs{{1, 1}}); }, "erase empty interval");

    check(Intervals::canAttendMeetings(Intervs{{7, 10}, {2, 4}}), "attend disjoint");
    check(!Intervals::canAttendMeetings(Intervs{{0, 30}, {5, 10}, {15, 20}}), "attend conflict");
    check(Intervals::canAttendMeetings(Intervs{{1, 2}, {2, 3}}), "attend back-to-back");
    check(Intervals::canAttendMeetings(Intervs{}), "attend none");
    check(Intervals::canAttendMeetings(Intervs{{1, 5}}), "attend one");
    check(!Intervals::canAttendMeetings(Intervs{{1, 5}, {1, 5}}), "attend identical conflict");
    check(!Intervals::canAttendMeetings(Intervs{{1, 10}, {2, 3}, {10, 11}}), "attend nested conflict");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::canAttendMeetings(Intervs{{3, 3}}); }, "attend empty meeting");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::canAttendMeetings(Intervs{{4, 3}}); }, "attend malformed");

    check(Intervals::minMeetingRooms(Intervs{{0, 30}, {5, 10}, {15, 20}}) == 2, "rooms classic");
    check(Intervals::minMeetingRooms(Intervs{{7, 10}, {2, 4}}) == 1, "rooms disjoint");
    check(Intervals::minMeetingRooms(Intervs{{1, 2}, {2, 3}, {3, 4}}) == 1, "rooms back-to-back");
    check(Intervals::minMeetingRooms(Intervs{}) == 0, "rooms none");
    check(Intervals::minMeetingRooms(Intervs{{1, 5}, {1, 5}, {1, 5}}) == 3, "rooms identical");
    check(Intervals::minMeetingRooms(Intervs{{1, 10}, {2, 3}, {3, 4}, {4, 5}}) == 2, "rooms nested");
    check(Intervals::minMeetingRooms(Intervs{{kMin, kMax}, {0, 1}}) == 2, "rooms extremes");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::minMeetingRooms(Intervs{{2, 2}}); }, "rooms empty meeting");

    check(Intervals::minArrowsToBurstBalloons(Intervs{{10, 16}, {2, 8}, {1, 6}, {7, 12}}) == 2,
          "arrows classic");
    check(Intervals::minArrowsToBurstBalloons(Intervs{{1, 2}, {3, 4}, {5, 6}, {7, 8}}) == 4,
          "arrows disjoint");
    check(Intervals::minArrowsToBurstBalloons(Intervs{{1, 2}, {2, 3}, {3, 4}, {4, 5}}) == 2,
          "arrows touching closed");
    check(Intervals::minArrowsToBurstBalloons(Intervs{{1, 2}, {2, 3}}) == 1, "arrows touching pair");
    check(Intervals::minArrowsToBurstBalloons(Intervs{}) == 0, "arrows none");
    check(Intervals::minArrowsToBurstBalloons(Intervs{{5, 5}}) == 1, "arrows point balloon");
    check(Intervals::minArrowsToBurstBalloons(Intervs{{kMin, kMax}, {kMax, kMax}}) == 1,
          "arrows extremes no overflow");
    check(Intervals::minArrowsToBurstBalloons(Intervs{{kMin, kMin}, {kMax, kMax}}) == 2,
          "arrows extreme apart");
    check(Intervals::minArrowsToBurstBalloons(Intervs{{-3, -1}, {-2, 0}}) == 1, "arrows negative");
    checkThrows<std::invalid_argument>(
        [] { (void)Intervals::minArrowsToBurstBalloons(Intervs{{3, 1}}); }, "arrows malformed");
}

}  // namespace

int main()
{
    try {
        testTwoPointers();
        testSlidingWindow();
        testSlidingWindowExactCases();
        testBinarySearch();
        testIntervals();
    } catch (const std::exception& error) {
        std::cerr << "FAILED: " << error.what() << '\n';
        return 1;
    }
    std::cout << "remaining_batch_a_tests passed (" << checks << " checks)\n";
    return 0;
}
