#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dsa::stack {

/// Static solutions for stack-based interview problems (54-63).
///
/// Error model: malformed input throws std::invalid_argument; arithmetic or
/// size overflow throws std::overflow_error / std::length_error; division by
/// zero throws std::domain_error. Inputs are never modified.
class Stack final {
public:
    Stack() = delete;

    /// Problem 55: stack that reports its minimum in O(1).
    ///
    /// Operations: push(value), pop(), top(), getMin(), size(), empty().
    /// Each entry stores the value together with the minimum of the stack at
    /// that depth, so the invariant "entries[i].second == min(values[0..i])"
    /// holds. pop/top/getMin on an empty stack are reported by returning
    /// false / std::nullopt (never undefined behavior).
    /// Time: O(1) amortized per operation. Extra space: O(n) total.
    /// Example: push(3), push(1), push(2), getMin() -> 1; popping 2 leaves
    /// the saved minimum 1 available without rescanning the stack.
    class MinStack final {
    public:
        MinStack() = default;

        /// Pushes value. Time O(1) amortized; extra space O(1).
        /// Example: push(3), then push(1) stores minima 3 and 1 by depth.
        void push(int value)
        {
            const int currentMin =
                entries_.empty() ? value : std::min(value, entries_.back().second);
            entries_.emplace_back(value, currentMin);
        }

        /// Removes the top element. Returns false (and does nothing) if empty.
        /// Time O(1); extra space O(1).
        /// Example: pop() after push(3) returns true and removes 3.
        bool pop() noexcept
        {
            if (entries_.empty()) {
                return false;
            }
            entries_.pop_back();
            return true;
        }

        /// Top value or std::nullopt if empty. Time O(1); extra space O(1).
        /// Example: after push(3), push(1), top() returns 1.
        [[nodiscard]] std::optional<int> top() const noexcept
        {
            if (entries_.empty()) {
                return std::nullopt;
            }
            return entries_.back().first;
        }

        /// Minimum value or std::nullopt if empty. Time O(1); extra space O(1).
        /// Example: after push(3), push(1), getMin() returns 1.
        [[nodiscard]] std::optional<int> getMin() const noexcept
        {
            if (entries_.empty()) {
                return std::nullopt;
            }
            return entries_.back().second;
        }

        /// Number of entries currently stored.
        /// Example: size() is 2 after two pushes.
        [[nodiscard]] std::size_t size() const noexcept { return entries_.size(); }
        /// Whether no entries are stored.
        /// Example: empty() is true initially and after popping every entry.
        [[nodiscard]] bool empty() const noexcept { return entries_.empty(); }

    private:
        std::vector<std::pair<int, int>> entries_;
    };

    /// Problem 54: Valid Parentheses over "()[]{}".
    ///
    /// Technique: stack of expected closers; the stack holds exactly the
    /// unmatched openers. Any other character makes the input invalid
    /// (returns false). Empty input is valid.
    /// Time: O(n). Extra space: O(n).
    /// Example: "([])" -> true; "([)]" -> false. Each opener pushes its
    /// required closer, which must match the next closing symbol.
    [[nodiscard]] static bool isValidParentheses(std::string_view text)
    {
        std::string expected;
        expected.reserve(text.size());
        for (const char c : text) {
            switch (c) {
            case '(': expected.push_back(')'); break;
            case '[': expected.push_back(']'); break;
            case '{': expected.push_back('}'); break;
            case ')':
            case ']':
            case '}':
                if (expected.empty() || expected.back() != c) {
                    return false;
                }
                expected.pop_back();
                break;
            default:
                return false;
            }
        }
        return expected.empty();
    }

    /// Problem 56: Daily Temperatures.
    ///
    /// result[i] is the number of days until a strictly warmer day, else 0.
    /// Technique: monotonic stack of indices with non-increasing temperatures.
    /// Time: O(n). Extra space: O(n).
    /// Example: [73,74,75,71,69,72,76,73] -> [1,1,4,2,1,1,0,0].
    /// Resolve stacked cooler days when a warmer temperature arrives.
    [[nodiscard]] static std::vector<int> dailyTemperatures(std::span<const int> temperatures)
    {
        std::vector<int> result(temperatures.size(), 0);
        std::vector<std::size_t> pending;
        for (std::size_t i = 0; i < temperatures.size(); ++i) {
            while (!pending.empty() && temperatures[pending.back()] < temperatures[i]) {
                result[pending.back()] = static_cast<int>(i - pending.back());
                pending.pop_back();
            }
            pending.push_back(i);
        }
        return result;
    }

