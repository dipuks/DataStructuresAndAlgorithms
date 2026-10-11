#pragma once

#include <algorithm>
#include <cstddef>
#include <expected>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace dsa::strings {

/// Reasons Strings::decode can reject its input (problem 76).
enum class DecodeError {
    MissingLength,      ///< a record does not start with a decimal digit
    NonCanonicalLength, ///< length has a leading zero (e.g. "01:a")
    LengthOverflow,     ///< length does not fit std::size_t
    MissingSeparator,   ///< digits are not followed by ':'
    Truncated           ///< fewer payload bytes remain than the length says
};

/// Static solutions for string interview problems (64-77).
///
/// Error model: invalid input throws std::invalid_argument unless a method
/// documents otherwise. Inputs are never modified; results are new strings.
class Strings final {
public:
    Strings() = delete;

    /// Problem 64: Longest Palindromic Substring.
    ///
    /// Returns the leftmost longest palindrome ("" for empty input), as a copy.
    /// Technique: expand around each of the 2n-1 centers.
    /// Time: O(n^2). Extra space: O(1) besides the result.
    /// Example: "babad" -> "bab" (the leftmost longest palindrome). Expand
    /// around each character and each gap between characters.
    [[nodiscard]] static std::string longestPalindrome(std::string_view text)
    {
        std::size_t bestStart = 0;
        std::size_t bestLength = 0;
        for (std::size_t center = 0; center < text.size(); ++center) {
            for (std::size_t odd = 0; odd < 2; ++odd) {
                std::size_t left = center;
                std::size_t right = center + odd;
                if (right >= text.size() || text[left] != text[right]) {
                    continue;
                }
                while (left > 0 && right + 1 < text.size() && text[left - 1] == text[right + 1]) {
                    --left;
                    ++right;
                }
                if (right - left + 1 > bestLength) {
                    bestLength = right - left + 1;
                    bestStart = left;
                }
            }
        }
        return std::string(text.substr(bestStart, bestLength));
    }

    /// Problem 65: Palindromic Substrings.
    ///
    /// Counts palindromic substrings by position (equal text at different
    /// positions counts separately). Technique: expand around each center.
    /// Time: O(n^2). Extra space: O(1).
    /// Example: "aaa" -> 6 palindromic substrings, counting positions.
    /// Every successful expansion adds one palindrome around its center.
    [[nodiscard]] static std::size_t countPalindromicSubstrings(std::string_view text)
    {
        std::size_t count = 0;
        for (std::size_t center = 0; center < text.size(); ++center) {
            for (std::size_t odd = 0; odd < 2; ++odd) {
                std::size_t left = center;
                std::size_t right = center + odd;
                while (right < text.size() && text[left] == text[right]) {
                    ++count;
                    if (left == 0) {
                        break;
                    }
                    --left;
                    ++right;
                }
            }
        }
        return count;
    }

