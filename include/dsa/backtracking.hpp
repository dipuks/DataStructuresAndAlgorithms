#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace dsa::backtracking {

/// Static backtracking solutions for interview problems.
class Backtracking final {
public:
    Backtracking() = delete;

    /// Generates every subset, preserving input order inside each subset.
    /// Any integer values are accepted; duplicates are distinct positions.
    /// The input is not modified. Time: O(n * 2^n); extra space: O(n) beyond
    /// the returned subsets (recursive path and call stack). Invariant: path
    /// contains exactly the chosen values among the positions already visited.
    /// Example: [1,2] -> [[],[2],[1],[1,2]] (exclude-first output order).
    /// Each position creates two recursive choices: omit it or include it.
    [[nodiscard]] static std::vector<std::vector<int>> subsets(
        std::span<const int> numbers)
    {
        std::vector<std::vector<int>> result;
        std::vector<int> path;
        std::function<void(std::size_t)> visit = [&](std::size_t index) {
            if (index == numbers.size()) {
                result.push_back(path);
                return;
            }
            visit(index + 1);
            path.push_back(numbers[index]);
            visit(index + 1);
            path.pop_back();
        };
        visit(0);
        return result;
    }

    /// Generates each distinct subset once, in nondecreasing value order.
    /// Any integers are accepted; input may contain duplicates and is copied
    /// before sorting. Time: O(n log n + n * 2^n); extra space: O(n) beyond
    /// the result. Invariant: path is sorted and every value is drawn from
    /// positions at or after the current recursion start.
    /// Example: [1,2,2] -> [[],[1],[1,2],[1,2,2],[2],[2,2]]. Sorting makes
    /// equal choices adjacent so each recursion level can skip duplicates.
    [[nodiscard]] static std::vector<std::vector<int>> subsetsWithDuplicates(
        std::span<const int> numbers)
    {
        std::vector<int> sorted(numbers.begin(), numbers.end());
        std::ranges::sort(sorted);
        std::vector<std::vector<int>> result;
        std::vector<int> path;
        std::function<void(std::size_t)> visit = [&](std::size_t start) {
            result.push_back(path);
            for (std::size_t index = start; index < sorted.size(); ++index) {
                if (index > start && sorted[index] == sorted[index - 1]) {
                    continue;
                }
                path.push_back(sorted[index]);
                visit(index + 1);
                path.pop_back();
            }
        };
        visit(0);
        return result;
    }

    /// Returns all distinct permutations; equal input values are not
    /// distinguished, so duplicates never produce repeated permutations.
    /// Any integers are accepted; input is copied and sorted.
    /// Time: O(n * n!) in the distinct-value worst case; extra space: O(n)
    /// beyond the result. Invariant: path contains each selected position
    /// once, and equal adjacent values are selected in index order.
    /// Example: [1,1,2] -> [[1,1,2],[1,2,1],[2,1,1]]. Used flags select
    /// positions, while the sorted duplicate rule avoids repeated results.
    [[nodiscard]] static std::vector<std::vector<int>> permutations(
        std::span<const int> numbers)
    {
        std::vector<int> sorted(numbers.begin(), numbers.end());
        std::ranges::sort(sorted);
        std::vector<std::vector<int>> result;
        std::vector<int> path;
        std::vector<bool> used(sorted.size(), false);
        std::function<void()> visit = [&] {
            if (path.size() == sorted.size()) {
                result.push_back(path);
                return;
            }
            for (std::size_t index = 0; index < sorted.size(); ++index) {
                if (used[index] ||
                    (index > 0 && sorted[index] == sorted[index - 1] &&
                     !used[index - 1])) {
                    continue;
                }
                used[index] = true;
                path.push_back(sorted[index]);
                visit();
                path.pop_back();
                used[index] = false;
            }
        };
        visit();
        return result;
    }

    /// Finds unique nondecreasing combinations summing to target. Candidates
    /// must be positive and target nonnegative; invalid values throw
    /// std::invalid_argument. Duplicate candidates are collapsed. The input
    /// is not modified. Time: exponential in target/min(candidate); extra
    /// space: O(target/min(candidate)) for recursion plus the result.
    /// Invariant: path is nondecreasing and its sum never exceeds target.
    /// Example: candidates = [2,3,6,7], target = 7 -> [[2,2,3],[7]].
    /// Recurse at the same index to allow reuse; later indices keep choices
    /// ordered and prevent generating permutations of the same combination.
    [[nodiscard]] static std::vector<std::vector<int>> combinationSum(
        std::span<const int> candidates,
        int target)
    {
        if (target < 0) {
            throw std::invalid_argument("target must be nonnegative");
        }
        std::vector<int> values(candidates.begin(), candidates.end());
        if (std::ranges::any_of(values, [](int value) { return value <= 0; })) {
            throw std::invalid_argument("candidates must be positive");
        }
        std::ranges::sort(values);
        values.erase(std::unique(values.begin(), values.end()), values.end());

        std::vector<std::vector<int>> result;
        std::vector<int> path;
        std::function<void(std::size_t, int)> visit =
            [&](std::size_t start, int remaining) {
                if (remaining == 0) {
                    result.push_back(path);
                    return;
                }
                for (std::size_t index = start; index < values.size(); ++index) {
                    if (values[index] > remaining) {
                        break;
                    }
                    path.push_back(values[index]);
                    visit(index, remaining - values[index]);
                    path.pop_back();
                }
            };
        visit(0, target);
        return result;
    }

