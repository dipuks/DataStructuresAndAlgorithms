#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace dsa::two_pointers {

/// Static solutions for two-pointer interview problems (checklist 15-22).
///
/// Methods never keep references to their arguments. Methods that mutate
/// their input say so explicitly; precondition violations that can be checked
/// cheaply throw std::invalid_argument before any mutation happens.
class TwoPointers final {
public:
    TwoPointers() = delete;

    /// Problem 15 - Valid Palindrome.
    ///
    /// Considers only ASCII letters and digits, compared case-insensitively;
    /// every other byte (spaces, punctuation, non-ASCII bytes) is skipped.
    /// An empty or all-ignored string is a palindrome. Input is not modified.
    ///
    /// Technique: converging pointers; each skips ignored bytes, and the
    /// invariant is that everything outside [left, right) already matched.
    /// Example: "A man, a plan, a canal: Panama" -> true. Skip punctuation
    /// and case differences, then compare bytes from opposite ends.
    /// Time: O(n). Extra space: O(1).
    [[nodiscard]] static bool validPalindrome(std::string_view text) noexcept
    {
        std::size_t left = 0;
        std::size_t right = text.size();  // exclusive end
        while (true) {
            while (left < right && !isAlnum(text[left])) {
                ++left;
            }
            while (left < right && !isAlnum(text[right - 1])) {
                --right;
            }
            if (right - left < 2) {
                return true;
            }
            if (toLower(text[left]) != toLower(text[right - 1])) {
                return false;
            }
            ++left;
            --right;
        }
    }

    /// Problem 16 - 3Sum.
    ///
    /// Returns every unique value triplet {a, b, c} with a <= b <= c and
    /// a + b + c == 0. Triplets are listed in lexicographic order. Duplicate
    /// values in the input never produce duplicate triplets, while repeated
    /// values may be used together (e.g. {-1,-1,2}). Sums are computed in
    /// long long so extreme int values cannot overflow. Input is not modified;
    /// fewer than three values yield an empty result.
    ///
    /// Technique: sort a copy, fix the smallest value, then sweep the rest with
    /// converging pointers, skipping equal values to guarantee uniqueness.
    /// Time: O(n^2) (plus O(n log n) sort). Extra space: O(n) for the sorted
    /// copy, excluding the output.
    /// Example: [-1,0,1,2,-1,-4] -> [[-1,-1,2],[-1,0,1]]. After sorting,
    /// move left to increase a low sum and right to lower a high sum; skip
    /// equal values so the result contains no repeated triplets.
    [[nodiscard]] static std::vector<std::array<int, 3>> threeSum(
        std::span<const int> numbers)
    {
        std::vector<int> sorted(numbers.begin(), numbers.end());
        std::ranges::sort(sorted);

        std::vector<std::array<int, 3>> result;
        const std::size_t size = sorted.size();
        for (std::size_t first = 0; first + 2 < size; ++first) {
            if (first > 0 && sorted[first] == sorted[first - 1]) {
                continue;
            }
            std::size_t left = first + 1;
            std::size_t right = size - 1;
            while (left < right) {
                const long long sum = static_cast<long long>(sorted[first]) +
                                      sorted[left] + sorted[right];
                if (sum < 0) {
                    ++left;
                } else if (sum > 0) {
                    --right;
                } else {
                    result.push_back({sorted[first], sorted[left], sorted[right]});
                    const int leftValue = sorted[left];
                    const int rightValue = sorted[right];
                    while (left < right && sorted[left] == leftValue) {
                        ++left;
                    }
                    while (left < right && sorted[right] == rightValue) {
                        --right;
                    }
                }
            }
        }
        return result;
    }

