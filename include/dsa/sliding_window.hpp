#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <deque>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dsa::sliding_window {

/// Static solutions for sliding-window interview problems (checklist 23-33).
///
/// Strings are treated as sequences of bytes (256 symbols), so the character
/// alphabet is not restricted to lowercase letters. Inputs are never modified.
/// Cheaply checkable precondition violations throw std::invalid_argument.
class SlidingWindow final {
public:
    SlidingWindow() = delete;

    /// Problem 23 - Maximum Sum of a Subarray of Size K (fixed window).
    ///
    /// Returns the largest sum of k consecutive elements, accumulated in long
    /// long, or std::nullopt when k > numbers.size() (no such window exists).
    /// Throws std::invalid_argument if k == 0.
    ///
    /// Technique: maintain the current window sum; add the entering element
    /// and subtract the leaving one.
    /// Time: O(n). Extra space: O(1).
    /// Example: [2,1,5,1,3,2], k = 3 -> 9 from [5,1,3]. Remove the departing
    /// value and add the entering value to slide the fixed-width sum.
    [[nodiscard]] static std::optional<long long> maxSumSubarrayOfSizeK(
        std::span<const int> numbers, std::size_t k)
    {
        if (k == 0) {
            throw std::invalid_argument("maxSumSubarrayOfSizeK: k must be positive");
        }
        if (k > numbers.size()) {
            return std::nullopt;
        }
        long long sum = 0;
        for (std::size_t i = 0; i < k; ++i) {
            sum += numbers[i];
        }
        long long best = sum;
        for (std::size_t i = k; i < numbers.size(); ++i) {
            sum += numbers[i];
            sum -= numbers[i - k];
            best = std::max(best, sum);
        }
        return best;
    }

