#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dsa::heaps {

/// Integer point in a 2D Cartesian plane.
struct Point {
    int x;
    int y;

    friend bool operator==(const Point&, const Point&) = default;
};

/// Static solutions for heap and priority-queue interview problems.
class Heaps final {
public:
    Heaps() = delete;

    /// Maintains a min-heap of the k largest values and returns its minimum,
    /// counting duplicates as separate entries.
    /// k is one-based and must be in [1, numbers.size()], otherwise throws
    /// invalid_argument. Input is unchanged. Time: O(n log k); auxiliary space:
    /// O(k).
    /// Example: [3,2,1,5,6,4], k = 2 -> 5. Keep only the k largest values;
    /// the min-heap root is the smallest among those retained.
    [[nodiscard]] static int kthLargest(
        std::span<const int> numbers, std::size_t k)
    {
        if (k == 0 || k > numbers.size()) {
            throw std::invalid_argument("k must be between 1 and input size");
        }
        std::priority_queue<int, std::vector<int>, std::greater<>> smallest;
        for (int number : numbers) {
            smallest.push(number);
            if (smallest.size() > k) {
                smallest.pop();
            }
        }
        return smallest.top();
    }

    /// Maintains the k largest values in a min-heap while simulating additions,
    /// returning one optional
    /// answer per added value. Initial/additional duplicates count separately;
    /// k must be positive or invalid_argument is thrown. A result is nullopt
    /// until at least k values have been seen. Time: O((n+m)log k);
    /// auxiliary space: O(k+m) including returned answers.
    /// Example: initial = [4,5,8,2], additions = [3,5], k = 3 -> [4,5].
    /// Each answer is the root of the heap holding the largest k values seen.
    [[nodiscard]] static std::vector<std::optional<int>> kthLargestInStream(
        std::span<const int> initialValues,
        std::span<const int> additions,
        std::size_t k)
    {
        if (k == 0) {
            throw std::invalid_argument("k must be positive");
        }
        std::priority_queue<int, std::vector<int>, std::greater<>> smallest;
        for (int number : initialValues) {
            smallest.push(number);
            if (smallest.size() > k) {
                smallest.pop();
            }
        }

        std::vector<std::optional<int>> results;
        results.reserve(additions.size());
        for (int number : additions) {
            smallest.push(number);
            if (smallest.size() > k) {
                smallest.pop();
            }
            if (smallest.size() == k) {
                results.emplace_back(smallest.top());
            } else {
                results.emplace_back(std::nullopt);
            }
        }
        return results;
    }

    /// Repeatedly removes the two maxima from a max-heap and reinserts their
    /// difference. Returns the
    /// remaining weight or zero when none remain; negative weights throw.
    /// Time: O(n log n); auxiliary space: O(n).
    /// Example: [2,7,4,1,8,1] -> 1. Repeatedly remove the two largest stones,
    /// push their positive difference, and stop when at most one remains.
    [[nodiscard]] static int lastStoneWeight(std::span<const int> stones)
    {
        std::priority_queue<int> heaviest;
        for (int stone : stones) {
            if (stone < 0) {
                throw std::invalid_argument("stone weights cannot be negative");
            }
            if (stone > 0) {
                heaviest.push(stone);
            }
        }
        while (heaviest.size() > 1) {
            const int first = heaviest.top();
            heaviest.pop();
            const int second = heaviest.top();
            heaviest.pop();
            if (first != second) {
                heaviest.push(first - second);
            }
        }
        return heaviest.empty() ? 0 : heaviest.top();
    }

