#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "dsa/arrays_hashing.hpp"

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

void testTwoSum()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{2, 7, 11, 15};
    const auto result = ArraysHashing::twoSum(numbers, 9);
    check(result == std::pair<std::size_t, std::size_t>{0, 1},
          "twoSum finds the expected pair");

    constexpr std::array repeated{3, 3};
    check(ArraysHashing::twoSum(repeated, 6) ==
              std::pair<std::size_t, std::size_t>{0, 1},
          "twoSum uses two different elements");

    constexpr std::array unmatched{1, 2, 3};
    check(!ArraysHashing::twoSum(unmatched, 10), "twoSum reports a missing pair");

    constexpr std::array<int, 0> empty{};
    check(!ArraysHashing::twoSum(empty, 0), "twoSum handles an empty range");

    constexpr std::array oneElement{std::numeric_limits<int>::min()};
    check(!ArraysHashing::twoSum(oneElement,
                                std::numeric_limits<long long>::max()),
          "twoSum safely handles an out-of-range target");
}

void testContainsDuplicate()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array repeated{1, 2, 3, 1};
    check(ArraysHashing::containsDuplicate(repeated),
          "containsDuplicate finds a duplicate");

    constexpr std::array unique{1, 2, 3};
    check(!ArraysHashing::containsDuplicate(unique),
          "containsDuplicate accepts unique values");

    constexpr std::array<int, 0> empty{};
    check(!ArraysHashing::containsDuplicate(empty),
          "containsDuplicate handles an empty range");
}

void testIsAnagram()
{
    using dsa::arrays_hashing::ArraysHashing;

    check(ArraysHashing::isAnagram("anagram", "nagaram"),
          "isAnagram accepts anagrams");
    check(!ArraysHashing::isAnagram("rat", "car"),
          "isAnagram rejects different letters");
    check(!ArraysHashing::isAnagram("ab", "a"),
          "isAnagram rejects different lengths");
    check(ArraysHashing::isAnagram("", ""),
          "isAnagram accepts two empty strings");
    check(!ArraysHashing::isAnagram("a", "A"),
          "isAnagram is case-sensitive");
}

void testGroupAnagrams()
{
    using dsa::arrays_hashing::ArraysHashing;

    const std::array<std::string, 6> words{
        "eat", "tea", "tan", "ate", "nat", "bat"};
    const auto groups = ArraysHashing::groupAnagrams(words);
    check(groups.size() == 3, "groupAnagrams finds three groups");
    check(groups[0] == std::vector<std::string>{"eat", "tea", "ate"},
          "groupAnagrams preserves input order within a group");
    check(groups[1] == std::vector<std::string>{"tan", "nat"},
          "groupAnagrams groups matching character frequencies");
    check(groups[2] == std::vector<std::string>{"bat"},
          "groupAnagrams keeps distinct groups separate");

    const std::array<std::string, 0> empty{};
    check(ArraysHashing::groupAnagrams(empty).empty(),
          "groupAnagrams handles an empty range");
}

void testTopKFrequentElements()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{1, 1, 1, 2, 2, 3};
    auto result = ArraysHashing::topKFrequentElements(numbers, 2);
    std::ranges::sort(result);
    check(result == std::vector<int>{1, 2},
          "topKFrequentElements returns the most frequent values");
    check(ArraysHashing::topKFrequentElements(numbers, 0).empty(),
          "topKFrequentElements handles k equal to zero");
    checkThrows<std::invalid_argument>(
        [&] { (void)ArraysHashing::topKFrequentElements(numbers, 4); },
        "topKFrequentElements rejects k larger than the distinct-value count");
}