    /// Problem 17 - 4Sum.
    ///
    /// Returns every unique value quadruplet {a, b, c, d} (a <= b <= c <= d)
    /// whose sum equals target, in lexicographic order. Values may repeat
    /// within a quadruplet only as often as they occur in the input. Sums use
    /// long long, so extreme ints and targets are handled without overflow.
    /// Input is not modified; fewer than four values yield an empty result.
    ///
    /// Technique: sort a copy, fix the two smallest values with nested loops
    /// (skipping duplicates), then converge two pointers over the remainder.
    /// Time: O(n^3). Extra space: O(n) for the sorted copy, excluding output.
    /// Example: [1,0,-1,0,-2,2], target = 0 ->
    /// [[-2,-1,1,2],[-2,0,0,2],[-1,0,0,1]]. Fix two values, then sweep for
    /// the remaining pair while skipping duplicates.
    [[nodiscard]] static std::vector<std::array<int, 4>> fourSum(
        std::span<const int> numbers, long long target)
    {
        std::vector<int> sorted(numbers.begin(), numbers.end());
        std::ranges::sort(sorted);

        std::vector<std::array<int, 4>> result;
        const std::size_t size = sorted.size();
        for (std::size_t first = 0; first + 3 < size; ++first) {
            if (first > 0 && sorted[first] == sorted[first - 1]) {
                continue;
            }
            for (std::size_t second = first + 1; second + 2 < size; ++second) {
                if (second > first + 1 && sorted[second] == sorted[second - 1]) {
                    continue;
                }
                std::size_t left = second + 1;
                std::size_t right = size - 1;
                while (left < right) {
                    const long long sum = static_cast<long long>(sorted[first]) +
                                          sorted[second] + sorted[left] +
                                          sorted[right];
                    if (sum < target) {
                        ++left;
                    } else if (sum > target) {
                        --right;
                    } else {
                        result.push_back({sorted[first], sorted[second],
                                          sorted[left], sorted[right]});
                        const int leftValue = sorted[left];
                        const int rightValue = sorted[right];
                        while (left < right && sorted[left] == leftValue) {
                            ++left;
                        }
                        while (left < right && sorted[right] == rightValue) {
                            --right;
                        }
                    }
                }
            }
        }
        return result;
    }

    /// Problem 18 - Container With Most Water.
    ///
    /// heights[i] is the height of a vertical line at x = i. Returns the
    /// largest area min(h[i], h[j]) * (j - i) over all pairs i < j, computed
    /// in long long. Fewer than two lines give 0. Input is not modified.
    /// Throws std::invalid_argument if any height is negative.
    ///
    /// Technique: start with the widest pair and always move the shorter
    /// line inward; the shorter line cannot be part of a better, narrower pair.
    /// Time: O(n). Extra space: O(1).
    /// Example: [1,8,6,2,5,4,8,3,7] -> 49. Width shrinks at every step, so
    /// move the shorter side; moving the taller side cannot improve the bound.
    [[nodiscard]] static long long maxArea(std::span<const int> heights)
    {
        requireNonNegative(heights);
        if (heights.size() < 2) {
            return 0;
        }
        std::size_t left = 0;
        std::size_t right = heights.size() - 1;
        long long best = 0;
        while (left < right) {
            const long long width = static_cast<long long>(right - left);
            const long long height = std::min(heights[left], heights[right]);
            best = std::max(best, width * height);
            if (heights[left] < heights[right]) {
                ++left;
            } else {
                --right;
            }
        }
        return best;
    }

    /// Problem 19 - Trapping Rain Water.
    ///
    /// Each element is the height of a unit-width bar. Returns the total
    /// units of water retained after rain, accumulated in long long. Fewer
    /// than three bars trap nothing. Input is not modified. Throws
    /// std::invalid_argument if any height is negative.
    ///
    /// Technique: converging pointers with running maxima; the side with the
    /// smaller bar is bounded by its own running maximum, so its level is final.
    /// Time: O(n). Extra space: O(1).
    /// Example: [0,1,0,2,1,0,1,3,2,1,2,1] -> 6 units. Resolve the lower side
    /// first because its running maximum determines the final water level.
    [[nodiscard]] static long long trapRainWater(std::span<const int> heights)
    {
        requireNonNegative(heights);
        if (heights.size() < 3) {
            return 0;
        }
        std::size_t left = 0;
        std::size_t right = heights.size() - 1;
        int leftMax = 0;
        int rightMax = 0;
        long long water = 0;
        while (left < right) {
            if (heights[left] < heights[right]) {
                leftMax = std::max(leftMax, heights[left]);
                water += leftMax - heights[left];
                ++left;
            } else {
                rightMax = std::max(rightMax, heights[right]);
                water += rightMax - heights[right];
                --right;
            }
        }
        return water;
    }

