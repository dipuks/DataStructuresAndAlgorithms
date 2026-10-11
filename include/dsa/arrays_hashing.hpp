#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dsa::arrays_hashing {

/// Static solutions for array and hash-table interview problems.
class ArraysHashing final {
public:
    ArraysHashing() = delete;

    /// Finds two distinct indices whose values add up to target.
    ///
    /// Returns the first pair found while scanning from left to right, or
    /// std::nullopt if no such pair exists. The input is not modified.
    /// Example: numbers = [2, 7, 11, 15], target = 9 -> indices [0, 1].
    /// Walkthrough: store each earlier value's index; at 7, the stored 2
    /// completes the target, so return before inserting the current value.
    ///
    /// Time: O(n) average; O(n^2) worst case with hash collisions.
    /// Extra space: O(n).
    [[nodiscard]] static std::optional<std::pair<std::size_t, std::size_t>>
    twoSum(std::span<const int> numbers, long long target)
    {
        std::unordered_map<int, std::size_t> firstIndex;
        firstIndex.reserve(numbers.size());

        for (std::size_t index = 0; index < numbers.size(); ++index) {
            const int number = numbers[index];

            // Skip subtraction only when it would overflow long long. In
            // that case, no int value in the input can be the required complement.
            if ((number > 0 &&
                 target < std::numeric_limits<long long>::min() + number) ||
                (number < 0 &&
                 target > std::numeric_limits<long long>::max() + number)) {
                firstIndex.try_emplace(number, index);
                continue;
            }

            const long long complement = target - number;
            if (complement >= std::numeric_limits<int>::min() &&
                complement <= std::numeric_limits<int>::max()) {
                const auto match = firstIndex.find(static_cast<int>(complement));
                if (match != firstIndex.end()) {
                    return std::pair{match->second, index};
                }
            }

            firstIndex.try_emplace(number, index);
        }

        return std::nullopt;
    }

    /// Groups strings containing the same bytes; string order is preserved
    /// within each group, and groups appear in the order of their first member.
    /// Example: ["eat", "tea", "tan", "ate"] ->
    /// [["eat", "tea", "ate"], ["tan"]]. Sorting each word gives a shared key.
    ///
    /// Time: O(n * k log k), where n is the string count and k is their
    /// maximum length. Extra space: O(n * k) for the result and hash table.
    [[nodiscard]] static std::vector<std::vector<std::string>> groupAnagrams(
        std::span<const std::string> words)
    {
        std::vector<std::vector<std::string>> groups;
        std::unordered_map<std::string, std::size_t> groupByKey;
        groupByKey.reserve(words.size());

        for (const std::string& word : words) {
            std::string key = word;
            std::ranges::sort(key);

            const auto [entry, inserted] =
                groupByKey.try_emplace(key, groups.size());
            if (inserted) {
                groups.emplace_back();
            }
            groups[entry->second].push_back(word);
        }

        return groups;
    }

    /// Returns the k values with the highest frequencies.
    ///
    /// When frequencies tie, their relative order is unspecified. Throws
    /// std::invalid_argument if k exceeds the number of distinct input values.
    /// Example: numbers = [1,1,1,2,2,3], k = 2 -> [1,2] (tie order unspecified).
    /// Walkthrough: count values, bucket them by frequency, then read buckets
    /// from highest frequency downward until k values have been collected.
    ///
    /// Time: O(n) average; O(n^2) worst case with hash collisions.
    /// Extra space: O(n).
    [[nodiscard]] static std::vector<int> topKFrequentElements(
        std::span<const int> numbers,
        std::size_t k)
    {
        std::unordered_map<int, std::size_t> frequencies;
        frequencies.reserve(numbers.size());
        for (const int number : numbers) {
            ++frequencies[number];
        }

        if (k > frequencies.size()) {
            throw std::invalid_argument("k exceeds the number of distinct values");
        }

        std::vector<std::vector<int>> buckets(numbers.size() + 1);
        for (const auto& [number, frequency] : frequencies) {
            buckets[frequency].push_back(number);
        }

        std::vector<int> result;
        result.reserve(k);
        for (std::size_t frequency = buckets.size();
             frequency > 0 && result.size() < k;
             --frequency) {
            for (const int number : buckets[frequency - 1]) {
                result.push_back(number);
                if (result.size() == k) {
                    break;
                }
            }
        }

        return result;
    }