    /// Reports whether a word can be formed by orthogonally adjacent board
    /// cells without reusing a cell. Board rows must be rectangular or
    /// std::invalid_argument is thrown. Empty word is true; a nonempty word
    /// on an empty board is false. Neither input is modified.
    /// Time: O(R*C*4^L) worst case; extra space: O(R*C) for visited cells
    /// and O(L) recursion. Invariant: visited cells are exactly those on the
    /// current path, which matches the corresponding word prefix.
    /// Example: board = ["ABCE","SFCS","ADEE"], word = "ABCCED" -> true.
    /// A recursive step appends one adjacent matching cell; backtracking
    /// unmarks it before trying another route.
    [[nodiscard]] static bool wordSearch(
        std::span<const std::string> board,
        std::string_view word)
    {
        if (board.empty()) {
            return word.empty();
        }
        const std::size_t columns = board.front().size();
        for (const std::string& row : board) {
            if (row.size() != columns) {
                throw std::invalid_argument("board must be rectangular");
            }
        }
        if (word.empty()) {
            return true;
        }
        if (columns == 0) {
            return false;
        }
        if (board.size() >
            std::numeric_limits<std::size_t>::max() / columns) {
            throw std::length_error("board is too large to search");
        }
        const std::size_t cellCount = board.size() * columns;
        if (word.size() > cellCount) {
            return false;
        }

        std::vector<bool> visited(cellCount, false);
        std::function<bool(std::size_t, std::size_t, std::size_t)> visit =
            [&](std::size_t row, std::size_t column, std::size_t index) {
                if (board[row][column] != word[index]) {
                    return false;
                }
                if (index + 1 == word.size()) {
                    return true;
                }
                const std::size_t cell = row * columns + column;
                visited[cell] = true;
                const bool found =
                    (row > 0 && !visited[cell - columns] &&
                     visit(row - 1, column, index + 1)) ||
                    (row + 1 < board.size() && !visited[cell + columns] &&
                     visit(row + 1, column, index + 1)) ||
                    (column > 0 && !visited[cell - 1] &&
                     visit(row, column - 1, index + 1)) ||
                    (column + 1 < columns && !visited[cell + 1] &&
                     visit(row, column + 1, index + 1));
                visited[cell] = false;
                return found;
            };

        for (std::size_t row = 0; row < board.size(); ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                if (visit(row, column, 0)) {
                    return true;
                }
            }
        }
        return false;
    }

    /// Returns every partition whose pieces are palindromes (byte-wise).
    /// Empty input has one partition: the empty partition. Input is not
    /// modified. Time: O(n * 2^n) worst case; extra space: O(n^2) for the
    /// palindrome table/path, excluding returned partitions. Invariant:
    /// palindrome[i][j] is true exactly when the inclusive range is a
    /// palindrome; path covers the text prefix before each recursive call.
    /// Example: "aab" -> [["a","a","b"],["aa","b"]]. The table makes each
    /// candidate cut test constant-time before the recursive search.
    [[nodiscard]] static std::vector<std::vector<std::string>>
    palindromePartitioning(std::string_view text)
    {
        const std::size_t size = text.size();
        std::vector<std::vector<bool>> palindrome(
            size, std::vector<bool>(size, false));
        for (std::size_t start = size; start > 0; --start) {
            const std::size_t left = start - 1;
            for (std::size_t right = left; right < size; ++right) {
                palindrome[left][right] =
                    text[left] == text[right] &&
                    (right - left < 2 || palindrome[left + 1][right - 1]);
            }
        }

        std::vector<std::vector<std::string>> result;
        std::vector<std::string> path;
        std::function<void(std::size_t)> visit = [&](std::size_t start) {
            if (start == size) {
                result.push_back(path);
                return;
            }
            for (std::size_t end = start; end < size; ++end) {
                if (palindrome[start][end]) {
                    path.emplace_back(text.substr(start, end - start + 1));
                    visit(end + 1);
                    path.pop_back();
                }
            }
        };
        visit(0);
        return result;
    }
};

} // namespace dsa::backtracking