    /// Problem 24 - Longest Substring Without Repeating Characters.
    ///
    /// Returns the length of the longest substring whose bytes are all
    /// distinct; 0 for an empty string.
    ///
    /// Technique: the window [left, i] holds distinct bytes; on a repeat jump
    /// left just past the byte's previous occurrence.
    /// Time: O(n). Extra space: O(1) (fixed 256-entry table).
    /// Example: "abcabcbb" -> 3 from "abc". On a repeated byte, move the
    /// left edge past its last occurrence so the window stays unique.
    [[nodiscard]] static std::size_t longestSubstringWithoutRepeating(
        std::string_view text) noexcept
    {
        std::array<std::ptrdiff_t, kAlphabet> lastSeen;
        lastSeen.fill(-1);
        std::ptrdiff_t left = 0;
        std::ptrdiff_t best = 0;
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(text.size()); ++i) {
            const auto symbol = index(text[static_cast<std::size_t>(i)]);
            if (lastSeen[symbol] >= left) {
                left = lastSeen[symbol] + 1;
            }
            lastSeen[symbol] = i;
            best = std::max(best, i - left + 1);
        }
        return static_cast<std::size_t>(best);
    }

    /// Problem 25 - Longest Repeating Character Replacement.
    ///
    /// Returns the length of the longest substring that can be turned into a
    /// run of one repeated byte using at most k replacements. Empty input
    /// gives 0; k >= text.size() gives text.size().
    ///
    /// Technique: a window is valid when size - (count of its most frequent
    /// byte) <= k. The window never shrinks: it slides when invalid, so the
    /// stale historical maximum frequency can only matter when it yields a
    /// strictly larger window.
    /// Time: O(n). Extra space: O(1).
    /// Example: text = "AABABBA", k = 1 -> 4. A window is repairable when
    /// its length minus its most common byte count is at most k.
    [[nodiscard]] static std::size_t longestRepeatingCharacterReplacement(
        std::string_view text, std::size_t k) noexcept
    {
        std::array<std::size_t, kAlphabet> counts{};
        std::size_t left = 0;
        std::size_t maxCount = 0;
        for (std::size_t right = 0; right < text.size(); ++right) {
            maxCount = std::max(maxCount, ++counts[index(text[right])]);
            if (right - left + 1 - maxCount > k) {
                --counts[index(text[left])];
                ++left;
            }
        }
        return text.size() - left;
    }

    /// Problem 26 - Minimum Window Substring.
    ///
    /// Returns a copy of the shortest substring of text containing every byte
    /// of pattern with at least its multiplicity, or std::nullopt if none
    /// exists. If several shortest windows exist, the leftmost is returned.
    /// An empty pattern is vacuously satisfied by the empty string.
    ///
    /// Technique: expand right until the window covers the pattern, then
    /// shrink left while it still does, recording the best window.
    /// Time: O(n + m). Extra space: O(1) for counts, plus the O(answer) result.
    /// Example: text = "ADOBECODEBANC", pattern = "ABC" -> "BANC". Expand
    /// until all required byte counts are met, then shrink to the shortest.
    [[nodiscard]] static std::optional<std::string> minimumWindowSubstring(
        std::string_view text, std::string_view pattern)
    {
        if (pattern.empty()) {
            return std::string{};
        }
        std::array<std::ptrdiff_t, kAlphabet> need{};
        for (const char c : pattern) {
            ++need[index(c)];
        }
        std::size_t missing = pattern.size();
        std::size_t left = 0;
        std::size_t bestStart = 0;
        std::size_t bestLength = 0;  // 0 means no window found
        for (std::size_t right = 0; right < text.size(); ++right) {
            if (need[index(text[right])]-- > 0) {
                --missing;
            }
            while (missing == 0) {
                if (bestLength == 0 || right - left + 1 < bestLength) {
                    bestStart = left;
                    bestLength = right - left + 1;
                }
                if (++need[index(text[left])] > 0) {
                    ++missing;
                }
                ++left;
            }
        }
        if (bestLength == 0) {
            return std::nullopt;
        }
        return std::string(text.substr(bestStart, bestLength));
    }

    /// Problem 27 - Permutation in String.
    ///
    /// Returns true if some contiguous substring of text is a permutation of
    /// pattern (same byte multiset). An empty pattern is contained in any
    /// text, so it returns true; a pattern longer than text returns false.
    ///
    /// Technique: fixed-size window with a byte-count difference table.
    /// Time: O(n + m). Extra space: O(1).
    /// Example: pattern = "ab", text = "eidbaooo" -> true. Slide a width-2
    /// frequency window and compare its byte counts to the pattern.
    [[nodiscard]] static bool permutationInString(
        std::string_view pattern, std::string_view text)
    {
        return !anagramStarts(text, pattern, true).empty();
    }

    /// Problem 28 - Find All Anagrams in a String.
    ///
    /// Returns, in ascending order, every start index i such that
    /// text.substr(i, pattern.size()) is a permutation of pattern. Overlapping
    /// matches are all reported. A pattern longer than text yields an empty
    /// result; an empty pattern matches at every index 0..text.size().
    ///
    /// Technique: fixed-size window with a byte-count difference table and a
    /// counter of unbalanced symbols updated in O(1) per slide.
    /// Time: O(n + m). Extra space: O(1) beyond the output.
    /// Example: text = "cbaebabacd", pattern = "abc" -> indices [0,6].
    /// Maintain the same count invariant as permutationInString and report
    /// every matching window, including overlapping matches.
    [[nodiscard]] static std::vector<std::size_t> findAllAnagrams(
        std::string_view text, std::string_view pattern)
    {
        return anagramStarts(text, pattern, false);
    }

    /// Problem 29 - Minimum Size Subarray Sum.
    ///
    /// Returns the length of the shortest contiguous subarray whose sum is at
    /// least target, or 0 if no subarray qualifies. Elements must be
    /// non-negative (required for the window to be monotone) and target must
    /// be positive; otherwise std::invalid_argument is thrown.
    ///
    /// Technique: grow the right edge, then shrink the left edge while the
    /// sum still reaches target. Sums use long long.
    /// Time: O(n). Extra space: O(1).
    /// Example: target = 7, numbers = [2,3,1,2,4,3] -> length 2 ([4,3]).
    /// Nonnegative values make shrinking safe once the sum reaches target.
    [[nodiscard]] static std::size_t minSubArrayLen(
        long long target, std::span<const int> numbers)
    {
        if (target <= 0) {
            throw std::invalid_argument("minSubArrayLen: target must be positive");
        }
        for (const int value : numbers) {
            if (value < 0) {
                throw std::invalid_argument(
                    "minSubArrayLen: elements must be non-negative");
            }
        }
        long long sum = 0;
        std::size_t left = 0;
        std::size_t best = 0;
        for (std::size_t right = 0; right < numbers.size(); ++right) {
            sum += numbers[right];
            while (sum >= target) {
                const std::size_t length = right - left + 1;
                best = (best == 0) ? length : std::min(best, length);
                sum -= numbers[left++];
            }
        }
        return best;
    }

    /// Problem 30 - Max Consecutive Ones III.
    ///
    /// bits must contain only 0 and 1 (std::invalid_argument otherwise).
    /// Returns the longest run of ones obtainable by flipping at most k zeros;
    /// empty input gives 0.
    ///
    /// Technique: variable window that holds at most k zeros.
    /// Time: O(n). Extra space: O(1).
    /// Example: bits = [1,1,1,0,0,0,1,1,1,1,0], k = 2 -> 6. Keep at most k
    /// zeros inside the current window and discard from the left as needed.
    [[nodiscard]] static std::size_t longestOnes(
        std::span<const int> bits, std::size_t k)
    {
        for (const int bit : bits) {
            if (bit != 0 && bit != 1) {
                throw std::invalid_argument("longestOnes: bits must be 0 or 1");
            }
        }
        std::size_t zeros = 0;
        std::size_t left = 0;
        std::size_t best = 0;
        for (std::size_t right = 0; right < bits.size(); ++right) {
            zeros += (bits[right] == 0);
            while (zeros > k) {
                zeros -= (bits[left++] == 0);
            }
            best = std::max(best, right - left + 1);
        }
        return best;
    }

    /// Problem 31 - Longest Substring with At Most K Distinct Characters.
    ///
    /// Returns the length of the longest substring with at most k distinct
    /// bytes. k == 0 gives 0 (only the empty substring qualifies). Fruit Into
    /// Baskets is the k == 2 case.
    ///
    /// Technique: variable window with per-byte counts and a distinct counter.
    /// Time: O(n). Extra space: O(1).
    /// Example: "eceba", k = 2 -> 3 from "ece". Shrink until the frequency
    /// table contains no more than k distinct bytes.
    [[nodiscard]] static std::size_t longestSubstringAtMostKDistinct(
        std::string_view text, std::size_t k) noexcept
    {
        if (k == 0) {
            return 0;
        }
        std::array<std::size_t, kAlphabet> counts{};
        std::size_t distinct = 0;
        std::size_t left = 0;
        std::size_t best = 0;
        for (std::size_t right = 0; right < text.size(); ++right) {
            distinct += (counts[index(text[right])]++ == 0);
            while (distinct > k) {
                distinct -= (--counts[index(text[left++])] == 0);
            }
            best = std::max(best, right - left + 1);
        }
        return best;
    }

    /// Problem 32 - Sliding Window Maximum.
    ///
    /// Returns the maximum of every window of k consecutive elements
    /// (numbers.size() - k + 1 values, left to right). Empty input returns an
    /// empty result. Throws std::invalid_argument if k == 0 or
    /// k > numbers.size() for non-empty input.
    ///
    /// Technique: a monotonic deque of indices whose values are strictly
    /// decreasing; the front is always the current window maximum.
    /// Time: O(n) amortized. Extra space: O(k) for the deque.
    /// Example: [1,3,-1,-3,5,3,6,7], k = 3 -> [3,3,5,5,6,7]. Keep candidate
    /// indices decreasing by value and remove expired indices from the front.
    [[nodiscard]] static std::vector<int> slidingWindowMaximum(
        std::span<const int> numbers, std::size_t k)
    {
        if (numbers.empty()) {
            return {};
        }
        if (k == 0 || k > numbers.size()) {
            throw std::invalid_argument(
                "slidingWindowMaximum: k must be in [1, size]");
        }
        std::vector<int> result;
        result.reserve(numbers.size() - k + 1);
        std::deque<std::size_t> candidates;
        for (std::size_t i = 0; i < numbers.size(); ++i) {
            if (!candidates.empty() && candidates.front() + k <= i) {
                candidates.pop_front();
            }
            while (!candidates.empty() && numbers[candidates.back()] <= numbers[i]) {
                candidates.pop_back();
            }
            candidates.push_back(i);
            if (i + 1 >= k) {
                result.push_back(numbers[candidates.front()]);
            }
        }
        return result;
    }

    /// Problem 33 - Subarray Product Less Than K.
    ///
    /// Counts contiguous non-empty subarrays whose product is strictly less
    /// than k. All elements must be positive (std::invalid_argument
    /// otherwise). k <= 1 gives 0. The running product never exceeds k - 1, and
    /// the multiplication is guarded by a division, so no overflow occurs even
    /// for k near the long long maximum.
    ///
    /// Technique: for each right edge, shrink left until the product is < k;
    /// every subarray ending at right and starting in [left, right] counts.
    /// Time: O(n). Extra space: O(1).
    /// Example: [10,5,2,6], k = 100 -> 8 subarrays. After restoring product
    /// below k, every suffix of the current window is also valid.
    [[nodiscard]] static long long numSubarrayProductLessThanK(
        std::span<const int> numbers, long long k)
    {
        for (const int value : numbers) {
            if (value <= 0) {
                throw std::invalid_argument(
                    "numSubarrayProductLessThanK: elements must be positive");
            }
        }
        if (k <= 1) {
            return 0;
        }
        const long long limit = k - 1;  // product must be <= limit
        long long product = 1;
        std::size_t left = 0;
        long long count = 0;
        for (std::size_t right = 0; right < numbers.size(); ++right) {
            const long long value = numbers[right];
            while (left < right && product > limit / value) {
                product /= numbers[left++];
            }
            if (value > limit) {
                left = right + 1;
                product = 1;
                continue;
            }
            product *= value;
            count += static_cast<long long>(right - left + 1);
        }
        return count;
    }

    /// Exactly K Distinct Integers - count of subarrays (checklist #30).
    ///
    /// Counts contiguous non-empty subarrays containing EXACTLY k distinct
    /// values. Subarrays are counted by position, so equal subarrays at
    /// different positions each count. k == 0 gives 0 (a non-empty subarray
    /// has at least one distinct value); empty input gives 0. Input is not
    /// modified. The count is long long (up to n(n+1)/2).
    ///
    /// Technique: exactly(k) = atMost(k) - atMost(k - 1); each atMost call is
    /// a variable window whose size is the number of valid subarrays ending
    /// at the right edge.
    /// Time: O(n) average (hash-map counts; O(n^2) worst case with
    /// collisions). Extra space: O(min(n, distinct values)).
    /// Example: [1,2,1,2,3], k = 2 -> 7 subarrays. Count at-most-2 windows
    /// and subtract at-most-1 windows to isolate exactly two distinct values.
    [[nodiscard]] static long long subarraysWithKDistinct(
        std::span<const int> numbers, std::size_t k)
    {
        if (k == 0) {
            return 0;
        }
        return countAtMostKDistinct(numbers, k) - countAtMostKDistinct(numbers, k - 1);
    }

    /// Longest Subarray of 1's After Deleting One Element (checklist #32).
    ///
    /// bits must contain only 0 and 1 (std::invalid_argument otherwise).
    /// Exactly one element MUST be deleted; returns the length of the longest
    /// non-empty run of 1s in the result. If the input is all ones the answer
    /// is size - 1. Empty input and single-element input give 0. Input is not
    /// modified.
    ///
    /// Technique: variable window holding at most one zero; that zero (or,
    /// without one, any element) is the deleted element, so the candidate is
    /// window length - 1.
    /// Time: O(n). Extra space: O(1).
    /// Example: [1,1,0,1] -> 3. Keep at most one zero, then subtract one from
    /// each candidate window because exactly one element must be deleted.
    [[nodiscard]] static std::size_t longestSubarrayAfterDeletingOne(
        std::span<const int> bits)
    {
        for (const int bit : bits) {
            if (bit != 0 && bit != 1) {
                throw std::invalid_argument(
                    "longestSubarrayAfterDeletingOne: bits must be 0 or 1");
            }
        }
        std::size_t zeros = 0;
        std::size_t left = 0;
        std::size_t best = 0;
        for (std::size_t right = 0; right < bits.size(); ++right) {
            zeros += (bits[right] == 0);
            while (zeros > 1) {
                zeros -= (bits[left++] == 0);
            }
            best = std::max(best, right - left);  // window length - 1
        }
        return best;
    }