    /// Returns, for each position, the product of every other input value.
    ///
    /// Products that cannot be represented by long long throw
    /// std::overflow_error. The input is not modified.
    /// Example: [1,2,3,4] -> [24,12,8,6]. The first pass writes prefixes;
    /// the reverse pass multiplies in suffixes, without dividing by values.
    ///
    /// Time: O(n). Extra space: O(1) beyond the returned vector.
    [[nodiscard]] static std::vector<long long> productExceptSelf(
        std::span<const int> numbers)
    {
        std::vector<long long> products(numbers.size(), 1);
        const std::size_t zeroCount = static_cast<std::size_t>(
            std::ranges::count(numbers, 0));
        if (zeroCount > 1) {
            std::ranges::fill(products, 0);
            return products;
        }

        long long prefix = 1;
        for (std::size_t index = 0; index < numbers.size(); ++index) {
            products[index] = prefix;
            if (index + 1 < numbers.size()) {
                prefix = multiplyChecked(prefix, numbers[index]);
            }
        }

        long long suffix = 1;
        for (std::size_t index = numbers.size(); index > 0; --index) {
            products[index - 1] =
                multiplyChecked(products[index - 1], suffix);
            if (index > 1) {
                suffix = multiplyChecked(suffix, numbers[index - 1]);
            }
        }

        return products;
    }

    /// Returns the length of the longest consecutive-value sequence.
    ///
    /// The input order is irrelevant and the input is not modified.
    /// Example: [100,4,200,1,3,2] -> 4 (the run is 1,2,3,4).
    /// Walkthrough: only extend from values with no predecessor; each run
    /// is therefore scanned once from its smallest member.
    ///
    /// Time: O(n) average; O(n^2) worst case with hash collisions.
    /// Extra space: O(n).
    [[nodiscard]] static std::size_t longestConsecutiveSequence(
        std::span<const int> numbers)
    {
        std::unordered_set<int> values(numbers.begin(), numbers.end());
        std::size_t longest = 0;

        for (const int number : values) {
            if (number != std::numeric_limits<int>::min() &&
                values.contains(number - 1)) {
                continue;
            }

            std::size_t length = 1;
            int current = number;
            while (current != std::numeric_limits<int>::max() &&
                   values.contains(current + 1)) {
                ++current;
                ++length;
            }
            longest = std::max(longest, length);
        }

        return longest;
    }

    /// Counts contiguous subarrays whose sum equals target.
    ///
    /// Throws std::overflow_error if a running prefix sum cannot fit in
    /// long long. The input is not modified.
    /// Example: [1,1,1], target = 2 -> 2 subarrays (the two length-2 ranges).
    /// Walkthrough: for prefix P, every previous prefix P-target starts one
    /// matching subarray ending at the current position.
    ///
    /// Time: O(n) average; O(n^2) worst case with hash collisions.
    /// Extra space: O(n).
    [[nodiscard]] static std::size_t subarraySumEqualsK(
        std::span<const int> numbers,
        long long target)
    {
        std::unordered_map<long long, std::size_t> prefixFrequencies;
        prefixFrequencies.reserve(numbers.size() + 1);
        prefixFrequencies.emplace(0, 1);

        long long prefix = 0;
        std::size_t count = 0;
        for (const int number : numbers) {
            prefix = addChecked(prefix, number);

            if (!subtractWouldOverflow(prefix, target)) {
                const auto match = prefixFrequencies.find(prefix - target);
                if (match != prefixFrequencies.end()) {
                    count += match->second;
                }
            }
            ++prefixFrequencies[prefix];
        }

        return count;
    }

    /// Returns the maximum contiguous-subarray sum, or nullopt for empty input.
    ///
    /// Throws std::overflow_error if an intermediate sum cannot fit in
    /// long long. The input is not modified.
    /// Example: [-2,1,-3,4,-1,2,1,-5,4] -> 6 from [4,-1,2,1].
    /// Walkthrough: at each element either start a new range or extend the
    /// previous best-ending-here range; retain the best seen overall.
    ///
    /// Time: O(n). Extra space: O(1).
    [[nodiscard]] static std::optional<long long> maximumSubarraySum(
        std::span<const int> numbers)
    {
        if (numbers.empty()) {
            return std::nullopt;
        }

        long long current = numbers.front();
        long long maximum = current;
        for (std::size_t index = 1; index < numbers.size(); ++index) {
            current = std::max(
                static_cast<long long>(numbers[index]),
                addChecked(current, numbers[index]));
            maximum = std::max(maximum, current);
        }

        return maximum;
    }

