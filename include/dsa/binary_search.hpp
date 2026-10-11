#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace dsa::binary_search {

/// Static solutions for binary-search and answer-space interview problems
/// (checklist 34-47).
///
/// Sortedness preconditions on large inputs are documented rather than
/// verified, because verifying them in O(n) would defeat the O(log n) bound.
/// Violating such a documented precondition gives an unspecified (but memory
/// safe) result. Cheap-to-check value constraints throw std::invalid_argument.
class BinarySearch final {
public:
    BinarySearch() = delete;

    /// Problem 34 - Binary Search.
    ///
    /// Precondition: sorted ascending. Returns the index of the leftmost
    /// element equal to target, or std::nullopt if absent.
    ///
    /// Technique: lower-bound search keeping the invariant that every element
    /// before lo is < target and every element from hi on is >= target.
    /// Time: O(log n). Extra space: O(1).
    /// Example: sorted = [-1,0,3,5,9,12], target = 9 -> index 4. Lower-bound
    /// search narrows to the first candidate, then equality confirms a match.
    [[nodiscard]] static std::optional<std::size_t> binarySearch(
        std::span<const int> sorted, int target) noexcept
    {
        const std::size_t position = lowerBound(sorted, target);
        if (position < sorted.size() && sorted[position] == target) {
            return position;
        }
        return std::nullopt;
    }

    /// Problem 35 - Search Insert Position.
    ///
    /// Precondition: sorted ascending (duplicates allowed). Returns the first
    /// index at which target could be inserted keeping the order, i.e. the
    /// index of the first element >= target, or sorted.size().
    ///
    /// Time: O(log n). Extra space: O(1).
    /// Example: sorted = [1,3,5,6], target = 2 -> index 1, the insertion point.
    [[nodiscard]] static std::size_t searchInsertPosition(
        std::span<const int> sorted, int target) noexcept
    {
        return lowerBound(sorted, target);
    }

    /// Problem 36 - Find First and Last Position of Element in Sorted Array.
    ///
    /// Precondition: sorted ascending. Returns the inclusive {first, last}
    /// index range of target, or std::nullopt if absent.
    ///
    /// Technique: lower bound of target and upper bound of target.
    /// Time: O(log n). Extra space: O(1).
    /// Example: [5,7,7,8,8,10], target = 8 -> [3,4]. Two boundary searches
    /// locate the first equal element and the first element greater than it.
    [[nodiscard]] static std::optional<std::pair<std::size_t, std::size_t>>
    searchRange(std::span<const int> sorted, int target) noexcept
    {
        const std::size_t first = lowerBound(sorted, target);
        if (first == sorted.size() || sorted[first] != target) {
            return std::nullopt;
        }
        const std::size_t end = upperBound(sorted, target);
        return std::pair{first, end - 1};
    }

    /// Problem 37 - Search in Rotated Sorted Array (distinct values).
    ///
    /// Precondition: a strictly ascending array rotated at an unknown pivot
    /// (possibly by zero). Returns the index of target or std::nullopt.
    /// With duplicates use searchRotatedWithDuplicates instead.
    ///
    /// Technique: at each step one half of [lo, hi] is sorted; test whether
    /// target lies inside that half and discard the other.
    /// Time: O(log n). Extra space: O(1).
    /// Example: rotated = [4,5,6,7,0,1,2], target = 0 -> index 4. Each
    /// iteration identifies which half is sorted and discards the other half.
    [[nodiscard]] static std::optional<std::size_t> searchRotated(
        std::span<const int> rotated, int target) noexcept
    {
        std::ptrdiff_t lo = 0;
        std::ptrdiff_t hi = static_cast<std::ptrdiff_t>(rotated.size()) - 1;
        while (lo <= hi) {
            const std::ptrdiff_t mid = lo + (hi - lo) / 2;
            if (at(rotated, mid) == target) {
                return static_cast<std::size_t>(mid);
            }
            if (at(rotated, lo) <= at(rotated, mid)) {
                if (at(rotated, lo) <= target && target < at(rotated, mid)) {
                    hi = mid - 1;
                } else {
                    lo = mid + 1;
                }
            } else {
                if (at(rotated, mid) < target && target <= at(rotated, hi)) {
                    lo = mid + 1;
                } else {
                    hi = mid - 1;
                }
            }
        }
        return std::nullopt;
    }

