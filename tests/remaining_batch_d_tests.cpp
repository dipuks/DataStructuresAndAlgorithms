#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dsa/backtracking.hpp"
#include "dsa/dynamic_programming.hpp"
#include "dsa/greedy.hpp"

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

void testBacktracking()
{
    using dsa::backtracking::Backtracking;

    constexpr std::array numbers{1, 2};
    const std::array<std::string, 0> emptyStrings{};
    const auto allSubsets = Backtracking::subsets(numbers);
    check(allSubsets.size() == 4, "subsets returns all subsets");
    check(std::ranges::find(allSubsets, std::vector<int>{1, 2}) !=
              allSubsets.end(),
          "subsets preserves values in a chosen subset");
    constexpr std::array<int, 0> empty{};
    check(Backtracking::subsets(empty) ==
              std::vector<std::vector<int>>{{}},
          "subsets of empty input contains the empty subset");

    constexpr std::array duplicateValues{1, 2, 2};
    const auto uniqueSubsets =
        Backtracking::subsetsWithDuplicates(duplicateValues);
    check(uniqueSubsets.size() == 6, "subsetsWithDuplicates removes repeats");
    check(uniqueSubsets.back() == std::vector<int>{2, 2},
          "subsetsWithDuplicates includes repeated values together");
    check(Backtracking::subsetsWithDuplicates(empty) ==
              std::vector<std::vector<int>>{{}},
          "subsetsWithDuplicates handles empty input");

    constexpr std::array permutationValues{1, 1, 2};
    const auto permutations = Backtracking::permutations(permutationValues);
    check(permutations.size() == 3, "permutations deduplicates equal values");
    check(permutations.front() == std::vector<int>{1, 1, 2},
          "permutations has deterministic sorted traversal");
    check(Backtracking::permutations(empty) ==
              std::vector<std::vector<int>>{{}},
          "permutations returns one empty permutation");

    constexpr std::array candidates{2, 3, 6, 7};
    check(Backtracking::combinationSum(candidates, 7) ==
              std::vector<std::vector<int>>{{2, 2, 3}, {7}},
          "combinationSum finds reusable candidate combinations");
    constexpr std::array duplicatedCandidates{2, 2};
    check(Backtracking::combinationSum(duplicatedCandidates, 4) ==
              std::vector<std::vector<int>>{{2, 2}},
          "combinationSum deduplicates candidate denominations");
    check(Backtracking::combinationSum(empty, 0) ==
              std::vector<std::vector<int>>{{}},
          "combinationSum handles the zero target");
    check(Backtracking::combinationSum(empty, 3).empty(),
          "combinationSum reports no solution");
    constexpr std::array zeroCandidate{0};
    checkThrows<std::invalid_argument>(
        [&] { (void)Backtracking::combinationSum(zeroCandidate, 1); },
        "combinationSum rejects nonpositive candidates");
    checkThrows<std::invalid_argument>(
        [&] { (void)Backtracking::combinationSum(empty, -1); },
        "combinationSum rejects a negative target");

    const std::vector<std::string> board{"ABCE", "SFCS", "ADEE"};
    check(Backtracking::wordSearch(board, "ABCCED"),
          "wordSearch finds a valid non-reusing path");
    check(!Backtracking::wordSearch(board, "ABCB"),
          "wordSearch does not reuse a board cell");
    check(Backtracking::wordSearch(board, ""),
          "wordSearch accepts the empty word");
    check(!Backtracking::wordSearch(emptyStrings, "x"),
          "wordSearch rejects nonempty words on an empty board");
    const std::vector<std::string> ragged{"ab", "c"};
    checkThrows<std::invalid_argument>(
        [&] { (void)Backtracking::wordSearch(ragged, "a"); },
        "wordSearch rejects ragged boards");

    check(Backtracking::palindromePartitioning("aab") ==
              std::vector<std::vector<std::string>>{{"a", "a", "b"},
                                                    {"aa", "b"}},
          "palindromePartitioning enumerates palindrome partitions");
    check(Backtracking::palindromePartitioning("") ==
              std::vector<std::vector<std::string>>{{}},
          "palindromePartitioning handles empty input");
}