    /// Returns the element occurring more than half the time, if one exists.
    ///
    /// A verification pass makes this correct even when the input does not
    /// satisfy the usual guaranteed-majority interview constraint. The input
    /// is not modified.
    /// Example: [2,2,1,1,1] -> 1. Candidate votes cancel unlike values;
    /// a second pass verifies that the surviving candidate is truly a majority.
    ///
    /// Time: O(n). Extra space: O(1).
    /// Example: [1,3,4,2,2] -> 2. Treat each value as the next array index;
    /// the duplicate creates a cycle whose entry is the repeated value.
    [[nodiscard]] static std::optional<int> majorityElement(
        std::span<const int> numbers)
    {
        if (numbers.empty()) {
            return std::nullopt;
        }

        int candidate = 0;
        std::size_t balance = 0;
        for (const int number : numbers) {
            if (balance == 0) {
                candidate = number;
                balance = 1;
            } else if (number == candidate) {
                ++balance;
            } else {
                --balance;
            }
        }

        std::size_t occurrences = 0;
        for (const int number : numbers) {
            occurrences += number == candidate;
        }

        if (occurrences > numbers.size() / 2) {
            return candidate;
        }
        return std::nullopt;
    }

    /// Finds the duplicate in n + 1 values, each in the inclusive range [1, n].
    ///
    /// Throws std::invalid_argument when the input violates that precondition.
    /// The input is not modified.
    ///
    /// Time: O(n). Extra space: O(1).
    [[nodiscard]] static int findDuplicateNumber(std::span<const int> numbers)
    {
        const std::size_t size = numbers.size();
        if (size < 2) {
            throw std::invalid_argument("input must contain n + 1 values for n >= 1");
        }

        for (const int number : numbers) {
            if (number < 1 || static_cast<std::size_t>(number) >= size) {
                throw std::invalid_argument("values must be in the range [1, n]");
            }
        }

        std::size_t slow = 0;
        std::size_t fast = 0;
        do {
            slow = static_cast<std::size_t>(numbers[slow]);
            fast = static_cast<std::size_t>(
                numbers[static_cast<std::size_t>(numbers[fast])]);
        } while (slow != fast);

        slow = 0;
        while (slow != fast) {
            slow = static_cast<std::size_t>(numbers[slow]);
            fast = static_cast<std::size_t>(numbers[fast]);
        }

        return static_cast<int>(slow);
    }

    /// Places each positive value at the index matching its value minus one
    /// and returns the smallest missing positive integer.
    ///
    /// Reorders numbers in place. Time: O(n). Extra space: O(1).
    /// Example: [3,4,-1,1] -> 2. Place each in-range positive x at index x-1;
    /// the first index still holding the wrong value identifies the gap.
    [[nodiscard]] static std::size_t firstMissingPositive(
        std::span<int> numbers)
    {
        const std::size_t size = numbers.size();
        for (std::size_t index = 0; index < size; ++index) {
            while (numbers[index] > 0 &&
                   static_cast<std::size_t>(numbers[index]) <= size) {
                const std::size_t target =
                    static_cast<std::size_t>(numbers[index] - 1);
                if (numbers[target] == numbers[index]) {
                    break;
                }
                std::swap(numbers[index], numbers[target]);
            }
        }

        for (std::size_t index = 0; index < size; ++index) {
            if (index >=
                    static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
                numbers[index] != static_cast<int>(index + 1)) {
                return index + 1;
            }
        }
        return size + 1;
    }

    /// Zeros every row and column containing a zero, modifying the matrix in
    /// place. The matrix must be rectangular; ragged input throws
    /// std::invalid_argument without modifying it.
    ///
    /// Time: O(rows * columns). Extra space: O(1).
    /// Example: [[1,1,1],[1,0,1],[1,1,1]] becomes
    /// [[1,0,1],[0,0,0],[1,0,1]]. First row/column serve as marker storage.
    static void setMatrixZeroes(std::span<std::vector<int>> matrix)
    {
        if (matrix.empty()) {
            return;
        }

        const std::size_t columns = matrix.front().size();
        for (const auto& row : matrix) {
            if (row.size() != columns) {
                throw std::invalid_argument("matrix must be rectangular");
            }
        }
        if (columns == 0) {
            return;
        }

        bool zeroFirstRow = false;
        bool zeroFirstColumn = false;
        for (std::size_t column = 0; column < columns; ++column) {
            zeroFirstRow = zeroFirstRow || matrix[0][column] == 0;
        }
        for (const auto& row : matrix) {
            zeroFirstColumn = zeroFirstColumn || row[0] == 0;
        }

        for (std::size_t row = 1; row < matrix.size(); ++row) {
            for (std::size_t column = 1; column < columns; ++column) {
                if (matrix[row][column] == 0) {
                    matrix[row][0] = 0;
                    matrix[0][column] = 0;
                }
            }
        }

        for (std::size_t row = 1; row < matrix.size(); ++row) {
            for (std::size_t column = 1; column < columns; ++column) {
                if (matrix[row][0] == 0 || matrix[0][column] == 0) {
                    matrix[row][column] = 0;
                }
            }
        }

        if (zeroFirstRow) {
            std::fill(matrix[0].begin(), matrix[0].end(), 0);
        }
        if (zeroFirstColumn) {
            for (auto& row : matrix) {
                row[0] = 0;
            }
        }
    }

