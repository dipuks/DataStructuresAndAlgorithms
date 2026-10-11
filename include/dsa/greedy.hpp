#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dsa::greedy {

/// Static greedy solutions for interview problems.
class Greedy final {
public:
    Greedy() = delete;

    /// Reports whether the last index is reachable. Jump lengths must be
    /// nonnegative or std::invalid_argument is thrown. Empty and singleton
    /// inputs are reachable. Input is unchanged. Time: O(n); extra space: O(1).
    /// Invariant: farthest is the greatest reachable index from any scanned
    /// index, and every scanned index is reachable.
    /// Example: [2,3,1,1,4] -> true. Extend the farthest reachable boundary;
    /// encountering an index beyond it proves the end cannot be reached.
    [[nodiscard]] static bool jumpGame(std::span<const int> jumps)
    {
        validateNonnegative(jumps, "jump lengths must be nonnegative");
        if (jumps.size() <= 1) {
            return true;
        }
        std::size_t farthest = 0;
        for (std::size_t index = 0; index < jumps.size(); ++index) {
            if (index > farthest) {
                return false;
            }
            const std::size_t step = static_cast<std::size_t>(jumps[index]);
            if (step >= jumps.size() - 1 - index) {
                return true;
            }
            farthest = std::max(farthest, index + step);
        }
        return true;
    }

    /// Returns the fewest jumps needed to reach the last index, or nullopt
    /// when unreachable. Jump lengths must be nonnegative or
    /// std::invalid_argument is thrown. Empty input is infeasible; singleton
    /// input needs zero jumps. Input is unchanged. Time: O(n); extra space: O(1).
    /// Invariant: currentEnd is the current jump layer's boundary and
    /// farthest is the next layer's furthest reachable index.
    /// Example: [2,3,1,1,4] -> 2 jumps. Treat all indices reachable in one
    /// jump count as a layer, then commit to the next layer's farthest edge.
    [[nodiscard]] static std::optional<std::size_t> jumpGameII(
        std::span<const int> jumps)
    {
        validateNonnegative(jumps, "jump lengths must be nonnegative");
        if (jumps.empty()) {
            return std::nullopt;
        }
        if (jumps.size() == 1) {
            return 0;
        }
        std::size_t currentEnd = 0;
        std::size_t farthest = 0;
        std::size_t count = 0;
        for (std::size_t index = 0; index + 1 < jumps.size(); ++index) {
            if (index > currentEnd) {
                return std::nullopt;
            }
            const std::size_t step = static_cast<std::size_t>(jumps[index]);
            const std::size_t reach =
                step >= jumps.size() - 1 - index
                    ? jumps.size() - 1
                    : index + step;
            farthest = std::max(farthest, reach);
            if (index == currentEnd) {
                if (farthest == currentEnd) {
                    return std::nullopt;
                }
                ++count;
                currentEnd = farthest;
                if (currentEnd == jumps.size() - 1) {
                    return count;
                }
            }
        }
        return std::nullopt;
    }

    /// Returns a start index from which the complete circuit is possible,
    /// or nullopt if no solution exists. Gas/cost lengths must match and all
    /// values must be nonnegative, otherwise std::invalid_argument is thrown.
    /// Empty input returns nullopt. Input is unchanged. Time: O(n); extra space: O(1).
    /// Invariant: candidate is the only possible start after the most recent
    /// negative tank prefix; total tracks feasibility over the full circuit.
    /// Example: gas = [1,2,3,4,5], cost = [3,4,5,1,2] -> start 3. A failed
    /// prefix rules out every start up to that point; total checks feasibility.
    [[nodiscard]] static std::optional<std::size_t> gasStation(
        std::span<const int> gas,
        std::span<const int> cost)
    {
        if (gas.size() != cost.size()) {
            throw std::invalid_argument("gas and cost lengths must match");
        }
        validateNonnegative(gas, "gas amounts must be nonnegative");
        validateNonnegative(cost, "cost amounts must be nonnegative");
        long long total = 0;
        long long tank = 0;
        std::size_t candidate = 0;
        for (std::size_t index = 0; index < gas.size(); ++index) {
            const long long difference =
                static_cast<long long>(gas[index]) - cost[index];
            total = checkedAdd(total, difference);
            tank = checkedAdd(tank, difference);
            if (tank < 0) {
                tank = 0;
                candidate = index + 1;
            }
        }
        if (gas.empty() || total < 0) {
            return std::nullopt;
        }
        return candidate == gas.size() ? std::optional<std::size_t>{0}
                                       : std::optional<std::size_t>{candidate};
    }