void testDynamicProgramming()
{
    using dsa::dynamic_programming::DynamicProgramming;

    const std::array<std::string, 0> emptyStrings{};
    check(DynamicProgramming::climbingStairs(0) == 1,
          "climbingStairs counts the empty route");
    check(DynamicProgramming::climbingStairs(5) == 8,
          "climbingStairs counts one- and two-step routes");
    check(DynamicProgramming::climbingStairs(92) ==
              12200160415121876738ULL,
          "climbingStairs supports its largest uint64 result");
    checkThrows<std::overflow_error>(
        [] { (void)DynamicProgramming::climbingStairs(93); },
        "climbingStairs reports count overflow");

    constexpr std::array houses{2, 7, 9, 3, 1};
    check(DynamicProgramming::houseRobber(houses) == 12,
          "houseRobber selects nonadjacent houses");
    constexpr std::array losses{-2, -1};
    check(DynamicProgramming::houseRobber(losses) == 0,
          "houseRobber may skip all negative houses");
    constexpr int largest = std::numeric_limits<int>::max();
    constexpr std::array largeHouses{largest, largest, largest};
    check(DynamicProgramming::houseRobber(largeHouses) ==
              2LL * std::numeric_limits<int>::max(),
          "houseRobber accumulates values in a wide result type");
    constexpr std::array<int, 0> empty{};
    check(DynamicProgramming::houseRobber(empty) == 0,
          "houseRobber handles empty input");

    constexpr std::array circularHouses{2, 3, 2};
    check(DynamicProgramming::houseRobberII(circularHouses) == 3,
          "houseRobberII treats the endpoints as adjacent");
    constexpr std::array oneHouse{5};
    check(DynamicProgramming::houseRobberII(oneHouse) == 5,
          "houseRobberII handles one house");
    check(DynamicProgramming::houseRobberII(empty) == 0,
          "houseRobberII handles empty input");

    constexpr std::array coins{1, 2, 5};
    check(DynamicProgramming::coinChange(coins, 11) == 3,
          "coinChange returns a minimum coin count");
    constexpr std::array onlyTwo{2};
    check(!DynamicProgramming::coinChange(onlyTwo, 3),
          "coinChange reports an impossible amount");
    check(DynamicProgramming::coinChange(empty, 0) == 0,
          "coinChange handles zero amount");
    constexpr std::array badCoins{0, 1};
    checkThrows<std::invalid_argument>(
        [&] { (void)DynamicProgramming::coinChange(badCoins, 3); },
        "coinChange rejects a nonpositive denomination");
    checkThrows<std::invalid_argument>(
        [&] { (void)DynamicProgramming::coinChange(coins, -1); },
        "coinChange rejects a negative amount");

    check(DynamicProgramming::coinChangeII(coins, 5) == 4,
          "coinChangeII counts unordered combinations");
    constexpr std::array repeatedCoins{1, 1, 2};
    check(DynamicProgramming::coinChangeII(repeatedCoins, 2) == 2,
          "coinChangeII deduplicates denominations");
    check(DynamicProgramming::coinChangeII(empty, 0) == 1,
          "coinChangeII counts the empty combination");
    checkThrows<std::invalid_argument>(
        [&] { (void)DynamicProgramming::coinChangeII(badCoins, 1); },
        "coinChangeII rejects nonpositive denominations");

    constexpr std::array sequence{10, 9, 2, 5, 3, 7, 101, 18};
    check(DynamicProgramming::longestIncreasingSubsequence(sequence) == 4,
          "longestIncreasingSubsequence finds a strict increasing run");
    constexpr std::array equalSequence{2, 2, 2};
    check(DynamicProgramming::longestIncreasingSubsequence(equalSequence) == 1,
          "longestIncreasingSubsequence is strictly increasing");
    check(DynamicProgramming::longestIncreasingSubsequence(empty) == 0,
          "longestIncreasingSubsequence handles empty input");

    constexpr std::array partitionable{1, 5, 11, 5};
    constexpr std::array notPartitionable{1, 2, 3, 5};
    check(DynamicProgramming::canPartition(partitionable),
          "canPartition finds equal-sum subsets");
    check(!DynamicProgramming::canPartition(notPartitionable),
          "canPartition rejects an uneven total");
    check(DynamicProgramming::canPartition(empty),
          "canPartition handles empty input");
    constexpr std::array negativeValues{-1};
    checkThrows<std::invalid_argument>(
        [&] { (void)DynamicProgramming::canPartition(negativeValues); },
        "canPartition rejects negative values");

    constexpr std::array targetValues{1, 1, 1, 1, 1};
    check(DynamicProgramming::targetSumWays(targetValues, 3) == 5,
          "targetSumWays counts sign assignments");
    check(DynamicProgramming::targetSumWays(targetValues, -3) == 5,
          "targetSumWays handles negative targets");
    constexpr std::array zeroValue{0};
    check(DynamicProgramming::targetSumWays(zeroValue, 0) == 2,
          "targetSumWays counts both signs for zero");
    check(DynamicProgramming::targetSumWays(targetValues, 20) == 0,
          "targetSumWays reports an unreachable target");
    check(DynamicProgramming::targetSumWays(empty, 0) == 1,
          "targetSumWays handles empty input");
    checkThrows<std::invalid_argument>(
        [&] { (void)DynamicProgramming::targetSumWays(negativeValues, 0); },
        "targetSumWays rejects negative values");

    check(DynamicProgramming::decodeWays("226") == 3,
          "decodeWays counts valid decodings");
    check(DynamicProgramming::decodeWays("06") == 0,
          "decodeWays rejects leading zero encodings");
    check(DynamicProgramming::decodeWays("") == 0,
          "decodeWays handles empty input");
    checkThrows<std::invalid_argument>(
        [] { (void)DynamicProgramming::decodeWays("1a"); },
        "decodeWays rejects nondigit input");

    const std::vector<std::string> dictionary{"leet", "code"};
    check(DynamicProgramming::wordBreak("leetcode", dictionary),
          "wordBreak segments known words");
    const std::vector<std::string> overlapping{"cats", "dog", "sand", "and",
                                               "cat"};
    check(!DynamicProgramming::wordBreak("catsandog", overlapping),
          "wordBreak detects an unsegmentable suffix");
    check(DynamicProgramming::wordBreak("", emptyStrings),
          "wordBreak treats empty text as segmentable");
    const std::vector<std::string> emptyWords{"", "a"};
    check(DynamicProgramming::wordBreak("a", emptyWords),
          "wordBreak ignores empty dictionary entries");
}