    /// Rotates an n-by-n matrix 90 degrees clockwise, modifying it in place.
    /// Throws std::invalid_argument for a non-square matrix without modifying it.
    ///
    /// Time: O(n^2). Extra space: O(1).
    /// Example: [[1,2],[3,4]] becomes [[3,1],[4,2]]. Transpose, then reverse
    /// every row to turn the diagonal reflection into a clockwise rotation.
    static void rotateImage(std::span<std::vector<int>> matrix)
    {
        const std::size_t size = matrix.size();
        for (const auto& row : matrix) {
            if (row.size() != size) {
                throw std::invalid_argument("matrix must be square");
            }
        }

        for (std::size_t row = 0; row < size; ++row) {
            for (std::size_t column = row + 1; column < size; ++column) {
                std::swap(matrix[row][column], matrix[column][row]);
            }
        }
        for (auto& row : matrix) {
            std::ranges::reverse(row);
        }
    }

    /// Reports whether any value occurs more than once.
    ///
    /// The input is not modified. An empty or single-element range has no
    /// duplicates.
    ///
    /// Time: O(n) average; O(n^2) worst case with hash collisions.
    /// Extra space: O(n).
    /// Example: [1,2,3,1] -> true. Inserting an already-seen value proves a
    /// duplicate exists, so the scan can stop immediately.
    [[nodiscard]] static bool containsDuplicate(std::span<const int> numbers)
    {
        std::unordered_set<int> seen;
        seen.reserve(numbers.size());

        for (const int number : numbers) {
            if (!seen.insert(number).second) {
                return true;
            }
        }

        return false;
    }

    /// Checks whether two byte strings contain the same characters with the
    /// same frequencies. UTF-8 input is compared as bytes, not Unicode code points.
    ///
    /// The input strings are not modified.
    ///
    /// Time: O(n + m), where n and m are the input byte lengths.
    /// Extra space: O(1), using a fixed 256-entry frequency table.
    /// Example: "listen", "silent" -> true. Increment byte counts for the
    /// first input and decrement them for the second.
    [[nodiscard]] static bool isAnagram(
        std::string_view first,
        std::string_view second) noexcept
    {
        if (first.size() != second.size()) {
            return false;
        }

        std::array<std::size_t, 256> frequencies{};

        for (const char character : first) {
            ++frequencies[static_cast<unsigned char>(character)];
        }

        for (const char character : second) {
            const auto index = static_cast<unsigned char>(character);
            if (frequencies[index] == 0) {
                return false;
            }
            --frequencies[index];
        }

        return true;
    }

private:
    static long long addChecked(long long first, long long second)
    {
        if ((second > 0 &&
             first > std::numeric_limits<long long>::max() - second) ||
            (second < 0 &&
             first < std::numeric_limits<long long>::min() - second)) {
            throw std::overflow_error("sum does not fit in long long");
        }
        return first + second;
    }

    static long long multiplyChecked(long long first, long long second)
    {
        if (first == 0 || second == 0) {
            return 0;
        }
        if ((first == -1 && second == std::numeric_limits<long long>::min()) ||
            (second == -1 && first == std::numeric_limits<long long>::min()) ||
            (first > 0 && second > 0 &&
             first > std::numeric_limits<long long>::max() / second) ||
            (first > 0 && second < 0 &&
             second < std::numeric_limits<long long>::min() / first) ||
            (first < 0 && second > 0 &&
             first < std::numeric_limits<long long>::min() / second) ||
            (first < 0 && second < 0 &&
             first < std::numeric_limits<long long>::max() / second)) {
            throw std::overflow_error("product does not fit in long long");
        }
        return first * second;
    }

    static bool subtractWouldOverflow(long long first, long long second) noexcept
    {
        return (second > 0 &&
                first < std::numeric_limits<long long>::min() + second) ||
               (second < 0 &&
                first > std::numeric_limits<long long>::max() + second);
    }
};

} // namespace dsa::arrays_hashing