    /// Problem 38 - Search in Rotated Sorted Array II (duplicates allowed).
    ///
    /// Precondition: a non-decreasing array rotated at an unknown pivot.
    /// Returns the index of SOME element equal to target (not necessarily the
    /// first), or std::nullopt.
    ///
    /// Technique: as searchRotated, but when lo, mid and hi hold equal values
    /// the sorted half is ambiguous, so both ends (known != target) are
    /// trimmed by one.
    /// Time: O(log n) typical, O(n) worst case (e.g. all equal values).
    /// Extra space: O(1).
    /// Example: rotated = [2,5,6,0,0,1,2], target = 0 -> an index containing
    /// 0. Equal boundaries may force a one-element trim, so worst case is O(n).
    [[nodiscard]] static std::optional<std::size_t> searchRotatedWithDuplicates(
        std::span<const int> rotated, int target) noexcept
    {
        std::ptrdiff_t lo = 0;
        std::ptrdiff_t hi = static_cast<std::ptrdiff_t>(rotated.size()) - 1;
        while (lo <= hi) {
            const std::ptrdiff_t mid = lo + (hi - lo) / 2;
            if (at(rotated, mid) == target) {
                return static_cast<std::size_t>(mid);
            }
            if (at(rotated, lo) == at(rotated, mid) &&
                at(rotated, mid) == at(rotated, hi)) {
                ++lo;
                --hi;
            } else if (at(rotated, lo) <= at(rotated, mid)) {
                if (at(rotated, lo) <= target && target < at(rotated, mid)) {
                    hi = mid - 1;
                } else {
                    lo = mid + 1;
                }
            } else {
                if (at(rotated, mid) < target && target <= at(rotated, hi)) {
                    lo = mid + 1;
                } else {
                    hi = mid - 1;
                }
            }
        }
        return std::nullopt;
    }

    /// Problem 39 - Find Minimum in Rotated Sorted Array (distinct values).
    ///
    /// Precondition: strictly ascending array rotated at an unknown pivot.
    /// Returns the minimum, or std::nullopt for an empty span.
    ///
    /// Technique: compare mid with the right end; the minimum is in the left
    /// half (including mid) iff a[mid] < a[hi].
    /// Time: O(log n). Extra space: O(1).
    /// Example: [3,4,5,1,2] -> 1. Compare the midpoint to the right endpoint
    /// to retain the half that must contain the rotation minimum.
    [[nodiscard]] static std::optional<int> findMinRotated(
        std::span<const int> rotated) noexcept
    {
        if (rotated.empty()) {
            return std::nullopt;
        }
        std::size_t lo = 0;
        std::size_t hi = rotated.size() - 1;
        while (lo < hi) {
            const std::size_t mid = lo + (hi - lo) / 2;
            if (rotated[mid] < rotated[hi]) {
                hi = mid;
            } else {
                lo = mid + 1;
            }
        }
        return rotated[lo];
    }

    /// Problem 40 - Find Minimum in Rotated Sorted Array II (duplicates).
    ///
    /// Precondition: non-decreasing array rotated at an unknown pivot.
    /// Returns the minimum, or std::nullopt for an empty span.
    ///
    /// Technique: as findMinRotated; when a[mid] == a[hi] the side is
    /// ambiguous, but dropping hi is safe because a[mid] still holds the same
    /// value.
    /// Time: O(log n) typical, O(n) worst case. Extra space: O(1).
    /// Example: [2,2,2,0,1] -> 0. Equal midpoint and right values are
    /// ambiguous, but discarding the rightmost duplicate preserves a minimum.
    [[nodiscard]] static std::optional<int> findMinRotatedWithDuplicates(
        std::span<const int> rotated) noexcept
    {
        if (rotated.empty()) {
            return std::nullopt;
        }
        std::size_t lo = 0;
        std::size_t hi = rotated.size() - 1;
        while (lo < hi) {
            const std::size_t mid = lo + (hi - lo) / 2;
            if (rotated[mid] < rotated[hi]) {
                hi = mid;
            } else if (rotated[mid] > rotated[hi]) {
                lo = mid + 1;
            } else {
                --hi;
            }
        }
        return rotated[lo];
    }