    /// Computes the minimum candies satisfying the neighbor-rating rule.
    /// Ratings may be any integers; input is unchanged. Throws
    /// std::overflow_error if the total cannot fit in long long.
    /// Time: O(n); extra space: O(n). Invariant: the left-to-right pass
    /// satisfies each rising edge; the reverse pass raises values as needed
    /// to satisfy each falling edge.
    /// Example: ratings = [1,0,2] -> 5 candies. One pass handles rising
    /// neighbors; the reverse pass handles falling neighbors.
    [[nodiscard]] static long long candy(std::span<const int> ratings)
    {
        if (ratings.empty()) {
            return 0;
        }
        std::vector<long long> candies(ratings.size(), 1);
        for (std::size_t index = 1; index < ratings.size(); ++index) {
            if (ratings[index] > ratings[index - 1]) {
                candies[index] = checkedAdd(candies[index - 1], 1);
            }
        }
        for (std::size_t index = ratings.size() - 1; index > 0; --index) {
            if (ratings[index - 1] > ratings[index]) {
                candies[index - 1] =
                    std::max(candies[index - 1], checkedAdd(candies[index], 1));
            }
        }
        long long total = 0;
        for (const long long count : candies) {
            total = checkedAdd(total, count);
        }
        return total;
    }

    /// Reconstructs a queue from (height, k) pairs where k is the number of
    /// people of at least that height before the person. Negative heights or
    /// k, and impossible constraints, throw std::invalid_argument. Input is
    /// unchanged. Time: O(n^2); extra space: O(n). Invariant: inserting each
    /// descending-height person at index k places exactly k qualifying people
    /// before them; a final validation enforces the input constraints.
    /// Example: [(7,0),(4,4),(7,1),(5,0),(6,1),(5,2)] ->
    /// [(5,0),(7,0),(5,2),(6,1),(4,4),(7,1)]. Insert tallest first at k.
    [[nodiscard]] static std::vector<std::pair<int, int>> queueReconstructionByHeight(
        std::span<const std::pair<int, int>> people)
    {
        std::vector<std::pair<int, int>> ordered(people.begin(), people.end());
        for (const auto& [height, k] : ordered) {
            if (height < 0 || k < 0) {
                throw std::invalid_argument("height and k must be nonnegative");
            }
        }
        std::ranges::sort(ordered, [](const auto& first, const auto& second) {
            return first.first != second.first ? first.first > second.first
                                               : first.second < second.second;
        });
        std::vector<std::pair<int, int>> queue;
        queue.reserve(ordered.size());
        for (const auto& person : ordered) {
            const auto position = static_cast<std::size_t>(person.second);
            if (position > queue.size()) {
                throw std::invalid_argument("queue constraints are impossible");
            }
            queue.insert(queue.begin() + static_cast<std::ptrdiff_t>(position),
                         person);
        }
        for (std::size_t index = 0; index < queue.size(); ++index) {
            std::size_t precedingAtLeastHeight = 0;
            for (std::size_t previous = 0; previous < index; ++previous) {
                precedingAtLeastHeight +=
                    queue[previous].first >= queue[index].first;
            }
            if (precedingAtLeastHeight !=
                static_cast<std::size_t>(queue[index].second)) {
                throw std::invalid_argument("queue constraints are inconsistent");
            }
        }
        return queue;
    }

    /// Sums all positive day-to-day price increases (unlimited transactions).
    /// Prices must be nonnegative or std::invalid_argument is thrown.
    /// Throws std::overflow_error if the result does not fit in long long.
    /// Time: O(n); extra space: O(1). Invariant: every adjacent positive
    /// increase is banked, which is equivalent to summing all profitable
    /// buy/sell transactions.
    /// Example: [7,1,5,3,6,4] -> 7. Collect each positive adjacent rise;
    /// splitting a multi-day rise into transactions preserves the same profit.
    [[nodiscard]] static long long stockII(std::span<const int> prices)
    {
        validateNonnegative(prices, "prices must be nonnegative");
        long long profit = 0;
        for (std::size_t index = 1; index < prices.size(); ++index) {
            if (prices[index] > prices[index - 1]) {
                profit = checkedAdd(
                    profit,
                    static_cast<long long>(prices[index]) - prices[index - 1]);
            }
        }
        return profit;
    }

