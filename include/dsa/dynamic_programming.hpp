#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace dsa::dynamic_programming {

/// Static dynamic-programming solutions for interview problems.
class DynamicProgramming final {
public:
    DynamicProgramming() = delete;

    /// Counts ways to reach step n using 1- or 2-step moves. n=0 has one
    /// empty sequence. Throws std::overflow_error if the count exceeds
    /// uint64_t. Input is a nonnegative step count. Time: O(n); extra space:
    /// O(1). Invariant: previous/current hold the counts for consecutive
    /// stair indices ending immediately before the next iteration.
    /// Example: n = 5 -> 8 routes. The recurrence adds the counts for the
    /// one-step and two-step predecessors, so only two values are retained.
    [[nodiscard]] static std::uint64_t climbingStairs(std::size_t n)
    {
        if (n == 0) {
            return 1;
        }
        std::uint64_t previous = 1;
        std::uint64_t current = 1;
        for (std::size_t step = 1; step < n; ++step) {
            const std::uint64_t next = checkedAdd(previous, current);
            previous = current;
            current = next;
        }
        return current;
    }

    /// Returns the maximum sum of nonadjacent values; empty input and
    /// negative-only input yield zero. Values may be negative; input is
    /// unchanged. Throws std::overflow_error if the result cannot fit in
    /// long long. Time: O(n); extra space: O(1). Invariant: oneBack and
    /// twoBack are the best sums through the preceding and second-preceding
    /// house, respectively.
    /// Example: [2,7,9,3,1] -> 12 (rob 2, 9, and 1). Compare skipping each
    /// house with taking it plus the best result two houses earlier.
    [[nodiscard]] static long long houseRobber(std::span<const int> values)
    {
        long long twoBack = 0;
        long long oneBack = 0;
        for (const int value : values) {
            const long long take = checkedAdd(twoBack, value);
            const long long current = std::max(oneBack, take);
            twoBack = oneBack;
            oneBack = current;
        }
        return oneBack;
    }

    /// Circular variant of houseRobber: first and last positions are adjacent.
    /// Empty and negative-only inputs yield zero. Throws std::overflow_error
    /// for an unrepresentable sum. Input is unchanged. Time: O(n); extra
    /// space: O(1). Invariant: each pass solves one contiguous range that
    /// excludes one endpoint of the circle.
    /// Example: [2,3,2] -> 3. Solve ranges excluding the first and last
    /// house separately, then choose the larger valid result.
    [[nodiscard]] static long long houseRobberII(std::span<const int> values)
    {
        if (values.size() <= 1) {
            return values.empty() ? 0 : std::max(0, values.front());
        }
        return std::max(robRange(values, 0, values.size() - 1),
                        robRange(values, 1, values.size()));
    }

    /// Returns the minimum number of positive coin denominations needed to
    /// make amount, or nullopt when impossible. Amount must be nonnegative
    /// and every coin positive, otherwise std::invalid_argument is thrown.
    /// Input is unchanged. Time: O(amount * coin_count); extra space:
    /// O(amount). Invariant: best[x] is the minimum coins found for every
    /// amount x processed so far.
    /// Example: coins = [1,2,5], amount = 11 -> 3 coins. For each amount,
    /// try each denomination and extend the best solution for the remainder.
    [[nodiscard]] static std::optional<std::size_t> coinChange(
        std::span<const int> coins,
        int amount)
    {
        if (amount < 0) {
            throw std::invalid_argument("amount must be nonnegative");
        }
        validateCoins(coins);
        const std::size_t unreachable = static_cast<std::size_t>(amount) + 1;
        std::vector<std::size_t> best(static_cast<std::size_t>(amount) + 1,
                                      unreachable);
        best[0] = 0;
        for (std::size_t value = 1; value <= static_cast<std::size_t>(amount);
             ++value) {
            for (const int coin : coins) {
                const std::size_t denomination =
                    static_cast<std::size_t>(coin);
                if (denomination <= value) {
                    best[value] =
                        std::min(best[value], best[value - denomination] + 1);
                }
            }
        }
        if (best[static_cast<std::size_t>(amount)] == unreachable) {
            return std::nullopt;
        }
        return best[static_cast<std::size_t>(amount)];
    }