    /// Problem 66: atoi with clamp.
    ///
    /// Skips leading ' ', reads an optional sign and then digits, stopping at
    /// the first other character. No digits yields 0. Values outside int are
    /// clamped to INT_MIN / INT_MAX; accumulation never overflows.
    /// Time: O(n). Extra space: O(1).
    /// Example: "  -42abc" -> -42. Skip leading spaces, read a sign and
    /// consume digits until the first non-digit, clamping to int bounds.
    [[nodiscard]] static int atoi(std::string_view text)
    {
        std::size_t i = 0;
        while (i < text.size() && text[i] == ' ') {
            ++i;
        }
        bool negative = false;
        if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
            negative = text[i] == '-';
            ++i;
        }
        constexpr long long limit = static_cast<long long>(std::numeric_limits<int>::max()) + 1;
        long long value = 0; // magnitude, capped at limit
        for (; i < text.size() && text[i] >= '0' && text[i] <= '9'; ++i) {
            value = std::min(limit, value * 10 + (text[i] - '0'));
        }
        if (negative) {
            return static_cast<int>(-value);
        }
        return value >= limit ? std::numeric_limits<int>::max() : static_cast<int>(value);
    }

    /// Problem 67: Simplify Path (Unix style).
    ///
    /// path must be absolute (start with '/'), else std::invalid_argument.
    /// Resolves ".", ".." (stays at root) and repeated slashes; no trailing
    /// slash except for "/". Names like "..." are ordinary names. Symlinks
    /// are not consulted. Technique: stack of components.
    /// Time: O(n). Extra space: O(n).
    /// Example: "/home//foo/../bar/" -> "/home/bar". Ignore empty and "."
    /// segments; ".." removes one accumulated path component when possible.
    [[nodiscard]] static std::string simplifyPath(std::string_view path)
    {
        if (path.empty() || path.front() != '/') {
            throw std::invalid_argument("path must be absolute");
        }
        std::vector<std::string_view> parts;
        std::size_t i = 0;
        while (i < path.size()) {
            while (i < path.size() && path[i] == '/') {
                ++i;
            }
            const std::size_t start = i;
            while (i < path.size() && path[i] != '/') {
                ++i;
            }
            const std::string_view part = path.substr(start, i - start);
            if (part.empty() || part == ".") {
                continue;
            }
            if (part == "..") {
                if (!parts.empty()) {
                    parts.pop_back();
                }
            } else {
                parts.push_back(part);
            }
        }
        std::string result;
        for (const std::string_view part : parts) {
            result.push_back('/');
            result.append(part);
        }
        return result.empty() ? "/" : result;
    }

    /// Problem 68: Reverse Words in a String.
    ///
    /// Words are runs of non-whitespace (space, \t, \n, \r, \f, \v); output
    /// joins reversed words with single spaces, no leading/trailing spaces.
    /// Time: O(n). Extra space: O(n) for the result.
    /// Example: "  the sky is blue  " -> "blue is sky the". Scan from the
    /// end, appending each non-whitespace run with one separator.
    [[nodiscard]] static std::string reverseWords(std::string_view text)
    {
        auto isSpace = [](char c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
        };
        std::string result;
        std::size_t end = text.size();
        while (end > 0) {
            while (end > 0 && isSpace(text[end - 1])) {
                --end;
            }
            std::size_t start = end;
            while (start > 0 && !isSpace(text[start - 1])) {
                --start;
            }
            if (start < end) {
                if (!result.empty()) {
                    result.push_back(' ');
                }
                result.append(text.substr(start, end - start));
            }
            end = start;
        }
        return result;
    }

    /// Problem 69: Compare Version Numbers.
    ///
    /// Versions are '.'-separated non-empty digit groups; missing groups count
    /// as 0 and leading zeros are ignored. Groups are compared as decimal
    /// strings so arbitrarily large groups cannot overflow. Malformed
    /// versions (empty, empty group, non-digit) throw std::invalid_argument.
    /// Returns -1, 0 or 1. Time: O(n + m). Extra space: O(1).
    /// Example: "1.01" compared with "1.001" -> 0. Normalize each numeric
    /// group by removing leading zeroes, then compare length and digits.
    [[nodiscard]] static int compareVersion(std::string_view first, std::string_view second)
    {
        validateVersion(first);
        validateVersion(second);
        std::size_t i = 0;
        std::size_t j = 0;
        while (i <= first.size() || j <= second.size()) {
            const std::string_view a = nextVersionGroup(first, i);
            const std::string_view b = nextVersionGroup(second, j);
            if (a.size() != b.size()) {
                return a.size() < b.size() ? -1 : 1;
            }
            if (const int cmp = a.compare(b); cmp != 0) {
                return cmp < 0 ? -1 : 1;
            }
        }
        return 0;
    }

    /// Problem 70: Multiply Strings (non-negative decimal integers).
    ///
    /// Both operands must be non-empty digit strings (leading zeros allowed),
    /// else std::invalid_argument. Result has no leading zeros ("0" for zero).
    /// Technique: schoolbook multiplication into a digit array.
    /// Time: O(n*m). Extra space: O(n+m).
    /// Example: "123" * "456" -> "56088". Multiply digit pairs into shifted
    /// result cells and propagate each row's carry.
    [[nodiscard]] static std::string multiply(std::string_view first, std::string_view second)
    {
        validateDigits(first);
        validateDigits(second);
        std::vector<int> cells(first.size() + second.size(), 0);
        for (std::size_t i = first.size(); i-- > 0;) {
            int carry = 0;
            for (std::size_t j = second.size(); j-- > 0;) {
                const int total = cells[i + j + 1] + (first[i] - '0') * (second[j] - '0') + carry;
                cells[i + j + 1] = total % 10;
                carry = total / 10;
            }
            cells[i] += carry;
        }
        std::string result;
        for (const int digit : cells) {
            if (!result.empty() || digit != 0) {
                result.push_back(static_cast<char>('0' + digit));
            }
        }
        return result.empty() ? "0" : result;
    }

    /// Problem 71: Add Strings (non-negative decimal integers).
    ///
    /// Same validation as multiply. Result has no leading zeros.
    /// Time: O(max(n,m)). Extra space: O(max(n,m)).
    /// Example: "11" + "123" -> "134". Add from least-significant digits,
    /// keep the carry, then reverse the collected result digits.
    [[nodiscard]] static std::string addStrings(std::string_view first, std::string_view second)
    {
        validateDigits(first);
        validateDigits(second);
        std::string reversed;
        int carry = 0;
        std::size_t i = first.size();
        std::size_t j = second.size();
        while (i > 0 || j > 0 || carry > 0) {
            int total = carry;
            if (i > 0) {
                total += first[--i] - '0';
            }
            if (j > 0) {
                total += second[--j] - '0';
            }
            reversed.push_back(static_cast<char>('0' + total % 10));
            carry = total / 10;
        }
        while (reversed.size() > 1 && reversed.back() == '0') {
            reversed.pop_back();
        }
        std::ranges::reverse(reversed);
        return reversed.empty() ? "0" : reversed;
    }

    /// Problem 72: Minimum Remove to Make Valid Parentheses.
    ///
    /// Removes the fewest '(' / ')' so the rest is balanced; other characters
    /// are kept. Of several answers, the one that drops unmatched ')' as soon
    /// as seen and the latest-unmatched '(' is returned (deterministic).
    /// Technique: stack of open indices, mark unmatched for deletion.
    /// Time: O(n). Extra space: O(n).
    /// Example: "lee(t(c)o)de)" -> "lee(t(c)o)de". Mark unmatched closing
    /// positions while scanning, then discard any opening positions left over.
    [[nodiscard]] static std::string minRemoveToMakeValid(std::string_view text)
    {
        std::vector<bool> removed(text.size(), false);
        std::vector<std::size_t> opens;
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (text[i] == '(') {
                opens.push_back(i);
            } else if (text[i] == ')') {
                if (opens.empty()) {
                    removed[i] = true;
                } else {
                    opens.pop_back();
                }
            }
        }
        for (const std::size_t index : opens) {
            removed[index] = true;
        }
        std::string result;
        result.reserve(text.size());
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (!removed[i]) {
                result.push_back(text[i]);
            }
        }
        return result;
    }

    /// Problem 73: One Edit Distance.
    ///
    /// True iff first can become second by exactly one insert, delete or
    /// replace of a byte (identical strings are false).
    /// Time: O(n). Extra space: O(1).
    /// Example: "ab" and "acb" -> true (one insertion). After the common
    /// prefix, allow one skipped byte in the longer string.
    [[nodiscard]] static bool isOneEditDistance(std::string_view first, std::string_view second)
    {
        if (first.size() > second.size()) {
            std::swap(first, second);
        }
        if (second.size() - first.size() > 1) {
            return false;
        }
        std::size_t i = 0;
        while (i < first.size() && first[i] == second[i]) {
            ++i;
        }
        if (first.size() == second.size()) {
            return i < first.size() && first.substr(i + 1) == second.substr(i + 1);
        }
        return first.substr(i) == second.substr(i + 1);
    }

    /// Problem 74: Longest Common Prefix.
    ///
    /// Returns "" for an empty list. A single string is its own prefix.
    /// Technique: shrink the prefix against each string.
    /// Time: O(total characters). Extra space: O(1) besides the result.
    /// Example: ["flower","flow","flight"] -> "fl". Shrink the candidate
    /// prefix against each next word until it matches.
    [[nodiscard]] static std::string longestCommonPrefix(std::span<const std::string_view> words)
    {
        if (words.empty()) {
            return {};
        }
        std::string_view prefix = words.front();
        for (const std::string_view word : words.subspan(1)) {
            std::size_t common = 0;
            while (common < prefix.size() && common < word.size() && prefix[common] == word[common]) {
                ++common;
            }
            prefix = prefix.substr(0, common);
        }
        return std::string(prefix);
    }

    /// Problem 75: Group Shifted Strings.
    ///
    /// Strings of lowercase 'a'-'z' belong to one group if one is a cyclic
    /// alphabet shift of the other (same length, same pairwise differences
    /// mod 26). Other characters throw std::invalid_argument. Groups appear in
    /// order of first member; members keep input order. "" forms its own group.
    /// Technique: hash by normalized difference signature.
    /// Time: O(total characters) average. Extra space: O(total characters).
    /// Example: ["abc","bcd","acef","xyz"] groups "abc","bcd","xyz"
    /// together because each has the same normalized cyclic differences.
    [[nodiscard]] static std::vector<std::vector<std::string>>
    groupShiftedStrings(std::span<const std::string_view> words)
    {
        std::vector<std::vector<std::string>> groups;
        std::unordered_map<std::string, std::size_t> groupIndex;
        for (const std::string_view word : words) {
            std::string signature;
            signature.reserve(word.size());
            for (const char c : word) {
                if (c < 'a' || c > 'z') {
                    throw std::invalid_argument("only lowercase letters are supported");
                }
                signature.push_back(static_cast<char>((c - word.front() + 26) % 26));
            }
            const auto [it, inserted] = groupIndex.try_emplace(signature, groups.size());
            if (inserted) {
                groups.emplace_back();
            }
            groups[it->second].emplace_back(word);
        }
        return groups;
    }

    /// Problem 76 (encode): length-prefixed encoding of a list of strings.
    ///
    /// Each string becomes "<decimal byte length>:<raw bytes>". Because the
    /// length is read first and exactly that many bytes are consumed, strings
    /// may contain any bytes (':', digits, NUL, ...) and may be empty; the
    /// empty list encodes to "". The encoding is canonical and injective.
    /// Time: O(total bytes). Extra space: O(total bytes).
    /// Example: ["cat",""] -> "3:cat0:". Length-prefixing lets the decoder
    /// distinguish payload delimiters and empty strings without escaping.
    [[nodiscard]] static std::string encode(std::span<const std::string_view> words)
    {
        std::string encoded;
        for (const std::string_view word : words) {
            encoded += std::to_string(word.size());
            encoded.push_back(':');
            encoded.append(word);
        }
        return encoded;
    }

    /// Problem 76 (decode): inverse of encode with explicit error reporting.
    ///
    /// Returns the strings, or a DecodeError describing the first malformed
    /// record; there is never a silent fallback or partial result.
    /// Only canonical encode output is accepted (no leading zeros in lengths).
    /// Time: O(input bytes). Extra space: O(input bytes).
    /// Example: encoded = "3:cat0:" -> ["cat",""]. Read a canonical byte
    /// length, require ':', then consume exactly that many payload bytes.
    [[nodiscard]] static std::expected<std::vector<std::string>, DecodeError>
    decode(std::string_view encoded)
    {
        std::vector<std::string> words;
        std::size_t i = 0;
        while (i < encoded.size()) {
            if (encoded[i] < '0' || encoded[i] > '9') {
                return std::unexpected(DecodeError::MissingLength);
            }
            const std::size_t digitsStart = i;
            std::size_t length = 0;
            while (i < encoded.size() && encoded[i] >= '0' && encoded[i] <= '9') {
                const std::size_t digit = static_cast<std::size_t>(encoded[i] - '0');
                if (length > (std::numeric_limits<std::size_t>::max() - digit) / 10) {
                    return std::unexpected(DecodeError::LengthOverflow);
                }
                length = length * 10 + digit;
                ++i;
            }
            if (i - digitsStart > 1 && encoded[digitsStart] == '0') {
                return std::unexpected(DecodeError::NonCanonicalLength);
            }
            if (i >= encoded.size() || encoded[i] != ':') {
                return std::unexpected(DecodeError::MissingSeparator);
            }
            ++i;
            if (encoded.size() - i < length) {
                return std::unexpected(DecodeError::Truncated);
            }
            words.emplace_back(encoded.substr(i, length));
            i += length;
        }
        return words;
    }

    /// Problem 77: Valid Parenthesis String where '*' may be '(' , ')' or empty.
    ///
    /// Characters other than '(', ')', '*' throw std::invalid_argument.
    /// Technique: track the range [low, high] of possible open counts.
    /// Time: O(n). Extra space: O(1).
    /// Example: "(*))" -> true. Track the minimum and maximum possible open
    /// counts, treating '*' as open, close, or empty as needed.
    [[nodiscard]] static bool checkValidString(std::string_view text)
    {
        long long low = 0;
        long long high = 0;
        for (const char c : text) {
            if (c == '(') {
                ++low;
                ++high;
            } else if (c == ')') {
                --low;
                --high;
            } else if (c == '*') {
                --low;
                ++high;
            } else {
                throw std::invalid_argument("unexpected character");
            }
            if (high < 0) {
                return false;
            }
            low = std::max(low, 0LL);
        }
        return low == 0;
    }