    /// Problem 20 - Sort Colors (Dutch national flag).
    ///
    /// Sorts colors in place into ascending order. Every value must be 0, 1
    /// or 2; otherwise std::invalid_argument is thrown BEFORE anything is
    /// modified. An empty span is valid.
    ///
    /// Technique: three regions - [0,low) zeros, [low,mid) ones, [high,end)
    /// twos - with mid scanning the unknown region [mid, high).
    /// Time: O(n), one pass. Extra space: O(1).
    /// Example: [2,0,2,1,1,0] -> [0,0,1,1,2,2]. The scan separates completed
    /// zero, one, and two regions from the still-unknown range.
    static void sortColors(std::span<int> colors)
    {
        for (const int color : colors) {
            if (color < 0 || color > 2) {
                throw std::invalid_argument(
                    "sortColors: every color must be 0, 1 or 2");
            }
        }

        std::size_t low = 0;
        std::size_t mid = 0;
        std::size_t high = colors.size();
        while (mid < high) {
            if (colors[mid] == 0) {
                std::swap(colors[low++], colors[mid++]);
            } else if (colors[mid] == 1) {
                ++mid;
            } else {
                std::swap(colors[mid], colors[--high]);
            }
        }
    }

    /// Problem 21 - Remove Duplicates from Sorted Array.
    ///
    /// Compacts a non-decreasing span in place so each distinct value appears
    /// once, and returns the logical length k. Elements [0, k) are the unique
    /// values in order; elements at [k, size) are unspecified. Throws
    /// std::invalid_argument, before any mutation, if the span is not sorted
    /// non-decreasingly. An empty span returns 0.
    ///
    /// Technique: slow/fast pointers; slow marks the end of the unique prefix.
    /// Time: O(n). Extra space: O(1).
    /// Example: [1,1,2] -> logical length 2 and prefix [1,2]. Advance the
    /// write index only when a value differs from the last kept value.
    [[nodiscard]] static std::size_t removeDuplicates(std::span<int> numbers)
    {
        if (!std::ranges::is_sorted(numbers)) {
            throw std::invalid_argument(
                "removeDuplicates: input must be sorted non-decreasingly");
        }
        if (numbers.empty()) {
            return 0;
        }
        std::size_t slow = 1;
        for (std::size_t fast = 1; fast < numbers.size(); ++fast) {
            if (numbers[fast] != numbers[slow - 1]) {
                numbers[slow++] = numbers[fast];
            }
        }
        return slow;
    }

    /// Problem 22 - Squares of a Sorted Array.
    ///
    /// Given values sorted non-decreasingly (negatives allowed), returns their
    /// squares sorted non-decreasingly. Squares are long long so that
    /// std::numeric_limits<int>::min() does not overflow. Throws
    /// std::invalid_argument if the input is not sorted. Input is not modified.
    ///
    /// Technique: the largest square is at one of the two ends, so fill the
    /// output from the back while converging two pointers.
    /// Time: O(n). Extra space: O(1) beyond the O(n) output.
    /// Example: [-4,-1,0,3,10] -> [0,1,9,16,100]. Compare endpoint squares
    /// and write the larger square into the result from right to left.
    [[nodiscard]] static std::vector<long long> sortedSquares(
        std::span<const int> numbers)
    {
        if (!std::ranges::is_sorted(numbers)) {
            throw std::invalid_argument(
                "sortedSquares: input must be sorted non-decreasingly");
        }
        std::vector<long long> result(numbers.size());
        std::size_t left = 0;
        std::size_t right = numbers.size();  // exclusive
        std::size_t out = numbers.size();
        while (left < right) {
            const long long leftSquare =
                static_cast<long long>(numbers[left]) * numbers[left];
            const long long rightSquare =
                static_cast<long long>(numbers[right - 1]) * numbers[right - 1];
            if (leftSquare > rightSquare) {
                result[--out] = leftSquare;
                ++left;
            } else {
                result[--out] = rightSquare;
                --right;
            }
        }
        return result;
    }

private:
    [[nodiscard]] static constexpr bool isAlnum(char c) noexcept
    {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z');
    }

    [[nodiscard]] static constexpr char toLower(char c) noexcept
    {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    }

    static void requireNonNegative(std::span<const int> heights)
    {
        for (const int height : heights) {
            if (height < 0) {
                throw std::invalid_argument("heights must be non-negative");
            }
        }
    }
};

}  // namespace dsa::two_pointers