    /// Problem 41 - Find Peak Element.
    ///
    /// Returns the index of any peak: an element strictly greater than its
    /// existing neighbours (out-of-range neighbours count as -infinity).
    /// Returns std::nullopt for an empty span. Precondition: adjacent
    /// elements differ (not verified; with equal neighbours the result may
    /// not be a peak).
    ///
    /// Technique: if a[mid] < a[mid+1] a peak exists to the right, otherwise
    /// at mid or to the left; halve toward the rising side.
    /// Time: O(log n). Extra space: O(1).
    /// Example: [1,2,3,1] -> index 2 (value 3). A rising midpoint-to-next
    /// slope guarantees a peak on the right; otherwise one remains on left.
    [[nodiscard]] static std::optional<std::size_t> findPeakElement(
        std::span<const int> values) noexcept
    {
        if (values.empty()) {
            return std::nullopt;
        }
        std::size_t lo = 0;
        std::size_t hi = values.size() - 1;
        while (lo < hi) {
            const std::size_t mid = lo + (hi - lo) / 2;
            if (values[mid] < values[mid + 1]) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    /// Extra - Sqrt(x) (integer square root).
    ///
    /// Returns floor(sqrt(x)) for x >= 0; throws std::invalid_argument for
    /// negative x. Squares are formed only for candidates <= 3037000499, so
    /// long long never overflows, including for the maximum long long.
    ///
    /// Technique: binary search on the answer for the largest m with m*m <= x.
    /// Time: O(log x). Extra space: O(1).
    /// Example: x = 8 -> 2. Search for the largest integer m whose square
    /// does not exceed x, keeping multiplication within the safe root bound.
    [[nodiscard]] static long long integerSquareRoot(long long x)
    {
        if (x < 0) {
            throw std::invalid_argument("integerSquareRoot: x must be non-negative");
        }
        constexpr long long kMaxRoot = 3037000499LL;  // floor(sqrt(2^63 - 1))
        long long lo = 0;
        long long hi = std::min(x, kMaxRoot);
        while (lo < hi) {
            const long long mid = lo + (hi - lo + 1) / 2;
            if (mid * mid <= x) {
                lo = mid;
            } else {
                hi = mid - 1;
            }
        }
        return lo;
    }

    /// Problem 43 - Koko Eating Bananas.
    ///
    /// Each hour Koko picks one pile and eats up to `speed` bananas from it
    /// (finishing a pile early wastes the rest of that hour). Returns the
    /// minimum integer speed >= 1 that finishes all piles within `hours`, or
    /// std::nullopt if impossible (hours is fewer than the number of
    /// non-empty piles). Piles must be non-negative (std::invalid_argument
    /// otherwise). With no bananas the answer is 1.
    ///
    /// Technique: binary search over speed in [1, max pile]; feasibility is
    /// monotone in speed. Hours are accumulated in long long.
    /// Time: O(n log M), M = max pile. Extra space: O(1).
    /// Example: piles = [3,6,7,11], hours = 8 -> speed 4. For each candidate
    /// speed, sum required hours; feasibility is monotone as speed increases.
    [[nodiscard]] static std::optional<long long> minEatingSpeed(
        std::span<const int> piles, long long hours)
    {
        long long maxPile = 1;
        long long nonEmpty = 0;
        for (const int pile : piles) {
            if (pile < 0) {
                throw std::invalid_argument("minEatingSpeed: piles must be non-negative");
            }
            maxPile = std::max<long long>(maxPile, pile);
            nonEmpty += (pile > 0);
        }
        if (hours < nonEmpty) {
            return std::nullopt;
        }
        long long lo = 1;
        long long hi = maxPile;  // always feasible: one hour per non-empty pile
        while (lo < hi) {
            const long long mid = lo + (hi - lo) / 2;
            long long needed = 0;
            for (const int pile : piles) {
                needed += (static_cast<long long>(pile) + mid - 1) / mid;
            }
            if (needed <= hours) {
                hi = mid;
            } else {
                lo = mid + 1;
            }
        }
        return lo;
    }

    /// Problem 44 - Capacity To Ship Packages Within D Days.
    ///
    /// Packages must ship in the given order, each day carrying a contiguous
    /// group whose total weight is at most the capacity. Returns the minimum
    /// capacity that needs at most `days` days. Weights must be non-negative
    /// (std::invalid_argument otherwise). No weights gives 0. If weights
    /// exist but days < 1, shipping is infeasible and std::nullopt is
    /// returned.
    ///
    /// Technique: binary search capacity in [max weight, total weight].
    /// Time: O(n log S), S = total weight. Extra space: O(1).
    /// Example: weights = [1,2,3,4,5,6,7,8,9,10], days = 5 -> capacity 15.
    /// Greedily count days at a candidate capacity and binary-search the
    /// smallest capacity that fits.
    [[nodiscard]] static std::optional<long long> shipWithinDays(
        std::span<const int> weights, long long days)
    {
        if (weights.empty()) {
            return 0;
        }
        const auto [lo, hi] = capacityBounds(weights, "shipWithinDays");
        if (days < 1) {
            return std::nullopt;
        }
        return smallestCapacity(weights, days, lo, hi);
    }

    /// Problem 45 - Split Array Largest Sum.
    ///
    /// Splits numbers into exactly m non-empty contiguous parts minimizing the
    /// largest part sum, and returns that sum. Elements must be non-negative
    /// (std::invalid_argument otherwise). Returns std::nullopt when no split
    /// exists: m == 0, m > size, or empty input.
    ///
    /// Technique: binary search the largest-sum bound in [max, total];
    /// greedy part counting is monotone. For non-negative data, "at most m
    /// parts" and "exactly m parts" coincide because parts can be split
    /// further without raising the maximum when m <= size.
    /// Time: O(n log S), S = total sum. Extra space: O(1).
    /// Example: numbers = [7,2,5,10,8], m = 2 -> 18. The greedy scan counts
    /// required parts for each largest-sum limit; fewer parts means try lower.
    [[nodiscard]] static std::optional<long long> splitArrayLargestSum(
        std::span<const int> numbers, std::size_t m)
    {
        if (numbers.empty() || m == 0 || m > numbers.size()) {
            for (const int value : numbers) {
                if (value < 0) {
                    throw std::invalid_argument(
                        "splitArrayLargestSum: elements must be non-negative");
                }
            }
            return std::nullopt;
        }
        const auto [lo, hi] = capacityBounds(numbers, "splitArrayLargestSum");
        return smallestCapacity(numbers, static_cast<long long>(m), lo, hi);
    }

    /// Problem 46 - Minimum Number of Days to Make m Bouquets.
    ///
    /// bloomDay[i] is the day flower i blooms (any int). A bouquet needs k
    /// ADJACENT bloomed flowers, and each flower is used at most once.
    /// Returns the smallest day by which m bouquets can be made, or
    /// std::nullopt if impossible (m * k > bloomDay.size(), tested without
    /// overflow). m and k must be positive, otherwise std::invalid_argument
    /// is thrown. Input is not modified.
    ///
    /// Technique: binary search the day over [min, max] of bloomDay;
    /// feasibility (greedy count of disjoint runs of k bloomed flowers) is
    /// monotone in the day, and the first feasible day is always a bloomDay.
    /// Time: O(n log(max - min)). Extra space: O(1).
    /// Example: bloomDay = [1,10,3,10,2], m = 3, k = 1 -> day 3. At each
    /// candidate day, count disjoint runs of k flowers that have bloomed.
    [[nodiscard]] static std::optional<int> minDaysToMakeBouquets(
        std::span<const int> bloomDay, long long m, long long k)
    {
        if (m <= 0 || k <= 0) {
            throw std::invalid_argument(
                "minDaysToMakeBouquets: m and k must be positive");
        }
        const auto size = static_cast<long long>(bloomDay.size());
        if (m > size / k) {
            return std::nullopt;
        }
        const auto [lowest, highest] = std::ranges::minmax(bloomDay);
        long long lo = lowest;
        long long hi = highest;  // feasible: every flower has bloomed
        while (lo < hi) {
            const long long mid = lo + (hi - lo) / 2;
            long long bouquets = 0;
            long long run = 0;
            for (const int day : bloomDay) {
                if (day <= mid) {
                    if (++run == k) {
                        ++bouquets;
                        run = 0;
                    }
                } else {
                    run = 0;
                }
            }
            if (bouquets >= m) {
                hi = mid;
            } else {
                lo = mid + 1;
            }
        }
        return static_cast<int>(lo);
    }

    /// Problem 47 - Magnetic Force Between Two Balls.
    ///
    /// Places `balls` balls into distinct baskets at the given positions
    /// (any order; equal positions are distinct baskets at distance 0) to
    /// maximize the minimum distance between any two balls. Returns that
    /// maximum minimum distance. balls must be at least 2
    /// (std::invalid_argument otherwise). Returns std::nullopt if there are
    /// fewer baskets than balls. Distances use long long, so extreme ints
    /// cannot overflow. Input is not modified.
    ///
    /// Technique: sort a copy, binary search the distance d in
    /// [0, max - min]; greedily placing each ball at the first basket at
    /// least d past the previous one is monotone in d.
    /// Time: O(n log n + n log(max - min)). Extra space: O(n) for the copy.
    /// Example: positions = [1,2,3,4,7], balls = 3 -> distance 3. For each
    /// candidate gap, greedily place balls in the earliest compatible baskets.
    [[nodiscard]] static std::optional<long long> maxMinMagneticForce(
        std::span<const int> positions, std::size_t balls)
    {
        if (balls < 2) {
            throw std::invalid_argument("maxMinMagneticForce: balls must be at least 2");
        }
        if (balls > positions.size()) {
            return std::nullopt;
        }
        std::vector<int> sorted(positions.begin(), positions.end());
        std::ranges::sort(sorted);

        long long lo = 0;  // always feasible while balls <= baskets
        long long hi = static_cast<long long>(sorted.back()) - sorted.front();
        while (lo < hi) {
            const long long mid = lo + (hi - lo + 1) / 2;
            std::size_t placed = 1;
            long long last = sorted.front();
            for (const int position : sorted) {
                if (position - last >= mid) {
                    ++placed;
                    last = position;
                }
            }
            if (placed >= balls) {
                lo = mid;
            } else {
                hi = mid - 1;
            }
        }
        return lo;
    }
    /// Extra - Median of Two Sorted Arrays.
    ///
    /// Precondition: both spans sorted ascending (either may be empty).
    /// Returns the median of the combined multiset as a double (average of the
    /// two middle values when the total count is even; computed in double so
    /// extreme ints cannot overflow), or std::nullopt if both are empty.
    ///
    /// Technique: binary search the partition of the smaller array so that
    /// the left halves hold half of all elements and every left element is
    /// <= every right element; empty sides use +/- infinity sentinels.
    /// Time: O(log min(n, m)). Extra space: O(1).
    /// Example: [1,3] and [2] -> 2. Binary-search a partition of the shorter
    /// array so the left-side maximum does not exceed the right-side minimum.
    [[nodiscard]] static std::optional<double> findMedianSortedArrays(
        std::span<const int> first, std::span<const int> second) noexcept
    {
        if (first.empty() && second.empty()) {
            return std::nullopt;
        }
        if (first.size() > second.size()) {
            std::swap(first, second);
        }
        constexpr long long kNegInf = std::numeric_limits<long long>::min();
        constexpr long long kPosInf = std::numeric_limits<long long>::max();

        const std::ptrdiff_t small = static_cast<std::ptrdiff_t>(first.size());
        const std::ptrdiff_t total = small + static_cast<std::ptrdiff_t>(second.size());
        const std::ptrdiff_t leftCount = (total + 1) / 2;
        std::ptrdiff_t lo = 0;
        std::ptrdiff_t hi = small;
        while (lo <= hi) {
            const std::ptrdiff_t i = lo + (hi - lo) / 2;  // taken from first
            const std::ptrdiff_t j = leftCount - i;       // taken from second
            const long long firstLeft = i > 0 ? first[static_cast<std::size_t>(i - 1)] : kNegInf;
            const long long firstRight = i < small ? first[static_cast<std::size_t>(i)] : kPosInf;
            const long long secondLeft = j > 0 ? second[static_cast<std::size_t>(j - 1)] : kNegInf;
            const long long secondRight =
                j < static_cast<std::ptrdiff_t>(second.size())
                    ? second[static_cast<std::size_t>(j)]
                    : kPosInf;
            if (firstLeft > secondRight) {
                hi = i - 1;
            } else if (secondLeft > firstRight) {
                lo = i + 1;
            } else {
                const long long leftMax = std::max(firstLeft, secondLeft);
                if (total % 2 == 1) {
                    return static_cast<double>(leftMax);
                }
                const long long rightMin = std::min(firstRight, secondRight);
                return (static_cast<double>(leftMax) + static_cast<double>(rightMin)) / 2.0;
            }
        }
        return std::nullopt;  // unreachable for sorted inputs
    }

    /// Extra - Search a 2D Matrix.
    ///
    /// matrix is row-major with each row sorted ascending and the first value
    /// of each row greater than the last value of the previous row, so the
    /// matrix read row by row is one sorted sequence. Rows may number a
    /// different count than columns (rectangular, non-square). Throws
    /// std::invalid_argument if rows have different lengths. Returns the
    /// {row, column} of target, or std::nullopt if absent or the matrix has
    /// no rows/columns.
    ///
    /// Technique: binary search over the flattened index, mapping
    /// k -> (k / columns, k % columns).
    /// Time: O(log(rows * columns)) after the O(rows) shape check.
    /// Extra space: O(1).
    /// Example: [[1,3,5],[7,9,11]], target = 9 -> [1,1]. Treat the matrix as
    /// a sorted flat sequence and map each found index back to row/column.
    [[nodiscard]] static std::optional<std::pair<std::size_t, std::size_t>>
    searchMatrix(std::span<const std::vector<int>> matrix, int target)
    {
        if (matrix.empty()) {
            return std::nullopt;
        }
        const std::size_t columns = matrix.front().size();
        for (const auto& row : matrix) {
            if (row.size() != columns) {
                throw std::invalid_argument("searchMatrix: matrix must be rectangular");
            }
        }
        if (columns == 0) {
            return std::nullopt;
        }
        std::size_t lo = 0;
        std::size_t hi = matrix.size() * columns;  // exclusive
        while (lo < hi) {
            const std::size_t mid = lo + (hi - lo) / 2;
            const int value = matrix[mid / columns][mid % columns];
            if (value == target) {
                return std::pair{mid / columns, mid % columns};
            }
            if (value < target) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return std::nullopt;
    }

private:
    [[nodiscard]] static int at(std::span<const int> values, std::ptrdiff_t index) noexcept
    {
        return values[static_cast<std::size_t>(index)];
    }

    // First index whose element is >= target.
    [[nodiscard]] static std::size_t lowerBound(
        std::span<const int> sorted, int target) noexcept
    {
        std::size_t lo = 0;
        std::size_t hi = sorted.size();
        while (lo < hi) {
            const std::size_t mid = lo + (hi - lo) / 2;
            if (sorted[mid] < target) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    // First index whose element is > target.
    [[nodiscard]] static std::size_t upperBound(
        std::span<const int> sorted, int target) noexcept
    {
        std::size_t lo = 0;
        std::size_t hi = sorted.size();
        while (lo < hi) {
            const std::size_t mid = lo + (hi - lo) / 2;
            if (sorted[mid] <= target) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    // Validates non-negativity; returns {max element, total sum}.
    [[nodiscard]] static std::pair<long long, long long> capacityBounds(
        std::span<const int> values, const char* caller)
    {
        long long largest = 0;
        long long total = 0;
        for (const int value : values) {
            if (value < 0) {
                throw std::invalid_argument(
                    std::string(caller) + ": elements must be non-negative");
            }
            largest = std::max<long long>(largest, value);
            total += value;
        }
        return {largest, total};
    }

    // Smallest capacity in [lo, hi] for which greedy grouping needs at most
    // maxGroups groups; hi (the total) always suffices when maxGroups >= 1.
    [[nodiscard]] static long long smallestCapacity(
        std::span<const int> values, long long maxGroups, long long lo, long long hi)
    {
        while (lo < hi) {
            const long long mid = lo + (hi - lo) / 2;
            long long groups = 1;
            long long current = 0;
            for (const int value : values) {
                if (current + value > mid) {
                    ++groups;
                    current = 0;
                }
                current += value;
            }
            if (groups <= maxGroups) {
                hi = mid;
            } else {
                lo = mid + 1;
            }
        }
        return lo;
    }
};

}  // namespace dsa::binary_search