private:
    [[nodiscard]] static long long countAtMostKDistinct(
        std::span<const int> numbers, std::size_t k)
    {
        std::unordered_map<int, std::size_t> counts;
        std::size_t left = 0;
        long long total = 0;
        for (std::size_t right = 0; right < numbers.size(); ++right) {
            ++counts[numbers[right]];
            while (counts.size() > k) {
                if (--counts[numbers[left]] == 0) {
                    counts.erase(numbers[left]);
                }
                ++left;
            }
            total += static_cast<long long>(right - left + 1);
        }
        return total;
    }
    static constexpr std::size_t kAlphabet = 256;

    [[nodiscard]] static constexpr std::size_t index(char c) noexcept
    {
        return static_cast<unsigned char>(c);
    }

    // Start indices of windows of text that are permutations of pattern.
    [[nodiscard]] static std::vector<std::size_t> anagramStarts(
        std::string_view text, std::string_view pattern, bool firstOnly)
    {
        std::vector<std::size_t> starts;
        const std::size_t width = pattern.size();
        if (width > text.size()) {
            return starts;
        }
        if (width == 0) {
            for (std::size_t i = 0; i <= text.size(); ++i) {
                starts.push_back(i);
                if (firstOnly) {
                    break;
                }
            }
            return starts;
        }

        // balance[c] = (count in pattern) - (count in window); unbalanced
        // tracks how many symbols have a non-zero balance.
        std::array<int, kAlphabet> balance{};
        std::size_t unbalanced = 0;
        const auto adjust = [&](char c, int delta) {
            int& entry = balance[index(c)];
            if (entry == 0) {
                ++unbalanced;
            }
            entry += delta;
            if (entry == 0) {
                --unbalanced;
            }
        };

        for (const char c : pattern) {
            adjust(c, 1);
        }
        for (std::size_t i = 0; i < text.size(); ++i) {
            adjust(text[i], -1);
            if (i >= width) {
                adjust(text[i - width], 1);
            }
            if (i + 1 >= width && unbalanced == 0) {
                starts.push_back(i + 1 - width);
                if (firstOnly) {
                    break;
                }
            }
        }
        return starts;
    }
};

}  // namespace dsa::sliding_window