    /// Counts unordered combinations of unlimited positive coins making
    /// amount. Amount must be nonnegative and coins positive or
    /// std::invalid_argument is thrown. Duplicate denominations are
    /// deduplicated. Throws std::overflow_error if the count exceeds uint64_t.
    /// Input is unchanged. Time: O(amount * distinct_coin_count); extra
    /// space: O(amount). Invariant: after each denomination, ways[x] counts
    /// combinations using only denominations processed so far.
    /// Example: coins = [1,2,5], amount = 5 -> 4 combinations. Process one
    /// denomination at a time and update upward to allow unlimited reuse.
    [[nodiscard]] static std::uint64_t coinChangeII(
        std::span<const int> coins,
        int amount)
    {
        if (amount < 0) {
            throw std::invalid_argument("amount must be nonnegative");
        }
        validateCoins(coins);
        std::vector<int> unique(coins.begin(), coins.end());
        std::ranges::sort(unique);
        unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
        std::vector<std::uint64_t> ways(static_cast<std::size_t>(amount) + 1, 0);
        ways[0] = 1;
        const std::size_t limit = static_cast<std::size_t>(amount);
        for (const int coin : unique) {
            const std::size_t denomination = static_cast<std::size_t>(coin);
            for (std::size_t value = denomination; value <= limit; ++value) {
                ways[value] =
                    checkedAdd(ways[value], ways[value - denomination]);
            }
        }
        return ways[static_cast<std::size_t>(amount)];
    }

    /// Returns the length of the longest strictly increasing subsequence.
    /// Any integer values are accepted; input is unchanged. Time: O(n log n);
    /// extra space: O(n). Invariant: tails[length-1] is the smallest tail
    /// known for an increasing subsequence of that length.
    /// Example: [10,9,2,5,3,7,101,18] -> 4. Replace the first tail >= each
    /// value; the tail list compactly represents achievable subsequence sizes.
    [[nodiscard]] static std::size_t longestIncreasingSubsequence(
        std::span<const int> values)
    {
        std::vector<int> tails;
        tails.reserve(values.size());
        for (const int value : values) {
            const auto position =
                std::ranges::lower_bound(tails, value);
            if (position == tails.end()) {
                tails.push_back(value);
            } else {
                *position = value;
            }
        }
        return tails.size();
    }

    /// Reports whether nonnegative values can be split into equal-sum
    /// subsets. Negative values throw std::invalid_argument. Empty input is
    /// true. Uses a one-dimensional subset-sum table.
    /// Input is unchanged. Time: O(n * sum); extra space: O(sum). Invariant:
    /// possible[s] is true exactly when a subset of processed values sums to s.
    /// Example: [1,5,11,5] -> true (11 equals 1+5+5). Update sums downward
    /// so the current value cannot be reused during the same iteration.
    [[nodiscard]] static bool canPartition(std::span<const int> values)
    {
        long long total = 0;
        for (const int value : values) {
            if (value < 0) {
                throw std::invalid_argument("values must be nonnegative");
            }
            total = checkedAdd(total, value);
        }
        if (total % 2 != 0) {
            return false;
        }
        const auto target = checkedSize(total / 2);
        std::vector<bool> possible(target + 1, false);
        possible[0] = true;
        for (const int value : values) {
            const std::size_t item = static_cast<std::size_t>(value);
            if (item > target) {
                continue;
            }
            for (std::size_t sum = target + 1; sum > item; --sum) {
                possible[sum - 1] =
                    possible[sum - 1] || possible[sum - 1 - item];
            }
        }
        return possible[target];
    }

    /// Counts assignments of +/- signs whose sum is target. Values must be
    /// nonnegative or std::invalid_argument is thrown. Returns zero when no
    /// assignment exists; throws std::overflow_error if the count exceeds
    /// uint64_t. Input is unchanged. Time: O(n * sum); extra space: O(sum).
    /// Invariant: ways[s] counts subsets of processed values totaling s;
    /// descending updates prevent reusing an item.
    /// Example: values = [1,1,1,1,1], target = 3 -> 5 assignments. Convert
    /// sign choices to subset sums, then count each subset exactly once.
    [[nodiscard]] static std::uint64_t targetSumWays(
        std::span<const int> values,
        long long target)
    {
        long long total = 0;
        for (const int value : values) {
            if (value < 0) {
                throw std::invalid_argument("values must be nonnegative");
            }
            total = checkedAdd(total, value);
        }
        if (target > total || target < -total) {
            return 0;
        }
        const long long subsetNumerator =
            target >= 0 ? total - target : total + target;
        if (subsetNumerator % 2 != 0) {
            return 0;
        }
        const auto subsetTarget = checkedSize(subsetNumerator / 2);
        std::vector<std::uint64_t> ways(subsetTarget + 1, 0);
        ways[0] = 1;
        for (const int value : values) {
            const std::size_t item = static_cast<std::size_t>(value);
            if (item > subsetTarget) {
                continue;
            }
            for (std::size_t sum = subsetTarget + 1; sum > item; --sum) {
                ways[sum - 1] = checkedAdd(
                    ways[sum - 1], ways[sum - 1 - item]);
            }
        }
        return ways[subsetTarget];
    }