void testProductExceptSelf()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{1, 2, 3, 4};
    check(ArraysHashing::productExceptSelf(numbers) ==
              std::vector<long long>{24, 12, 8, 6},
          "productExceptSelf returns products without division");

    constexpr std::array withZero{1, 2, 0, 4};
    check(ArraysHashing::productExceptSelf(withZero) ==
              std::vector<long long>{0, 0, 8, 0},
          "productExceptSelf handles a zero");

    constexpr std::array multipleZeros{
        std::numeric_limits<int>::max(), std::numeric_limits<int>::max(),
        std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), 0, 0};
    check(ArraysHashing::productExceptSelf(multipleZeros) ==
              std::vector<long long>(multipleZeros.size(), 0),
          "productExceptSelf avoids unused overflowing prefixes with multiple zeros");

    constexpr std::array negativeValues{-1, 2, 3};
    check(ArraysHashing::productExceptSelf(negativeValues) ==
              std::vector<long long>{6, -3, -2},
          "productExceptSelf handles negative values");

    constexpr int large = std::numeric_limits<int>::max();
    constexpr std::array largeNumbers{large, large, large};
    check(ArraysHashing::productExceptSelf(largeNumbers) ==
              std::vector<long long>{
                  static_cast<long long>(large) * large,
                  static_cast<long long>(large) * large,
                  static_cast<long long>(large) * large},
          "productExceptSelf does not calculate the unused full product");

    constexpr std::array overflowingProducts{large, large, large, large};
    checkThrows<std::overflow_error>(
        [&] { (void)ArraysHashing::productExceptSelf(overflowingProducts); },
        "productExceptSelf reports products outside the result type");
}

void testLongestConsecutiveSequence()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{100, 4, 200, 1, 3, 2};
    check(ArraysHashing::longestConsecutiveSequence(numbers) == 4,
          "longestConsecutiveSequence counts an unordered sequence");

    constexpr std::array extremes{
        std::numeric_limits<int>::min(), std::numeric_limits<int>::max()};
    check(ArraysHashing::longestConsecutiveSequence(extremes) == 1,
          "longestConsecutiveSequence safely handles integer boundaries");

    constexpr std::array<int, 0> empty{};
    check(ArraysHashing::longestConsecutiveSequence(empty) == 0,
          "longestConsecutiveSequence handles an empty range");
}

void testSubarraySumEqualsK()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{1, 1, 1};
    check(ArraysHashing::subarraySumEqualsK(numbers, 2) == 2,
          "subarraySumEqualsK counts matching positive subarrays");

    constexpr std::array negatives{1, -1, 0};
    check(ArraysHashing::subarraySumEqualsK(negatives, 0) == 3,
          "subarraySumEqualsK handles negative values and repeated prefixes");

    constexpr std::array<int, 0> empty{};
    check(ArraysHashing::subarraySumEqualsK(empty, 0) == 0,
          "subarraySumEqualsK handles an empty range");

    constexpr std::array positive{1};
    check(ArraysHashing::subarraySumEqualsK(
              positive, std::numeric_limits<long long>::min()) == 0,
          "subarraySumEqualsK safely handles an extreme target");
}

void testMaximumSubarraySum()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{-2, 1, -3, 4, -1, 2, 1, -5, 4};
    check(ArraysHashing::maximumSubarraySum(numbers) == 6,
          "maximumSubarraySum finds the best contiguous range");

    constexpr std::array negatives{-3, -2, -5};
    check(ArraysHashing::maximumSubarraySum(negatives) == -2,
          "maximumSubarraySum handles all-negative input");

    constexpr std::array<int, 0> empty{};
    check(!ArraysHashing::maximumSubarraySum(empty),
          "maximumSubarraySum reports empty input");
}

void testMajorityElement()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{2, 2, 1, 1, 1, 2, 2};
    check(ArraysHashing::majorityElement(numbers) == 2,
          "majorityElement finds a strict majority");

    constexpr std::array noMajority{1, 2, 3, 2};
    check(!ArraysHashing::majorityElement(noMajority),
          "majorityElement verifies that a majority exists");

    constexpr std::array<int, 0> empty{};
    check(!ArraysHashing::majorityElement(empty),
          "majorityElement handles an empty range");
}