    /// Problem 57: Car Fleet.
    ///
    /// Cars drive toward target and cannot pass; a car catching one ahead
    /// joins its fleet. Preconditions (else std::invalid_argument): equal
    /// sizes, 0 <= position < target, speed > 0. Cars sharing a position
    /// travel together (ties are ordered slowest first). Arrival times are
    /// compared exactly by cross multiplication (no floating point).
    /// Technique: sort by distance to target ascending, count strictly
    /// increasing arrival times. Time: O(n log n). Extra space: O(n).
    /// Example: target = 12, positions = [10,8,0,5,3], speeds = [2,4,1,1,3]
    /// -> 3 fleets. Sort nearest-to-target first and merge cars whose arrival
    /// time is no later than the fleet ahead.
    [[nodiscard]] static std::size_t carFleet(int target, std::span<const int> position,
                                              std::span<const int> speed)
    {
        if (position.size() != speed.size()) {
            throw std::invalid_argument("position and speed sizes differ");
        }
        std::vector<std::pair<long long, long long>> cars; // distance, speed
        cars.reserve(position.size());
        for (std::size_t i = 0; i < position.size(); ++i) {
            if (position[i] < 0 || position[i] >= target || speed[i] <= 0) {
                throw std::invalid_argument("position must be in [0,target) and speed positive");
            }
            cars.emplace_back(static_cast<long long>(target) - position[i], speed[i]);
        }
        std::ranges::sort(cars, [](const auto& a, const auto& b) {
            return a.first != b.first ? a.first < b.first : a.second < b.second;
        });
        std::size_t fleets = 0;
        long long leadDistance = 0;
        long long leadSpeed = 1;
        for (const auto& [distance, carSpeed] : cars) {
            // new fleet iff distance/carSpeed > leadDistance/leadSpeed
            if (fleets == 0 || distance * leadSpeed > leadDistance * carSpeed) {
                ++fleets;
                leadDistance = distance;
                leadSpeed = carSpeed;
            }
        }
        return fleets;
    }

    /// Problem 58: Largest Rectangle in Histogram.
    ///
    /// Throws std::invalid_argument on a negative height. Returns 0 for an
    /// empty histogram. Technique: monotonic stack of increasing heights; a
    /// popped bar's maximal width is bounded by the new top and current index.
    /// Time: O(n). Extra space: O(n).
    /// Example: heights = [2,1,5,6,2,3] -> area 10. When a shorter bar
    /// arrives, each taller popped bar has found its first right boundary.
    [[nodiscard]] static long long largestRectangleArea(std::span<const int> heights)
    {
        for (const int h : heights) {
            if (h < 0) {
                throw std::invalid_argument("negative height");
            }
        }
        std::vector<std::size_t> increasing;
        long long best = 0;
        for (std::size_t i = 0; i <= heights.size(); ++i) {
            const long long current = i == heights.size() ? 0 : heights[i];
            while (!increasing.empty() && heights[increasing.back()] >= current) {
                const long long height = heights[increasing.back()];
                increasing.pop_back();
                const std::size_t left = increasing.empty() ? 0 : increasing.back() + 1;
                best = std::max(best, height * static_cast<long long>(i - left));
            }
            increasing.push_back(i);
        }
        return best;
    }

    /// Problem 59: Maximal Rectangle of '1' cells in a binary matrix.
    ///
    /// Rows must have equal length and contain only '0'/'1', else
    /// std::invalid_argument. Empty matrix or empty rows give 0.
    /// Technique: per-row histogram of consecutive ones fed to
    /// largestRectangleArea. Time: O(rows*cols). Extra space: O(cols).
    /// Example: ["10100","10111","11111","10010"] -> area 6. Each row
    /// updates column heights, then the histogram solver finds its best block.
    [[nodiscard]] static long long maximalRectangle(std::span<const std::string> matrix)
    {
        if (matrix.empty()) {
            return 0;
        }
        const std::size_t columns = matrix.front().size();
        std::vector<int> heights(columns, 0);
        long long best = 0;
        for (const std::string& row : matrix) {
            if (row.size() != columns) {
                throw std::invalid_argument("ragged matrix");
            }
            for (std::size_t c = 0; c < columns; ++c) {
                if (row[c] != '0' && row[c] != '1') {
                    throw std::invalid_argument("matrix cells must be '0' or '1'");
                }
                heights[c] = row[c] == '1' ? heights[c] + 1 : 0;
            }
            best = std::max(best, largestRectangleArea(heights));
        }
        return best;
    }