void testGreedy()
{
    using dsa::greedy::Greedy;

    constexpr std::array reachableJumps{2, 3, 1, 1, 4};
    constexpr std::array blockedJumps{3, 2, 1, 0, 4};
    constexpr std::array<int, 0> empty{};
    constexpr std::array one{0};
    check(Greedy::jumpGame(reachableJumps), "jumpGame finds a reachable end");
    check(!Greedy::jumpGame(blockedJumps), "jumpGame detects an unreachable end");
    check(Greedy::jumpGame(empty), "jumpGame treats empty input as reachable");
    check(Greedy::jumpGame(one), "jumpGame handles a singleton");
    constexpr std::array invalidJumps{-1};
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::jumpGame(invalidJumps); },
        "jumpGame rejects negative jump lengths");

    check(Greedy::jumpGameII(reachableJumps) == 2,
          "jumpGameII finds the minimum jumps");
    check(!Greedy::jumpGameII(blockedJumps),
          "jumpGameII reports unreachable input");
    check(!Greedy::jumpGameII(empty), "jumpGameII reports empty input infeasible");
    check(Greedy::jumpGameII(one) == 0,
          "jumpGameII needs no jump for a singleton");
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::jumpGameII(invalidJumps); },
        "jumpGameII rejects negative jumps");

    constexpr std::array gas{1, 2, 3, 4, 5};
    constexpr std::array cost{3, 4, 5, 1, 2};
    check(Greedy::gasStation(gas, cost) == 3,
          "gasStation finds a feasible start");
    constexpr std::array insufficientGas{2, 3, 4};
    constexpr std::array highCost{3, 4, 3};
    check(!Greedy::gasStation(insufficientGas, highCost),
          "gasStation reports no feasible circuit");
    check(!Greedy::gasStation(empty, empty),
          "gasStation handles an empty route");
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::gasStation(gas, empty); },
        "gasStation requires matching lengths");
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::gasStation(invalidJumps, one); },
        "gasStation rejects negative gas amounts");

    constexpr std::array ratings{1, 0, 2};
    check(Greedy::candy(ratings) == 5,
          "candy satisfies increasing and decreasing neighbors");
    constexpr std::array equalRatings{3, 3, 3};
    check(Greedy::candy(equalRatings) == 3,
          "candy handles equal ratings");
    check(Greedy::candy(empty) == 0, "candy handles empty input");

    constexpr std::array<std::pair<int, int>, 6> people{
        {{7, 0}, {4, 4}, {7, 1}, {5, 0}, {6, 1}, {5, 2}}};
    check(Greedy::queueReconstructionByHeight(people) ==
              std::vector<std::pair<int, int>>{
                  {5, 0}, {7, 0}, {5, 2}, {6, 1}, {4, 4}, {7, 1}},
          "queueReconstructionByHeight restores the standard queue");
    constexpr std::array<std::pair<int, int>, 0> noPeople{};
    check(Greedy::queueReconstructionByHeight(noPeople).empty(),
          "queue reconstruction handles empty input");
    constexpr std::array<std::pair<int, int>, 1> impossibleQueue{{{5, 1}}};
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::queueReconstructionByHeight(impossibleQueue); },
        "queue reconstruction rejects impossible constraints");
    constexpr std::array<std::pair<int, int>, 1> malformedQueue{{{-1, 0}}};
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::queueReconstructionByHeight(malformedQueue); },
        "queue reconstruction rejects negative heights");

    constexpr std::array prices{7, 1, 5, 3, 6, 4};
    check(Greedy::stockII(prices) == 7,
          "stockII collects each profitable rise");
    constexpr std::array declining{5, 4, 3};
    check(Greedy::stockII(declining) == 0,
          "stockII earns nothing from declining prices");
    check(Greedy::stockII(empty) == 0, "stockII handles empty input");
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::stockII(invalidJumps); },
        "stockII rejects negative prices");

    constexpr std::array straights{1, 2, 3, 6, 2, 3, 4, 7, 8};
    check(Greedy::handOfStraights(straights, 3),
          "handOfStraights forms consecutive groups");
    constexpr std::array brokenStraight{1, 2, 4};
    check(!Greedy::handOfStraights(brokenStraight, 3),
          "handOfStraights detects a missing card");
    constexpr std::array highCards{std::numeric_limits<int>::max() - 1,
                                   std::numeric_limits<int>::max()};
    check(Greedy::handOfStraights(highCards, 2),
          "handOfStraights handles values at the integer boundary");
    check(Greedy::handOfStraights(empty, 2),
          "handOfStraights accepts an empty hand");
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::handOfStraights(straights, 0); },
        "handOfStraights rejects a zero group size");

    check(Greedy::minDeletionsUniqueFrequencies("aaabbbcc") == 2,
          "minDeletionsUniqueFrequencies makes frequencies unique");
    check(Greedy::minDeletionsUniqueFrequencies("") == 0,
          "minDeletionsUniqueFrequencies handles empty input");
    check(Greedy::minDeletionsUniqueFrequencies("aab") == 0,
          "minDeletionsUniqueFrequencies preserves already unique counts");

    check(Greedy::partitionLabels("ababcbacadefegdehijhklij") ==
              std::vector<std::size_t>{9, 7, 8},
          "partitionLabels finds maximal independent ranges");
    check(Greedy::partitionLabels("") == std::vector<std::size_t>{},
          "partitionLabels handles empty input");
    check(Greedy::partitionLabels("abc") ==
              std::vector<std::size_t>{1, 1, 1},
          "partitionLabels separates distinct characters");

    constexpr std::array<int, 5> successfulBills{5, 5, 5, 10, 20};
    constexpr std::array<int, 3> unsuccessfulBills{5, 10, 20};
    check(Greedy::lemonadeChange(successfulBills),
          "lemonadeChange makes change when possible");
    check(!Greedy::lemonadeChange(unsuccessfulBills),
          "lemonadeChange reports when change is unavailable");
    check(Greedy::lemonadeChange(empty),
          "lemonadeChange handles no customers");
    constexpr std::array invalidBills{10, 15};
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::lemonadeChange(invalidBills); },
        "lemonadeChange rejects invalid bill denominations");

    constexpr std::array greed{1, 2, 3};
    constexpr std::array cookies{1, 1};
    check(Greedy::assignCookies(greed, cookies) == 1,
          "assignCookies maximizes satisfied children");
    check(Greedy::assignCookies(empty, empty) == 0,
          "assignCookies handles empty inputs");
    constexpr std::array zeroGreed{0};
    constexpr std::array zeroCookie{0};
    check(Greedy::assignCookies(zeroGreed, zeroCookie) == 1,
          "assignCookies handles zero sizes and greed");
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::assignCookies(invalidJumps, cookies); },
        "assignCookies rejects negative greed");

    constexpr std::array<std::array<int, 3>, 4> triplets{{
        {2, 5, 3}, {1, 8, 4}, {1, 7, 5}, {2, 3, 4}}};
    constexpr std::array<int, 3> target{2, 7, 5};
    check(Greedy::mergeTriplets(triplets, target),
          "mergeTriplets combines compatible components");
    constexpr std::array<int, 3> unreachableTarget{2, 5, 5};
    check(!Greedy::mergeTriplets(triplets, unreachableTarget),
          "mergeTriplets rejects missing target components");
    constexpr std::array<std::array<int, 3>, 0> noTriplets{};
    constexpr std::array<int, 3> zeroTarget{0, 0, 0};
    check(!Greedy::mergeTriplets(noTriplets, zeroTarget),
          "mergeTriplets handles empty input");
    constexpr std::array<std::array<int, 3>, 1> negativeTriplet{{{-1, 0, 0}}};
    checkThrows<std::invalid_argument>(
        [&] { (void)Greedy::mergeTriplets(negativeTriplet, target); },
        "mergeTriplets rejects negative components");
}

} // namespace

int main()
{
    try {
        testBacktracking();
        testDynamicProgramming();
        testGreedy();
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "Passed " << checks << " checks.\n";
    return 0;
}