    /// Selects k points by squared Euclidean distance, ordered by increasing
    /// distance
    /// distance then by x and y for deterministic ties. k may be zero but may
    /// not exceed the point count. Time: O(n log k); auxiliary space: O(k).
    /// Example: points = [[1,3],[-2,2]], k = 1 -> [[-2,2]]. Compare squared
    /// distances and retain only the k nearest points in a bounded heap.
    [[nodiscard]] static std::vector<Point> kClosestPoints(
        std::span<const Point> points, std::size_t k)
    {
        if (k > points.size()) {
            throw std::invalid_argument("k exceeds the point count");
        }
        const auto distance = [](const Point& point) {
            const long long x = point.x;
            const long long y = point.y;
            const auto magnitude = [](long long value) {
                return static_cast<unsigned long long>(
                    value < 0 ? -value : value);
            };
            const auto absoluteX = magnitude(x);
            const auto absoluteY = magnitude(y);
            return absoluteX * absoluteX + absoluteY * absoluteY;
        };
        const auto less = [&](const Point& first, const Point& second) {
            if (distance(first) != distance(second)) {
                return distance(first) < distance(second);
            }
            if (first.x != second.x) {
                return first.x < second.x;
            }
            return first.y < second.y;
        };
        std::priority_queue<Point, std::vector<Point>, decltype(less)> nearest(less);
        for (const Point& point : points) {
            nearest.push(point);
            if (nearest.size() > k) {
                nearest.pop();
            }
        }
        std::vector<Point> result;
        result.reserve(k);
        while (!nearest.empty()) {
            result.push_back(nearest.top());
            nearest.pop();
        }
        std::ranges::reverse(result);
        return result;
    }

    /// Merges k sorted integer lists into one sorted list. Every input list
    /// must be nondecreasing or invalid_argument is thrown; empty lists are
    /// allowed. Time: O(N log(k+1)); auxiliary space: O(k) beyond the N-value
    /// returned vector, where N is total input size.
    /// Example: [[1,4,5],[1,3,4],[2,6]] -> [1,1,2,3,4,4,5,6]. Keep one
    /// current head per list; after removing the smallest, advance that list.
    [[nodiscard]] static std::vector<int> mergeKSortedLists(
        std::span<const std::vector<int>> lists)
    {
        struct Cursor {
            int value;
            std::size_t list;
            std::size_t offset;
        };
        struct GreaterCursor {
            bool operator()(const Cursor& first, const Cursor& second) const
            {
                if (first.value != second.value) {
                    return first.value > second.value;
                }
                if (first.list != second.list) {
                    return first.list > second.list;
                }
                return first.offset > second.offset;
            }
        };

        std::size_t total = 0;
        for (const auto& list : lists) {
            if (!std::ranges::is_sorted(list)) {
                throw std::invalid_argument("each input list must be sorted");
            }
            total += list.size();
        }
        std::priority_queue<Cursor, std::vector<Cursor>, GreaterCursor> pending;
        for (std::size_t list = 0; list < lists.size(); ++list) {
            if (!lists[list].empty()) {
                pending.push({lists[list][0], list, 0});
            }
        }
        std::vector<int> result;
        result.reserve(total);
        while (!pending.empty()) {
            const Cursor cursor = pending.top();
            pending.pop();
            result.push_back(cursor.value);
            const std::size_t nextOffset = cursor.offset + 1;
            if (nextOffset < lists[cursor.list].size()) {
                pending.push(
                    {lists[cursor.list][nextOffset], cursor.list, nextOffset});
            }
        }
        return result;
    }

    /// Uses task frequencies and the maximum-frequency idle-frame formula to
    /// return the least number of time slots required to execute unit-time
    /// tasks, with identical tasks separated by at least cooldown idle slots.
    /// Empty tasks require zero slots; any char is a distinct task identity.
    /// Time: O(n + A log A), A is distinct task count; auxiliary space: O(A).
    /// Example: tasks = [A,A,A,B,B,B], cooldown = 2 -> 8 intervals. The
    /// most frequent tasks set a frame lower bound; compare it with task count.
    [[nodiscard]] static std::size_t leastTaskIntervals(
        std::span<const char> tasks, std::size_t cooldown)
    {
        if (tasks.empty()) {
            return 0;
        }
        std::vector<std::size_t> frequencies(256, 0);
        for (char task : tasks) {
            ++frequencies[static_cast<unsigned char>(task)];
        }
        std::ranges::sort(frequencies, std::greater<>());
        const std::size_t maximum = frequencies.front();
        std::size_t maximumKinds = 0;
        for (std::size_t frequency : frequencies) {
            maximumKinds += frequency == maximum;
        }
        const std::size_t largest = std::numeric_limits<std::size_t>::max();
        if (maximum > 1 &&
            (cooldown == largest ||
             (maximum - 1) > (largest - maximumKinds) / (cooldown + 1))) {
            throw std::overflow_error("task schedule length exceeds size_t");
        }
        const std::size_t frame =
            (maximum - 1) * (cooldown + 1) + maximumKinds;
        return std::max(tasks.size(), frame);
    }