    /// Reports whether cards can be partitioned into consecutive groups of
    /// groupSize. groupSize must be positive or std::invalid_argument is
    /// thrown. Empty input is true; input is unchanged. Time: O(n log n);
    /// extra space: O(n). Invariant: repeatedly consume the required number
    /// of consecutive cards beginning with the smallest remaining card.
    /// Example: cards = [1,2,3,6,2,3,4,7,8], groupSize = 3 -> true. The
    /// smallest remaining value must begin a group, so consume the next run.
    [[nodiscard]] static bool handOfStraights(
        std::span<const int> cards,
        std::size_t groupSize)
    {
        if (groupSize == 0) {
            throw std::invalid_argument("group size must be positive");
        }
        if (cards.size() % groupSize != 0) {
            return false;
        }
        if (cards.empty()) {
            return true;
        }
        if (groupSize > cards.size()) {
            return false;
        }
        std::map<int, std::size_t> counts;
        for (const int card : cards) {
            ++counts[card];
        }
        while (!counts.empty()) {
            const int start = counts.begin()->first;
            const std::size_t multiplicity = counts.begin()->second;
            for (std::size_t offset = 0; offset < groupSize; ++offset) {
                const long long value =
                    static_cast<long long>(start) +
                    static_cast<long long>(offset);
                if (value > std::numeric_limits<int>::max()) {
                    return false;
                }
                const auto found = counts.find(static_cast<int>(value));
                if (found == counts.end() || found->second < multiplicity) {
                    return false;
                }
                found->second -= multiplicity;
                if (found->second == 0) {
                    counts.erase(found);
                }
            }
        }
        return true;
    }

    /// Returns the minimum deletions needed so all byte-character
    /// frequencies are unique. Input is unchanged; the empty string needs
    /// zero deletions. Time: O(n + 256^2); extra space: O(1). Invariant:
    /// used contains the positive frequencies already retained, and each
    /// duplicate frequency is reduced until unique or zero.
    /// Example: "aaabbbcc" -> 2 deletions. Keep each positive character
    /// frequency unique by lowering collisions until a free count is found.
    [[nodiscard]] static std::size_t minDeletionsUniqueFrequencies(
        std::string_view text)
    {
        std::array<std::size_t, 256> frequencies{};
        for (const char character : text) {
            ++frequencies[static_cast<unsigned char>(character)];
        }
        std::unordered_set<std::size_t> used;
        std::size_t deletions = 0;
        for (std::size_t frequency : frequencies) {
            while (frequency > 0 && !used.insert(frequency).second) {
                --frequency;
                ++deletions;
            }
        }
        return deletions;
    }

    /// Splits a string into the largest possible contiguous partitions such
    /// that each byte appears in at most one partition. Returns lengths;
    /// input is unchanged. Time: O(n); extra space: O(1). Invariant: end is
    /// the furthest last occurrence of any byte in the current partition.
    /// Example: "ababcbacadefegdehijhklij" -> [9,7,8]. Extend each partition
    /// to include the last occurrence of every character already seen.
    [[nodiscard]] static std::vector<std::size_t> partitionLabels(
        std::string_view text)
    {
        std::array<std::size_t, 256> last{};
        for (std::size_t index = 0; index < text.size(); ++index) {
            last[static_cast<unsigned char>(text[index])] = index;
        }
        std::vector<std::size_t> partitions;
        std::size_t start = 0;
        std::size_t end = 0;
        for (std::size_t index = 0; index < text.size(); ++index) {
            end = std::max(end, last[static_cast<unsigned char>(text[index])]);
            if (index == end) {
                partitions.push_back(end - start + 1);
                start = index + 1;
            }
        }
        return partitions;
    }