    /// Problem 60: Decode String, e.g. "3[a2[c]]" -> "accaccacc".
    ///
    /// Grammar: letters and k[encoded]; k is a positive unsigned decimal.
    /// Any other character, unbalanced brackets, a count not followed by '['
    /// or a zero count is std::invalid_argument (empty bodies like "2[]" are
    /// allowed). If a count or the decoded size would exceed maxLength,
    /// std::length_error is thrown (so no overflow or runaway allocation).
    /// Technique: stack of (prefix, repeat) frames.
    /// Time: O(n + output). Extra space: O(n + output).
    /// Example: "3[a2[c]]" -> "accaccacc". A frame saves the text before '['
    /// and its repeat count; ']' expands the completed nested segment.
    [[nodiscard]] static std::string decodeString(std::string_view encoded,
                                                  std::size_t maxLength = 100'000'000)
    {
        struct Frame {
            std::string prefix;
            std::size_t repeat;
        };
        std::vector<Frame> frames;
        std::string current;
        std::size_t i = 0;
        while (i < encoded.size()) {
            const char c = encoded[i];
            if (c >= '0' && c <= '9') {
                std::size_t count = 0;
                while (i < encoded.size() && encoded[i] >= '0' && encoded[i] <= '9') {
                    const std::size_t digit = static_cast<std::size_t>(encoded[i] - '0');
                    if (digit > maxLength || count > (maxLength - digit) / 10) {
                        throw std::length_error("repeat count too large");
                    }
                    count = count * 10 + digit;
                    ++i;
                }
                if (i >= encoded.size() || encoded[i] != '[') {
                    throw std::invalid_argument("count must be followed by '['");
                }
                if (count == 0) {
                    throw std::invalid_argument("repeat count must be positive");
                }
                frames.push_back({std::move(current), count});
                current.clear();
                ++i;
            } else if (c == ']') {
                if (frames.empty()) {
                    throw std::invalid_argument("unmatched ']'");
                }
                Frame frame = std::move(frames.back());
                frames.pop_back();
                if (!current.empty() &&
                    frame.repeat > (maxLength - std::min(maxLength, frame.prefix.size())) / current.size()) {
                    throw std::length_error("decoded string too long");
                }
                std::string expanded = std::move(frame.prefix);
                for (std::size_t r = 0; r < frame.repeat && !current.empty(); ++r) {
                    expanded += current;
                }
                current = std::move(expanded);
                ++i;
            } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                if (current.size() >= maxLength) {
                    throw std::length_error("decoded string too long");
                }
                current.push_back(c);
                ++i;
            } else {
                throw std::invalid_argument("unexpected character");
            }
        }
        if (!frames.empty()) {
            throw std::invalid_argument("unmatched '['");
        }
        return current;
    }

    /// Problem 61: Remove K Digits to get the smallest number.
    ///
    /// num must be digits only (else std::invalid_argument). Result has no
    /// leading zeros; "0" if everything is removed. k >= length removes all.
    /// Technique: monotonic non-decreasing stack, drop larger predecessors.
    /// Time: O(n). Extra space: O(n).
    /// Example: num = "1432219", k = 3 -> "1219". Remove a larger preceding
    /// digit whenever a smaller digit arrives, then trim any remaining suffix.
    [[nodiscard]] static std::string removeKDigits(std::string_view num, std::size_t k)
    {
        for (const char c : num) {
            if (c < '0' || c > '9') {
                throw std::invalid_argument("non-digit character");
            }
        }
        if (k >= num.size()) {
            return "0";
        }
        std::string kept;
        kept.reserve(num.size());
        for (const char c : num) {
            while (k > 0 && !kept.empty() && kept.back() > c) {
                kept.pop_back();
                --k;
            }
            kept.push_back(c);
        }
        kept.resize(kept.size() - k);
        const std::size_t firstNonZero = kept.find_first_not_of('0');
        return firstNonZero == std::string::npos ? "0" : kept.substr(firstNonZero);
    }

    /// Problem 62: Asteroid Collision.
    ///
    /// Sign is direction, magnitude is size; equal sizes both explode.
    /// Zero-sized asteroids throw std::invalid_argument. Magnitudes are
    /// compared in long long so INT_MIN is safe. Technique: stack of survivors.
    /// Time: O(n). Extra space: O(n).
    /// Example: [5,10,-5] -> [5]. Only a right-moving survivor and incoming
    /// left-moving asteroid can collide; compare magnitudes until resolved.
    [[nodiscard]] static std::vector<int> asteroidCollision(std::span<const int> asteroids)
    {
        std::vector<int> survivors;
        survivors.reserve(asteroids.size());
        for (const int asteroid : asteroids) {
            if (asteroid == 0) {
                throw std::invalid_argument("asteroid size must be non-zero");
            }
            bool alive = true;
            while (alive && asteroid < 0 && !survivors.empty() && survivors.back() > 0) {
                const long long top = survivors.back();
                const long long incoming = -static_cast<long long>(asteroid);
                if (top < incoming) {
                    survivors.pop_back();
                } else {
                    if (top == incoming) {
                        survivors.pop_back();
                    }
                    alive = false;
                }
            }
            if (alive) {
                survivors.push_back(asteroid);
            }
        }
        return survivors;
    }

    /// Problem 63: Basic Calculator II for non-negative integers with
    /// '+', '-', '*', '/' and spaces; division truncates toward zero.
    ///
    /// No parentheses or unary operators. Malformed expressions (empty,
    /// dangling/adjacent operators, other characters, adjacent numbers)
    /// throw std::invalid_argument; division by zero throws std::domain_error;
    /// any literal, intermediate or final value outside long long throws
    /// std::overflow_error (checked arithmetic, no wraparound).
    /// Technique: running sum plus the current multiplicative term.
    /// Time: O(n). Extra space: O(1).
    /// Example: "3+2*2" -> 7. Commit completed additive terms to sum, while
    /// applying multiplication/division immediately to the current term.
    [[nodiscard]] static long long calculate(std::string_view expression)
    {
        constexpr long long maxValue = std::numeric_limits<long long>::max();
        constexpr long long minValue = std::numeric_limits<long long>::min();
        auto addChecked = [&](long long a, long long b) {
            if ((b > 0 && a > maxValue - b) || (b < 0 && a < minValue - b)) {
                throw std::overflow_error("calculator overflow");
            }
            return a + b;
        };
        auto mulChecked = [&](long long a, long long b) {
            if (a == 0 || b == 0) {
                return 0LL;
            }
            if ((a > 0) == (b > 0)) {
                const bool tooBig = a > 0 ? a > maxValue / b : a < maxValue / b;
                if (tooBig) {
                    throw std::overflow_error("calculator overflow");
                }
            } else {
                // minValue / -1 would itself overflow, so -1 is special-cased.
                if (a == -1) {
                    return -b;
                }
                if (b == -1) {
                    return -a;
                }
                const bool tooSmall = a > 0 ? b < minValue / a : a < minValue / b;
                if (tooSmall) {
                    throw std::overflow_error("calculator overflow");
                }
            }
            return a * b;
        };

        long long sum = 0;
        long long term = 0;
        char pending = '+'; // operator that applies to the next number
        bool expectNumber = true;
        bool sawNumber = false;
        std::size_t i = 0;
        while (i < expression.size()) {
            const char c = expression[i];
            if (c == ' ') {
                ++i;
            } else if (c >= '0' && c <= '9') {
                if (!expectNumber) {
                    throw std::invalid_argument("missing operator");
                }
                long long number = 0;
                while (i < expression.size() && expression[i] >= '0' && expression[i] <= '9') {
                    const int digit = expression[i] - '0';
                    if (number > (maxValue - digit) / 10) {
                        throw std::overflow_error("number literal overflow");
                    }
                    number = number * 10 + digit;
                    ++i;
                }
                switch (pending) {
                case '+': sum = addChecked(sum, term); term = number; break;
                case '-': sum = addChecked(sum, term); term = -number; break;
                case '*': term = mulChecked(term, number); break;
                default:
                    if (number == 0) {
                        throw std::domain_error("division by zero");
                    }
                    term /= number;
                    break;
                }
                sawNumber = true;
                expectNumber = false;
            } else if (c == '+' || c == '-' || c == '*' || c == '/') {
                if (expectNumber) {
                    throw std::invalid_argument("operator without left operand");
                }
                pending = c;
                expectNumber = true;
                ++i;
            } else {
                throw std::invalid_argument("unexpected character");
            }
        }
        if (!sawNumber || expectNumber) {
            throw std::invalid_argument("incomplete expression");
        }
        return addChecked(sum, term);
    }
};

} // namespace dsa::stack