private:
    static void validateDigits(std::string_view text)
    {
        if (text.empty()) {
            throw std::invalid_argument("number must not be empty");
        }
        for (const char c : text) {
            if (c < '0' || c > '9') {
                throw std::invalid_argument("number must contain only digits");
            }
        }
    }

    static void validateVersion(std::string_view version)
    {
        std::size_t groupLength = 0;
        for (const char c : version) {
            if (c == '.') {
                if (groupLength == 0) {
                    throw std::invalid_argument("empty version group");
                }
                groupLength = 0;
            } else if (c >= '0' && c <= '9') {
                ++groupLength;
            } else {
                throw std::invalid_argument("invalid version character");
            }
        }
        if (groupLength == 0) {
            throw std::invalid_argument("empty version group");
        }
    }

    // Returns the next group without leading zeros ("" means zero) and moves
    // position past the '.'; once exhausted it yields zero groups.
    static std::string_view nextVersionGroup(std::string_view version, std::size_t& position)
    {
        if (position > version.size()) {
            return {};
        }
        std::size_t end = version.find('.', position);
        if (end == std::string_view::npos) {
            end = version.size();
        }
        std::string_view group = version.substr(position, end - position);
        position = end + 1;
        const std::size_t firstNonZero = group.find_first_not_of('0');
        return firstNonZero == std::string_view::npos ? std::string_view{} : group.substr(firstNonZero);
    }
};

} // namespace dsa::strings