    /// Reorders the input bytes so no two adjacent bytes match, returning
    /// nullopt if that is impossible. Empty input returns an engaged empty
    /// string. Bytes, including non-ASCII values, are treated as distinct
    /// task identities. Ties are deterministic by unsigned-byte value.
    /// Time: O(n log(A+1)); auxiliary space: O(A+n) including the result.
    /// Example: "aab" -> a valid result is "aba". Place the most frequent
    /// available byte, holding it out until a different byte is placed.
    [[nodiscard]] static std::optional<std::string> reorganizeString(
        std::string_view input)
    {
        struct CharacterCount {
            std::size_t count;
            unsigned char character;
        };
        struct LowerPriority {
            bool operator()(const CharacterCount& first,
                            const CharacterCount& second) const
            {
                if (first.count != second.count) {
                    return first.count < second.count;
                }
                return first.character > second.character;
            }
        };

        std::array<std::size_t, 256> frequencies{};
        for (char value : input) {
            ++frequencies[static_cast<unsigned char>(value)];
        }
        std::priority_queue<CharacterCount, std::vector<CharacterCount>,
                            LowerPriority>
            available;
        for (std::size_t value = 0; value < frequencies.size(); ++value) {
            if (frequencies[value] > 0) {
                available.push(
                    {frequencies[value], static_cast<unsigned char>(value)});
            }
        }

        std::string result;
        result.reserve(input.size());
        std::optional<CharacterCount> held;
        while (!available.empty()) {
            CharacterCount current = available.top();
            available.pop();
            result.push_back(static_cast<char>(current.character));
            --current.count;
            if (held && held->count > 0) {
                available.push(*held);
            }
            held = current.count > 0 ? std::optional<CharacterCount>{current}
                                     : std::nullopt;
        }
        if (result.size() != input.size()) {
            return std::nullopt;
        }
        return result;
    }
};

/// Online median using a max-heap lower half and min-heap upper half; the
/// invariant keeps lower at most one value larger and every lower value no
/// greater than every upper value. Supports duplicates and
/// negative values; finding a median on an empty stream returns nullopt.
/// addNum is O(log n) time and O(n) auxiliary space; findMedian is O(1) time
/// and O(1) auxiliary space.
/// Example: add 1, then 2, then 3 -> medians 1, 1.5, 2. A max-heap stores
/// the lower half and a min-heap stores the upper half.
class MedianFinder final {
public:
    MedianFinder() = default;

    /// Adds one value to the stream. Time: O(log n); auxiliary space: O(n).
    /// Example: addNum(1), addNum(2) leaves a median of 1.5.
    void addNum(int value)
    {
        if (lower_.empty() || value <= lower_.top()) {
            lower_.push(value);
        } else {
            upper_.push(value);
        }

        if (lower_.size() > upper_.size() + 1) {
            upper_.push(lower_.top());
            lower_.pop();
        } else if (upper_.size() > lower_.size()) {
            lower_.push(upper_.top());
            upper_.pop();
        }
    }

    /// Returns the middle value or mean of the two middle values; empty stream
    /// yields nullopt. Time: O(1); auxiliary space: O(1).
    /// Example: after adding 1, 2, 3, returns 2; the middle of an odd count.
    [[nodiscard]] std::optional<double> findMedian() const noexcept
    {
        if (lower_.empty()) {
            return std::nullopt;
        }
        if (lower_.size() != upper_.size()) {
            return static_cast<double>(lower_.top());
        }
        return (static_cast<double>(lower_.top()) +
                static_cast<double>(upper_.top())) /
               2.0;
    }

private:
    std::priority_queue<int> lower_;
    std::priority_queue<int, std::vector<int>, std::greater<>> upper_;
};

} // namespace dsa::heaps