void testFindDuplicateNumber()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{1, 3, 4, 2, 2};
    check(ArraysHashing::findDuplicateNumber(numbers) == 2,
          "findDuplicateNumber detects a repeated value in constant space");

    constexpr std::array invalid{0, 1};
    checkThrows<std::invalid_argument>(
        [&] { (void)ArraysHashing::findDuplicateNumber(invalid); },
        "findDuplicateNumber rejects values outside the required range");

    constexpr std::array<int, 0> empty{};
    checkThrows<std::invalid_argument>(
        [&] { (void)ArraysHashing::findDuplicateNumber(empty); },
        "findDuplicateNumber rejects an input that is too short");
}

void testFirstMissingPositive()
{
    using dsa::arrays_hashing::ArraysHashing;

    std::array numbers{3, 4, -1, 1};
    check(ArraysHashing::firstMissingPositive(numbers) == 2,
          "firstMissingPositive finds a missing middle value");

    std::array consecutive{1, 2, 0};
    check(ArraysHashing::firstMissingPositive(consecutive) == 3,
          "firstMissingPositive finds the first value after a prefix");

    std::array allPresent{1, 2, 3};
    check(ArraysHashing::firstMissingPositive(allPresent) == 4,
          "firstMissingPositive handles a complete positive prefix");

    std::array<int, 0> empty{};
    check(ArraysHashing::firstMissingPositive(empty) == 1,
          "firstMissingPositive handles an empty range");
}

void testSetMatrixZeroes()
{
    using dsa::arrays_hashing::ArraysHashing;

    std::vector<std::vector<int>> matrix{{1, 1, 1}, {1, 0, 1}, {1, 1, 1}};
    ArraysHashing::setMatrixZeroes(matrix);
    check(matrix == std::vector<std::vector<int>>{
                        {1, 0, 1}, {0, 0, 0}, {1, 0, 1}},
          "setMatrixZeroes clears the matching row and column");

    std::vector<std::vector<int>> zeroFirstColumn{{0, 1}, {1, 1}};
    ArraysHashing::setMatrixZeroes(zeroFirstColumn);
    check(zeroFirstColumn == std::vector<std::vector<int>>{{0, 0}, {0, 1}},
          "setMatrixZeroes handles a zero in the first column");

    std::vector<std::vector<int>> ragged{{1, 2}, {3}};
    checkThrows<std::invalid_argument>(
        [&] { ArraysHashing::setMatrixZeroes(ragged); },
        "setMatrixZeroes rejects ragged rows");
    check(ragged == std::vector<std::vector<int>>{{1, 2}, {3}},
          "setMatrixZeroes validates before modifying the matrix");
}

void testRotateImage()
{
    using dsa::arrays_hashing::ArraysHashing;

    std::vector<std::vector<int>> matrix{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    ArraysHashing::rotateImage(matrix);
    check(matrix == std::vector<std::vector<int>>{
                        {7, 4, 1}, {8, 5, 2}, {9, 6, 3}},
          "rotateImage rotates the square matrix clockwise");

    std::vector<std::vector<int>> notSquare{{1, 2, 3}, {4, 5, 6}};
    checkThrows<std::invalid_argument>(
        [&] { ArraysHashing::rotateImage(notSquare); },
        "rotateImage rejects a non-square matrix");
    check(notSquare == std::vector<std::vector<int>>{{1, 2, 3}, {4, 5, 6}},
          "rotateImage validates before modifying the matrix");
}

} // namespace

int main()
{
    try {
        testTwoSum();
        testContainsDuplicate();
        testIsAnagram();
        testGroupAnagrams();
        testTopKFrequentElements();
        testProductExceptSelf();
        testLongestConsecutiveSequence();
        testSubarraySumEqualsK();
        testMaximumSubarraySum();
        testMajorityElement();
        testFindDuplicateNumber();
        testFirstMissingPositive();
        testSetMatrixZeroes();
        testRotateImage();
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "Passed " << checks << " checks.\n";
    return 0;
}
