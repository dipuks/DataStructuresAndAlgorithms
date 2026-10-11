#include <climits>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dsa/linked_lists.hpp"
#include "dsa/stack.hpp"
#include "dsa/strings.hpp"

namespace {

std::size_t checks = 0;

void check(bool condition, std::string_view message)
{
    ++checks;
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

template <typename Exception, typename Function>
void checkThrows(Function&& function, std::string_view message)
{
    ++checks;
    try {
        function();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(std::string(message));
}

using dsa::linked_lists::LinkedLists;
using dsa::linked_lists::ListNode;
using dsa::linked_lists::RandomNode;
using dsa::stack::Stack;
using dsa::strings::DecodeError;
using dsa::strings::Strings;
using Ints = std::vector<int>;
using Views = std::vector<std::string_view>;
using Strs = std::vector<std::string>;

// RAII owners so the tests never leak, even when a check throws.
struct ListGuard {
    ListNode* head;
    explicit ListGuard(ListNode* h) : head(h) {}
    explicit ListGuard(const Ints& values) : head(LinkedLists::fromValues(values)) {}
    ListGuard(const ListGuard&) = delete;
    ListGuard& operator=(const ListGuard&) = delete;
    ~ListGuard() { LinkedLists::destroy(head); }
};

struct RandomGuard {
    RandomNode* head;
    explicit RandomGuard(RandomNode* h) : head(h) {}
    RandomGuard(const RandomGuard&) = delete;
    RandomGuard& operator=(const RandomGuard&) = delete;
    ~RandomGuard() { LinkedLists::destroyRandomList(head); }
};

void testStack()
{
    check(Stack::isValidParentheses(""), "54 empty");
    check(Stack::isValidParentheses("()[]{}"), "54 simple");
    check(Stack::isValidParentheses("{[()]}"), "54 nested");
    check(!Stack::isValidParentheses("(]"), "54 mismatch");
    check(!Stack::isValidParentheses("(("), "54 unclosed");
    check(!Stack::isValidParentheses("))"), "54 unopened");
    check(!Stack::isValidParentheses("(a)"), "54 foreign char");

    Stack::MinStack ms;
    check(ms.empty() && !ms.top() && !ms.getMin() && !ms.pop(), "55 empty ops");
    ms.push(5);
    ms.push(3);
    ms.push(3);
    ms.push(7);
    check(ms.size() == 4 && *ms.top() == 7 && *ms.getMin() == 3, "55 push");
    check(ms.pop() && ms.pop(), "55 pop");
    check(*ms.getMin() == 3 && *ms.top() == 3, "55 duplicate min");
    check(ms.pop(), "55 pop 3");
    check(*ms.getMin() == 5 && *ms.top() == 5, "55 min restored");
    ms.push(INT_MIN);
    check(*ms.getMin() == INT_MIN, "55 int min");

    check(Stack::dailyTemperatures(Ints{73, 74, 75, 71, 69, 72, 76, 73}) ==
              (Ints{1, 1, 4, 2, 1, 1, 0, 0}), "56 normal");
    check(Stack::dailyTemperatures(Ints{}).empty(), "56 empty");
    check(Stack::dailyTemperatures(Ints{5, 5, 5}) == (Ints{0, 0, 0}), "56 equal");

    check(Stack::carFleet(12, Ints{10, 8, 0, 5, 3}, Ints{2, 4, 1, 1, 3}) == 3, "57 normal");
    check(Stack::carFleet(10, Ints{}, Ints{}) == 0, "57 empty");
    check(Stack::carFleet(10, Ints{3}, Ints{3}) == 1, "57 single");
    check(Stack::carFleet(10, Ints{6, 6}, Ints{1, 5}) == 1, "57 same position");
    check(Stack::carFleet(100, Ints{0, 2, 4}, Ints{4, 2, 1}) == 1, "57 one fleet");
    check(Stack::carFleet(INT_MAX, Ints{0, 1}, Ints{INT_MAX, 1}) == 1, "57 large values");
    checkThrows<std::invalid_argument>([] { (void)Stack::carFleet(10, Ints{1}, Ints{}); }, "57 sizes");
    checkThrows<std::invalid_argument>([] { (void)Stack::carFleet(10, Ints{1}, Ints{0}); }, "57 speed");
    checkThrows<std::invalid_argument>([] { (void)Stack::carFleet(10, Ints{10}, Ints{1}); }, "57 position");

    check(Stack::largestRectangleArea(Ints{2, 1, 5, 6, 2, 3}) == 10, "58 normal");
    check(Stack::largestRectangleArea(Ints{}) == 0, "58 empty");
    check(Stack::largestRectangleArea(Ints{0, 0}) == 0, "58 zeros");
    check(Stack::largestRectangleArea(Ints{INT_MAX, INT_MAX}) == 2LL * INT_MAX, "58 wide");
    checkThrows<std::invalid_argument>([] { (void)Stack::largestRectangleArea(Ints{-1}); }, "58 negative");

    check(Stack::maximalRectangle(Strs{"10100", "10111", "11111", "10010"}) == 6, "59 normal");
    check(Stack::maximalRectangle(Strs{}) == 0, "59 empty");
    check(Stack::maximalRectangle(Strs{""}) == 0, "59 empty row");
    check(Stack::maximalRectangle(Strs{"0"}) == 0, "59 zero");
    check(Stack::maximalRectangle(Strs{"1"}) == 1, "59 one");
    checkThrows<std::invalid_argument>([] { (void)Stack::maximalRectangle(Strs{"1", "11"}); }, "59 ragged");
    checkThrows<std::invalid_argument>([] { (void)Stack::maximalRectangle(Strs{"1x"}); }, "59 char");

    check(Stack::decodeString("3[a]2[bc]") == "aaabcbc", "60 normal");
    check(Stack::decodeString("3[a2[c]]") == "accaccacc", "60 nested");
    check(Stack::decodeString("2[abc]3[cd]ef") == "abcabccdcdcdef", "60 mixed");
    check(Stack::decodeString("").empty(), "60 empty");
    check(Stack::decodeString("10[a]") == "aaaaaaaaaa", "60 multi-digit");
    check(Stack::decodeString("2[]x") == "x", "60 empty body");
    for (const char* bad : {"3[a", "a]", "[a]", "3", "0[a]", "3a", "2[a$]"}) {
        checkThrows<std::invalid_argument>([&] { (void)Stack::decodeString(bad); }, "60 malformed");
    }
    checkThrows<std::length_error>([] { (void)Stack::decodeString("99999999999999999999[a]"); }, "60 count overflow");
    checkThrows<std::length_error>([] { (void)Stack::decodeString("100[100[a]]", 1000); }, "60 too long");
    checkThrows<std::length_error>([] { (void)Stack::decodeString("5[a]", 0); }, "60 zero max");

    check(Stack::removeKDigits("1432219", 3) == "1219", "61 normal");
    check(Stack::removeKDigits("10200", 1) == "200", "61 leading zeros");
    check(Stack::removeKDigits("10", 2) == "0", "61 all");
    check(Stack::removeKDigits("", 0) == "0", "61 empty");
    check(Stack::removeKDigits("12345", 2) == "123", "61 ascending");
    check(Stack::removeKDigits("100", 1) == "0", "61 zero result");
    check(Stack::removeKDigits("9", 0) == "9", "61 k zero");
    checkThrows<std::invalid_argument>([] { (void)Stack::removeKDigits("1a", 1); }, "61 non digit");

    check(Stack::asteroidCollision(Ints{5, 10, -5}) == (Ints{5, 10}), "62 normal");
    check(Stack::asteroidCollision(Ints{8, -8}).empty(), "62 equal");
    check(Stack::asteroidCollision(Ints{10, 2, -5}) == (Ints{10}), "62 chain");
    check(Stack::asteroidCollision(Ints{-2, -1, 1, 2}) == (Ints{-2, -1, 1, 2}), "62 none");
    check(Stack::asteroidCollision(Ints{1, -INT_MAX, INT_MIN}) == (Ints{-INT_MAX, INT_MIN}), "62 int min");
    check(Stack::asteroidCollision(Ints{}).empty(), "62 empty");
    checkThrows<std::invalid_argument>([] { (void)Stack::asteroidCollision(Ints{0}); }, "62 zero");

    check(Stack::calculate("3+2*2") == 7, "63 precedence");
    check(Stack::calculate(" 3/2 ") == 1, "63 division");
    check(Stack::calculate(" 3+5 / 2 ") == 5, "63 spaces");
    check(Stack::calculate("14-3/2") == 13, "63 minus");
    check(Stack::calculate("0-2*3") == -6, "63 negative");
    check(Stack::calculate("1-2/3*4") == 1, "63 truncation");
    check(Stack::calculate("42") == 42, "63 single");
    check(Stack::calculate("9223372036854775807") == LLONG_MAX, "63 max literal");
    check(Stack::calculate("0-9223372036854775807-1") == LLONG_MIN, "63 min result");
    checkThrows<std::overflow_error>([] { (void)Stack::calculate("9223372036854775808"); }, "63 literal overflow");
    checkThrows<std::overflow_error>([] { (void)Stack::calculate("9223372036854775807+1"); }, "63 add overflow");
    checkThrows<std::overflow_error>([] { (void)Stack::calculate("4294967296*4294967296"); }, "63 mul overflow");
    checkThrows<std::overflow_error>([] { (void)Stack::calculate("0-9223372036854775807-2"); }, "63 sub overflow");
    checkThrows<std::domain_error>([] { (void)Stack::calculate("1/0"); }, "63 div zero");
    for (const char* bad : {"", "   ", "+1", "1+", "1++2", "1 2", "(1)", "1+a", "*2"}) {
        checkThrows<std::invalid_argument>([&] { (void)Stack::calculate(bad); }, "63 malformed");
    }
}

void testStringsFirstHalf()
{
    check(Strings::longestPalindrome("babad") == "bab", "64 odd leftmost");
    check(Strings::longestPalindrome("cbbd") == "bb", "64 even");
    check(Strings::longestPalindrome("").empty(), "64 empty");
    check(Strings::longestPalindrome("a") == "a", "64 single");
    check(Strings::longestPalindrome("abc") == "a", "64 none");
    check(Strings::longestPalindrome("aaaa") == "aaaa", "64 all same");

    check(Strings::countPalindromicSubstrings("abc") == 3, "65 distinct");
    check(Strings::countPalindromicSubstrings("aaa") == 6, "65 same");
    check(Strings::countPalindromicSubstrings("") == 0, "65 empty");
    check(Strings::countPalindromicSubstrings("abba") == 6, "65 even");

    check(Strings::atoi("42") == 42, "66 plain");
    check(Strings::atoi("   -042") == -42, "66 space sign zeros");
    check(Strings::atoi("1337c0d3") == 1337, "66 stops");
    check(Strings::atoi("0-1") == 0, "66 sign after digit");
    check(Strings::atoi("words and 987") == 0, "66 no digits");
    check(Strings::atoi("+-12") == 0, "66 double sign");
    check(Strings::atoi("") == 0, "66 empty");
    check(Strings::atoi("2147483647") == INT_MAX, "66 max");
    check(Strings::atoi("-2147483648") == INT_MIN, "66 min");
    check(Strings::atoi("2147483648") == INT_MAX, "66 clamp high");
    check(Strings::atoi("-91283472332") == INT_MIN, "66 clamp low");
    check(Strings::atoi("99999999999999999999999") == INT_MAX, "66 huge");

    check(Strings::simplifyPath("/home/") == "/home", "67 trailing");
    check(Strings::simplifyPath("/../") == "/", "67 above root");
    check(Strings::simplifyPath("/home//foo/") == "/home/foo", "67 double slash");
    check(Strings::simplifyPath("/a/./b/../../c/") == "/c", "67 mixed");
    check(Strings::simplifyPath("/.../a") == "/.../a", "67 dots name");
    check(Strings::simplifyPath("/") == "/", "67 root");
    checkThrows<std::invalid_argument>([] { (void)Strings::simplifyPath("a/b"); }, "67 relative");
    checkThrows<std::invalid_argument>([] { (void)Strings::simplifyPath(""); }, "67 empty");

    check(Strings::reverseWords("the sky is blue") == "blue is sky the", "68 normal");
    check(Strings::reverseWords("  hello   world  ") == "world hello", "68 spaces");
    check(Strings::reverseWords("").empty(), "68 empty");
    check(Strings::reverseWords("   ").empty(), "68 only spaces");
    check(Strings::reverseWords("a") == "a", "68 single");
    check(Strings::reverseWords("a\tb\nc") == "c b a", "68 whitespace kinds");

    check(Strings::compareVersion("1.01", "1.001") == 0, "69 leading zeros");
    check(Strings::compareVersion("1.0", "1.0.0") == 0, "69 padding");
    check(Strings::compareVersion("0.1", "1.1") == -1, "69 less");
    check(Strings::compareVersion("1.0.1", "1") == 1, "69 greater");
    check(Strings::compareVersion("7.5.2.4", "7.5.3") == -1, "69 mid");
    check(Strings::compareVersion("1", "1.0.0.1") == -1, "69 trailing nonzero");
    check(Strings::compareVersion("99999999999999999999999.1", "99999999999999999999998") == 1, "69 big");
    for (const char* bad : {"", "1..2", "1.", ".1", "a"}) {
        checkThrows<std::invalid_argument>([&] { (void)Strings::compareVersion(bad, "1"); }, "69 malformed");
    }

    check(Strings::multiply("2", "3") == "6", "70 small");
    check(Strings::multiply("123", "456") == "56088", "70 normal");
    check(Strings::multiply("0", "999") == "0", "70 zero");
    check(Strings::multiply("00", "12") == "0", "70 zero padded");
    check(Strings::multiply("999", "999") == "998001", "70 carries");
    check(Strings::multiply("99999999999999999999", "99999999999999999999") ==
              "9999999999999999999800000000000000000001", "70 big");
    checkThrows<std::invalid_argument>([] { (void)Strings::multiply("", "1"); }, "70 empty");
    checkThrows<std::invalid_argument>([] { (void)Strings::multiply("1", "1a"); }, "70 non digit");

    check(Strings::addStrings("11", "123") == "134", "71 normal");
    check(Strings::addStrings("456", "77") == "533", "71 lengths");
    check(Strings::addStrings("0", "0") == "0", "71 zero");
    check(Strings::addStrings("999", "1") == "1000", "71 carry out");
    check(Strings::addStrings("007", "3") == "10", "71 leading zeros");
    check(Strings::addStrings("00", "0") == "0", "71 zero padded");
    checkThrows<std::invalid_argument>([] { (void)Strings::addStrings("", "1"); }, "71 empty");
    checkThrows<std::invalid_argument>([] { (void)Strings::addStrings("-1", "1"); }, "71 sign");
}

void testStringsSecondHalf()
{
    check(Strings::minRemoveToMakeValid("lee(t(c)o)de)") == "lee(t(c)o)de", "72 trailing close");
    check(Strings::minRemoveToMakeValid("a)b(c)d") == "ab(c)d", "72 leading close");
    check(Strings::minRemoveToMakeValid("))((").empty(), "72 all bad");
    check(Strings::minRemoveToMakeValid("").empty(), "72 empty");
    check(Strings::minRemoveToMakeValid("abc") == "abc", "72 no parens");
    check(Strings::minRemoveToMakeValid("(a(b(c)d)") == "a(b(c)d)", "72 extra open");

    check(Strings::isOneEditDistance("ab", "acb"), "73 insert");
    check(Strings::isOneEditDistance("cat", "cut"), "73 replace");
    check(Strings::isOneEditDistance("cat", "cast"), "73 insert middle");
    check(Strings::isOneEditDistance("cat", "at"), "73 delete");
    check(Strings::isOneEditDistance("a", ""), "73 to empty");
    check(Strings::isOneEditDistance("", "A"), "73 from empty");
    check(!Strings::isOneEditDistance("", ""), "73 both empty");
    check(!Strings::isOneEditDistance("ab", "ab"), "73 identical");
    check(!Strings::isOneEditDistance("cat", "dog"), "73 far");
    check(!Strings::isOneEditDistance("a", "abc"), "73 length gap");
    check(!Strings::isOneEditDistance("ab", "ba"), "73 swap");

    check(Strings::longestCommonPrefix(Views{"flower", "flow", "flight"}) == "fl", "74 normal");
    check(Strings::longestCommonPrefix(Views{"dog", "racecar", "car"}).empty(), "74 none");
    check(Strings::longestCommonPrefix(Views{}).empty(), "74 empty list");
    check(Strings::longestCommonPrefix(Views{"abc"}) == "abc", "74 single");
    check(Strings::longestCommonPrefix(Views{"", "a"}).empty(), "74 empty word");
    check(Strings::longestCommonPrefix(Views{"a", "a"}) == "a", "74 equal");

    const auto groups = Strings::groupShiftedStrings(
        Views{"abc", "bcd", "acef", "xyz", "az", "ba", "a", "z"});
    check(groups == (std::vector<Strs>{{"abc", "bcd", "xyz"}, {"acef"}, {"az", "ba"}, {"a", "z"}}), "75 normal");
    check(Strings::groupShiftedStrings(Views{}).empty(), "75 empty list");
    check(Strings::groupShiftedStrings(Views{"", ""}) == (std::vector<Strs>{{"", ""}}), "75 empty strings");
    checkThrows<std::invalid_argument>([] { (void)Strings::groupShiftedStrings(Views{"A"}); }, "75 uppercase");

    const std::string_view nul("\0", 1);
    const Views tricky{"a", "", "b:c", "12:x", nul, "3:abc", ""};
    const std::string encoded = Strings::encode(tricky);
    const auto decoded = Strings::decode(encoded);
    check(decoded.has_value() && *decoded == (Strs{"a", "", "b:c", "12:x", std::string("\0", 1), "3:abc", ""}),
          "76 round trip");
    check(Strings::encode(Views{}).empty(), "76 empty list encodes empty");
    check(Strings::decode("").has_value() && Strings::decode("").value().empty(), "76 empty decode");
    check(Strings::encode(Views{""}) == "0:", "76 one empty string");
    check(Strings::decode("0:").value() == (Strs{""}), "76 decode one empty");
    check(Strings::encode(Views{"a", ""}) == "1:a0:", "76 format");
    check(Strings::decode(Strings::encode(Views{"", ""})).value() == (Strs{"", ""}), "76 two empties");
    const auto expectError = [](std::string_view input, DecodeError error, std::string_view message) {
        const auto result = Strings::decode(input);
        check(!result.has_value() && result.error() == error, message);
    };
    expectError("x", DecodeError::MissingLength, "76 missing length");
    expectError(":", DecodeError::MissingLength, "76 colon first");
    expectError("01:a", DecodeError::NonCanonicalLength, "76 leading zero");
    expectError("99999999999999999999999:", DecodeError::LengthOverflow, "76 overflow");
    expectError("3", DecodeError::MissingSeparator, "76 no colon at end");
    expectError("3abc", DecodeError::MissingSeparator, "76 wrong separator");
    expectError("1:a2", DecodeError::MissingSeparator, "76 trailing digits");
    expectError("5:abc", DecodeError::Truncated, "76 truncated");
    expectError("18446744073709551615:a", DecodeError::Truncated, "76 max length truncated");

    check(Strings::checkValidString("()"), "77 simple");
    check(Strings::checkValidString("(*)"), "77 star empty");
    check(Strings::checkValidString("(*))"), "77 star open");
    check(Strings::checkValidString("*"), "77 lone star");
    check(Strings::checkValidString(""), "77 empty");
    check(Strings::checkValidString("(*"), "77 star close");
    check(!Strings::checkValidString(")("), "77 wrong order");
    check(!Strings::checkValidString("(("), "77 unclosed");
    check(!Strings::checkValidString("*)("), "77 star cannot fix order");
    checkThrows<std::invalid_argument>([] { (void)Strings::checkValidString("(a)"); }, "77 foreign char");
}

void testLinkedLists()
{
    const auto roundTrip = Ints{1, 2, 3};
    ListGuard original(roundTrip);
    check(LinkedLists::toVector(original.head) == roundTrip, "fromValues/toVector");
    check(LinkedLists::fromValues(Ints{}) == nullptr && LinkedLists::toVector(nullptr).empty(), "empty list");
    LinkedLists::destroy(nullptr);

    {
        ListGuard list(Ints{1, 2, 3, 4});
        list.head = LinkedLists::reverseList(list.head);
        check(LinkedLists::toVector(list.head) == (Ints{4, 3, 2, 1}), "78 normal");
        ListGuard one(Ints{9});
        one.head = LinkedLists::reverseList(one.head);
        check(LinkedLists::toVector(one.head) == (Ints{9}), "78 single");
        check(LinkedLists::reverseList(nullptr) == nullptr, "78 null");
    }

    {
        ListGuard a(Ints{1, 3, 5});
        ListGuard b(Ints{2, 3, 6, 7});
        ListGuard merged(LinkedLists::mergeTwoLists(a.head, b.head));
        a.head = nullptr;
        b.head = nullptr;
        check(LinkedLists::toVector(merged.head) == (Ints{1, 2, 3, 3, 5, 6, 7}), "79 normal");
        ListGuard x(Ints{1, 2});
        ListGuard onlyFirst(LinkedLists::mergeTwoLists(x.head, nullptr));
        x.head = nullptr;
        check(LinkedLists::toVector(onlyFirst.head) == (Ints{1, 2}), "79 second null");
        ListGuard y(Ints{4});
        ListGuard onlySecond(LinkedLists::mergeTwoLists(nullptr, y.head));
        y.head = nullptr;
        check(LinkedLists::toVector(onlySecond.head) == (Ints{4}), "79 first null");
        check(LinkedLists::mergeTwoLists(nullptr, nullptr) == nullptr, "79 both null");
        // Stability: equal values keep `first` nodes ahead of `second` nodes.
        ListGuard s1(Ints{1});
        ListGuard s2(Ints{1});
        ListNode* const firstNode = s1.head;
        ListGuard stable(LinkedLists::mergeTwoLists(s1.head, s2.head));
        s1.head = nullptr;
        s2.head = nullptr;
        check(stable.head == firstNode, "79 stable");
    }

    {
        const std::vector<std::pair<Ints, Ints>> cases{
            {{1, 2, 3, 4}, {1, 4, 2, 3}}, {{1, 2, 3, 4, 5}, {1, 5, 2, 4, 3}},
            {{1, 2}, {1, 2}}, {{1}, {1}}};
        for (const auto& [input, expected] : cases) {
            ListGuard list(input);
            ListNode* const head = list.head;
            LinkedLists::reorderList(list.head);
            check(list.head == head && LinkedLists::toVector(list.head) == expected, "80 reorder");
        }
        LinkedLists::reorderList(nullptr);
        ++checks;
    }

    {
        ListGuard list(Ints{1, 2, 3, 4, 5});
        list.head = LinkedLists::removeNthFromEnd(list.head, 2);
        check(LinkedLists::toVector(list.head) == (Ints{1, 2, 3, 5}), "81 middle");
        list.head = LinkedLists::removeNthFromEnd(list.head, 1);
        check(LinkedLists::toVector(list.head) == (Ints{1, 2, 3}), "81 tail");
        list.head = LinkedLists::removeNthFromEnd(list.head, 3);
        check(LinkedLists::toVector(list.head) == (Ints{2, 3}), "81 head");
        checkThrows<std::out_of_range>([&] { list.head = LinkedLists::removeNthFromEnd(list.head, 0); }, "81 zero");
        checkThrows<std::out_of_range>([&] { list.head = LinkedLists::removeNthFromEnd(list.head, 3); }, "81 too large");
        check(LinkedLists::toVector(list.head) == (Ints{2, 3}), "81 unchanged after error");
        ListGuard single(Ints{1});
        single.head = LinkedLists::removeNthFromEnd(single.head, 1);
        check(single.head == nullptr, "81 single");
        checkThrows<std::out_of_range>([] { (void)LinkedLists::removeNthFromEnd(nullptr, 1); }, "81 null");
    }

    {
        ListGuard list(Ints{1, 2, 3, 4});
        ListNode* const second = list.head->next;
        ListNode* const tail = list.head->next->next->next;
        check(!LinkedLists::hasCycle(list.head) && LinkedLists::findCycleStart(list.head) == nullptr, "82 acyclic");
        tail->next = second;
        const bool cyclic = LinkedLists::hasCycle(list.head);
        const ListNode* const start = LinkedLists::findCycleStart(list.head);
        tail->next = nullptr; // restore before the guard destroys the list
        check(cyclic && start == second, "82 cycle start");
        ListGuard self(Ints{1});
        self.head->next = self.head;
        const bool selfCycle = LinkedLists::hasCycle(self.head);
        const ListNode* const selfStart = LinkedLists::findCycleStart(self.head);
        self.head->next = nullptr;
        check(selfCycle && selfStart == self.head, "82 self loop");
        check(!LinkedLists::hasCycle(nullptr) && !LinkedLists::hasCycle(self.head), "82 null and single");
    }

    {
        const auto add = [](const Ints& a, const Ints& b) {
            ListGuard x(a);
            ListGuard y(b);
            ListGuard sum(LinkedLists::addTwoNumbers(x.head, y.head));
            check(LinkedLists::toVector(x.head) == a && LinkedLists::toVector(y.head) == b, "83 inputs untouched");
            return LinkedLists::toVector(sum.head);
        };
        check(add({2, 4, 3}, {5, 6, 4}) == (Ints{7, 0, 8}), "83 normal");
        check(add({9, 9, 9, 9}, {1}) == (Ints{0, 0, 0, 0, 1}), "83 carry chain");
        check(add({}, {}) == (Ints{0}), "83 both empty");
        check(add({0}, {0}) == (Ints{0}), "83 zeros");
        check(add({0, 0}, {0}) == (Ints{0}), "83 padded zeros");
        check(add({5}, {5}) == (Ints{0, 1}), "83 final carry");
        check(add({1}, {}) == (Ints{1}), "83 one empty");
        checkThrows<std::invalid_argument>([&] { (void)add({10}, {1}); }, "83 bad digit");
        checkThrows<std::invalid_argument>([&] { (void)add({1, 2}, {1, -1}); }, "83 bad digit later");
    }

    {
        RandomGuard source(new RandomNode(7));
        source.head->next = new RandomNode(13);
        source.head->next->next = new RandomNode(11);
        source.head->next->random = source.head;
        source.head->next->next->random = source.head->next->next;
        RandomGuard copy(LinkedLists::copyRandomList(source.head));
        const RandomNode* a = source.head;
        const RandomNode* b = copy.head;
        bool same = true;
        bool distinct = true;
        for (; a != nullptr && b != nullptr; a = a->next, b = b->next) {
            distinct = distinct && a != b;
            same = same && a->value == b->value;
        }
        check(a == nullptr && b == nullptr && same && distinct, "84 structure");
        check(copy.head->random == nullptr, "84 null random");
        check(copy.head->next->random == copy.head, "84 random inside copy");
        check(copy.head->next->next->random == copy.head->next->next, "84 self random");
        check(source.head->next->random == source.head, "84 source untouched");
        check(LinkedLists::copyRandomList(nullptr) == nullptr, "84 null");

        RandomNode outsider(1);
        RandomGuard bad(new RandomNode(2));
        bad.head->random = &outsider;
        checkThrows<std::invalid_argument>([&] { (void)LinkedLists::copyRandomList(bad.head); }, "84 outside random");
        bad.head->random = nullptr;
    }

    {
        LinkedLists::LRUCache cache(2);
        cache.put(1, 1);
        cache.put(2, 2);
        check(cache.get(1) == 1, "85 hit");
        cache.put(3, 3);
        check(!cache.get(2).has_value(), "85 evicted lru");
        cache.put(4, 4);
        check(!cache.get(1).has_value() && cache.get(3) == 3 && cache.get(4) == 4, "85 eviction order");
        cache.put(3, 30);
        cache.put(5, 5);
        check(cache.get(3) == 30 && !cache.get(4).has_value() && cache.size() == 2, "85 update refreshes");
        check(cache.capacity() == 2, "85 capacity");

        LinkedLists::LRUCache none(0);
        none.put(1, 1);
        check(!none.get(1).has_value() && none.size() == 0 && none.capacity() == 0, "85 capacity zero");

        LinkedLists::LRUCache one(1);
        one.put(1, 1);
        one.put(2, 2);
        check(!one.get(1).has_value() && one.get(2) == 2, "85 capacity one");

        LinkedLists::LRUCache moved(std::move(cache));
        check(moved.get(3) == 30 && moved.size() == 2, "85 move");
        LinkedLists::LRUCache negative(2);
        negative.put(INT_MIN, INT_MAX);
        check(negative.get(INT_MIN) == INT_MAX, "85 extreme values");
    }
}

} // namespace

int main()
{
    try {
        testStack();
        testStringsFirstHalf();
        testStringsSecondHalf();
        testLinkedLists();
    } catch (const std::exception& error) {
        std::cerr << "FAILED: " << error.what() << " (after " << checks << " checks)\n";
        return 1;
    }
    std::cout << "remaining_batch_b_tests passed (" << checks << " checks)\n";
    return 0;
}