    /// Counts decodings of a digit string with 1..26 mapped to letters.
    /// Nondigit input throws std::invalid_argument; the empty string has zero
    /// decodings. Throws std::overflow_error if the count exceeds uint64_t.
    /// The string is not modified. Time: O(n); extra space: O(1). Invariant:
    /// twoBack and oneBack are the counts through the two preceding prefixes.
    /// Example: "226" -> 3 ("BZ", "VF", "BBF"). A valid one-digit suffix
    /// uses the prior count; a valid two-digit suffix adds the older count.
    [[nodiscard]] static std::uint64_t decodeWays(std::string_view digits)
    {
        if (digits.empty()) {
            return 0;
        }
        std::uint64_t twoBack = 1;
        std::uint64_t oneBack = digits.front() == '0' ? 0 : 1;
        if (!isDigit(digits.front())) {
            throw std::invalid_argument("input must contain only digits");
        }
        for (std::size_t index = 1; index < digits.size(); ++index) {
            if (!isDigit(digits[index])) {
                throw std::invalid_argument("input must contain only digits");
            }
            std::uint64_t current =
                digits[index] == '0' ? 0 : oneBack;
            const int pair =
                (digits[index - 1] - '0') * 10 + (digits[index] - '0');
            if (pair >= 10 && pair <= 26) {
                current = checkedAdd(current, twoBack);
            }
            twoBack = oneBack;
            oneBack = current;
        }
        return oneBack;
    }

    /// Reports whether text can be segmented into dictionary words.
    /// Empty text is segmentable; empty dictionary entries are ignored.
    /// Inputs are not modified. Time: O(n^2) average hash-table operations;
    /// extra space: O(n + total dictionary bytes). Invariant: reachable[i]
    /// means the prefix ending at i has a dictionary segmentation.
    /// Example: text = "leetcode", words = ["leet","code"] -> true. Mark a
    /// prefix reachable when a reachable earlier cut leaves a dictionary word.
    [[nodiscard]] static bool wordBreak(
        std::string_view text,
        std::span<const std::string> dictionary)
    {
        std::unordered_set<std::string_view> words;
        words.reserve(dictionary.size());
        std::size_t maximumWordLength = 0;
        for (const std::string& word : dictionary) {
            if (!word.empty()) {
                words.emplace(word);
                maximumWordLength = std::max(maximumWordLength, word.size());
            }
        }
        std::vector<bool> reachable(text.size() + 1, false);
        reachable[0] = true;
        for (std::size_t end = 1; end <= text.size(); ++end) {
            const std::size_t first =
                end > maximumWordLength ? end - maximumWordLength : 0;
            for (std::size_t start = first; start < end; ++start) {
                if (reachable[start] &&
                    words.contains(text.substr(start, end - start))) {
                    reachable[end] = true;
                    break;
                }
            }
        }
        return reachable[text.size()];
    }

private:
    static std::uint64_t checkedAdd(std::uint64_t first, std::uint64_t second)
    {
        if (first > std::numeric_limits<std::uint64_t>::max() - second) {
            throw std::overflow_error("count exceeds uint64_t");
        }
        return first + second;
    }

    static long long checkedAdd(long long first, long long second)
    {
        if ((second > 0 &&
             first > std::numeric_limits<long long>::max() - second) ||
            (second < 0 &&
             first < std::numeric_limits<long long>::min() - second)) {
            throw std::overflow_error("sum does not fit in long long");
        }
        return first + second;
    }

    static void validateCoins(std::span<const int> coins)
    {
        if (std::ranges::any_of(coins, [](int coin) { return coin <= 0; })) {
            throw std::invalid_argument("coin denominations must be positive");
        }
    }

    static std::size_t checkedSize(long long value)
    {
        if (value < 0 ||
            static_cast<unsigned long long>(value) >
                static_cast<unsigned long long>(
                    std::numeric_limits<std::size_t>::max())) {
            throw std::length_error("dynamic-programming table is too large");
        }
        const std::size_t result = static_cast<std::size_t>(value);
        if (result == std::numeric_limits<std::size_t>::max()) {
            throw std::length_error("dynamic-programming table is too large");
        }
        return result;
    }

    static long long robRange(
        std::span<const int> values,
        std::size_t begin,
        std::size_t end)
    {
        long long twoBack = 0;
        long long oneBack = 0;
        for (std::size_t index = begin; index < end; ++index) {
            const long long current =
                std::max(oneBack, checkedAdd(twoBack, values[index]));
            twoBack = oneBack;
            oneBack = current;
        }
        return oneBack;
    }

    static bool isDigit(char character) noexcept
    {
        return character >= '0' && character <= '9';
    }
};

} // namespace dsa::dynamic_programming