    /// Reports whether change can be made for every lemonade customer paying
    /// with 5, 10, or 20. Other bills throw std::invalid_argument. Input is
    /// unchanged. Time: O(n); extra space: O(1). Invariant: fives and tens
    /// are the available bills after serving the processed customers; use a
    /// ten and five before three fives when giving a twenty in change.
    /// Example: [5,5,5,10,20] -> true. Give a 20-change using 10+5 first,
    /// preserving three 5s when that better future option is available.
    [[nodiscard]] static bool lemonadeChange(std::span<const int> bills)
    {
        if (std::ranges::any_of(bills, [](int bill) {
                return bill != 5 && bill != 10 && bill != 20;
            })) {
            throw std::invalid_argument("bills must be 5, 10, or 20");
        }
        std::size_t fives = 0;
        std::size_t tens = 0;
        for (const int bill : bills) {
            if (bill == 5) {
                ++fives;
            } else if (bill == 10) {
                if (fives == 0) {
                    return false;
                }
                --fives;
                ++tens;
            } else if (bill == 20) {
                if (tens > 0 && fives > 0) {
                    --tens;
                    --fives;
                } else if (fives >= 3) {
                    fives -= 3;
                } else {
                    return false;
                }
            } else {
                throw std::invalid_argument("bills must be 5, 10, or 20");
            }
        }
        return true;
    }

    /// Maximizes the number of children receiving a cookie at least as large
    /// as their greed. Greed and cookie sizes must be nonnegative or
    /// std::invalid_argument is thrown. Inputs are unchanged.
    /// Time: O(n log n + m log m); extra space: O(n + m). Invariant:
    /// process cookies from smallest to largest, satisfying the least-greedy
    /// unsatisfied child whenever the current cookie is large enough.
    /// Example: greed = [1,2,3], cookies = [1,1] -> 1 child. Match small
    /// cookies to the least-greedy child they can satisfy.
    [[nodiscard]] static std::size_t assignCookies(
        std::span<const int> greed,
        std::span<const int> cookies)
    {
        validateNonnegative(greed, "greed factors must be nonnegative");
        validateNonnegative(cookies, "cookie sizes must be nonnegative");
        std::vector<int> children(greed.begin(), greed.end());
        std::vector<int> sizes(cookies.begin(), cookies.end());
        std::ranges::sort(children);
        std::ranges::sort(sizes);
        std::size_t child = 0;
        for (const int size : sizes) {
            if (child < children.size() && size >= children[child]) {
                ++child;
            }
        }
        return child;
    }

    /// Reports whether some triplets can be merged componentwise to equal
    /// target. Triplets and target must be nonnegative or
    /// std::invalid_argument is thrown. Inputs are unchanged.
    /// Time: O(n); extra space: O(1). Invariant: matched marks exactly the
    /// target components supplied by some usable triplet; unusable triplets
    /// exceed the target in at least one component and cannot contribute.
    /// Example: target = [2,7,5], triplets = [[2,3,4],[1,7,5]] -> true.
    /// Ignore triplets that exceed the target and collect matching components.
    [[nodiscard]] static bool mergeTriplets(
        std::span<const std::array<int, 3>> triplets,
        const std::array<int, 3>& target)
    {
        validateNonnegative(target, "target components must be nonnegative");
        std::array<bool, 3> matched{false, false, false};
        for (const auto& triplet : triplets) {
            validateNonnegative(triplet, "triplet components must be nonnegative");
            if (triplet[0] <= target[0] &&
                triplet[1] <= target[1] &&
                triplet[2] <= target[2]) {
                for (std::size_t component = 0; component < 3; ++component) {
                    matched[component] =
                        matched[component] ||
                        triplet[component] == target[component];
                }
            }
        }
        return matched[0] && matched[1] && matched[2];
    }

private:
    static void validateNonnegative(
        std::span<const int> values,
        const char* message)
    {
        if (std::ranges::any_of(values, [](int value) { return value < 0; })) {
            throw std::invalid_argument(message);
        }
    }

    template <std::size_t Size>
    static void validateNonnegative(
        const std::array<int, Size>& values,
        const char* message)
    {
        if (std::ranges::any_of(values, [](int value) { return value < 0; })) {
            throw std::invalid_argument(message);
        }
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
};

} // namespace dsa::greedy
