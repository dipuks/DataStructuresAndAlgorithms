#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dsa/graphs.hpp"
#include "dsa/heaps.hpp"
#include "dsa/trees.hpp"

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

std::unique_ptr<dsa::trees::Node> makeTree()
{
    auto root = std::make_unique<dsa::trees::Node>(4);
    root->left = std::make_unique<dsa::trees::Node>(2);
    root->right = std::make_unique<dsa::trees::Node>(7);
    root->left->left = std::make_unique<dsa::trees::Node>(1);
    root->left->right = std::make_unique<dsa::trees::Node>(3);
    root->right->left = std::make_unique<dsa::trees::Node>(6);
    root->right->right = std::make_unique<dsa::trees::Node>(9);
    return root;
}

void testTrees()
{
    using dsa::trees::Node;
    using dsa::trees::Trees;

    check(Trees::maximumDepth(nullptr) == 0, "maximumDepth empty tree");
    check(Trees::diameter(nullptr) == 0, "diameter empty tree");
    check(Trees::isBalanced(nullptr), "empty tree is balanced");
    check(Trees::sameTree(nullptr, nullptr), "sameTree accepts null trees");
    check(Trees::isSubtree(nullptr, nullptr), "empty tree is an empty subtree");
    check(Trees::levelOrder(nullptr).empty(), "levelOrder empty tree");
    check(Trees::zigzagLevelOrder(nullptr).empty(),
          "zigzagLevelOrder empty tree");
    check(Trees::inorderTraversal(nullptr).empty(), "inorderTraversal empty tree");
    check(Trees::rightSideView(nullptr).empty(), "rightSideView empty tree");
    check(Trees::countGoodNodes(nullptr) == 0, "countGoodNodes empty tree");
    check(Trees::flattenPreorder(nullptr) == nullptr,
          "flattenPreorder accepts an empty tree");
    check(!Trees::maximumPathSum(nullptr), "maximumPathSum empty tree");
    check(Trees::lowestCommonAncestor(nullptr, nullptr, nullptr) == nullptr,
          "lowestCommonAncestor rejects null arguments");
    check(Trees::isValidBst(nullptr), "empty tree is a valid BST");
    check(!Trees::kthSmallest(nullptr, 1), "kthSmallest on empty tree");
    check(!Trees::kthSmallest(nullptr, 0), "kthSmallest rejects k zero");
    check(Trees::serialize(nullptr) == "#", "serialize null root");
    check(Trees::deserialize("#") == nullptr, "deserialize null root");
    checkThrows<std::invalid_argument>([] { (void)Trees::deserialize(""); },
                                       "deserialize rejects empty encoding");
    checkThrows<std::invalid_argument>([] { (void)Trees::deserialize("1,#"); },
                                       "deserialize rejects truncated tree");
    checkThrows<std::invalid_argument>([] { (void)Trees::deserialize("#,"); },
                                       "deserialize rejects trailing delimiter");

    auto root = makeTree();
    check(Trees::maximumDepth(root.get()) == 3, "maximumDepth normal tree");
    check(Trees::diameter(root.get()) == 4, "diameter counts edges");
    check(Trees::isBalanced(root.get()), "isBalanced accepts balanced tree");
    check(Trees::levelOrder(root.get()) ==
              std::vector<std::vector<int>>{{4}, {2, 7}, {1, 3, 6, 9}},
          "levelOrder traverses breadth first");
    check(Trees::zigzagLevelOrder(root.get()) ==
              std::vector<std::vector<int>>{{4}, {7, 2}, {1, 3, 6, 9}},
          "zigzagLevelOrder alternates level direction");
    check(Trees::inorderTraversal(root.get()) ==
              std::vector<int>{1, 2, 3, 4, 6, 7, 9},
          "inorderTraversal visits left-root-right");
    check(Trees::rightSideView(root.get()) == std::vector<int>{4, 7, 9},
          "rightSideView chooses final node at each level");
    check(Trees::countGoodNodes(root.get()) == 3,
          "countGoodNodes uses root-to-node maxima");
    check(Trees::isValidBst(root.get()), "isValidBst accepts strict BST");
    check(Trees::kthSmallest(root.get(), 4) == 4,
          "kthSmallest returns inorder rank");
    check(!Trees::kthSmallest(root.get(), 8), "kthSmallest handles too-large k");

    Node* left = root->left.get();
    check(Trees::lowestCommonAncestorBst(root.get(), left->left.get(),
                                         left->right.get()) == left,
          "lowestCommonAncestorBst returns identity ancestor");
    check(Trees::lowestCommonAncestor(root.get(), left->left.get(),
                                      root->right->right.get()) == root.get(),
          "lowestCommonAncestor handles targets in separate subtrees");
    check(Trees::lowestCommonAncestor(root.get(), left, left) == left,
          "lowestCommonAncestor supports the same node twice");
    auto arbitraryTree = std::make_unique<Node>(10);
    arbitraryTree->left = std::make_unique<Node>(20);
    arbitraryTree->right = std::make_unique<Node>(3);
    arbitraryTree->left->right = std::make_unique<Node>(30);
    arbitraryTree->right->left = std::make_unique<Node>(2);
    check(Trees::lowestCommonAncestor(arbitraryTree.get(),
                                      arbitraryTree->left->right.get(),
                                      arbitraryTree->right->left.get()) ==
              arbitraryTree.get(),
          "lowestCommonAncestor does not assume BST ordering");
    Node outsider(2);
    check(Trees::lowestCommonAncestorBst(root.get(), left, &outsider) == nullptr,
          "lowestCommonAncestorBst rejects equal-value outsider pointer");
    check(Trees::lowestCommonAncestor(root.get(), left, &outsider) == nullptr,
          "lowestCommonAncestor rejects a missing target identity");

    auto same = makeTree();
    check(Trees::sameTree(root.get(), same.get()), "sameTree compares shape and values");
    same->right->right->value = 10;
    check(!Trees::sameTree(root.get(), same.get()), "sameTree detects value difference");
    auto subtree = std::make_unique<Node>(2);
    subtree->left = std::make_unique<Node>(1);
    subtree->right = std::make_unique<Node>(3);
    check(Trees::isSubtree(root.get(), subtree.get()), "isSubtree finds matching subtree");
    Node absent(100);
    check(!Trees::isSubtree(root.get(), &absent), "isSubtree rejects absent values");

    auto invalidBst = std::make_unique<Node>(2);
    invalidBst->left = std::make_unique<Node>(2);
    check(!Trees::isValidBst(invalidBst.get()), "BST duplicate values are rejected");
    auto skewed = std::make_unique<Node>(1);
    skewed->right = std::make_unique<Node>(2);
    skewed->right->right = std::make_unique<Node>(3);
    check(!Trees::isBalanced(skewed.get()), "isBalanced detects a skewed tree");

    auto pathTree = std::make_unique<Node>(-10);
    pathTree->left = std::make_unique<Node>(9);
    pathTree->right = std::make_unique<Node>(20);
    pathTree->right->left = std::make_unique<Node>(15);
    pathTree->right->right = std::make_unique<Node>(7);
    check(Trees::maximumPathSum(pathTree.get()) == 42,
          "maximumPathSum combines a path through an interior node");
    auto allNegative = std::make_unique<Node>(-3);
    allNegative->left = std::make_unique<Node>(-5);
    allNegative->right = std::make_unique<Node>(-2);
    check(Trees::maximumPathSum(allNegative.get()) == -2,
          "maximumPathSum handles all-negative nodes");

    auto flattenTree = makeTree();
    const std::array<Node*, 7> preorderNodes{
        flattenTree.get(), flattenTree->left.get(),
        flattenTree->left->left.get(), flattenTree->left->right.get(),
        flattenTree->right.get(), flattenTree->right->left.get(),
        flattenTree->right->right.get()};
    check(Trees::flattenPreorder(flattenTree.get()) == flattenTree.get(),
          "flattenPreorder returns the original root");
    Node* flattened = flattenTree.get();
    for (Node* expected : preorderNodes) {
        check(flattened == expected && flattened->left == nullptr,
              "flattenPreorder preserves preorder node identity and clears left links");
        flattened = flattened->right.get();
    }
    check(flattened == nullptr, "flattenPreorder terminates the right-only chain");

    constexpr std::array preorder{4, 2, 1, 3, 7, 6, 9};
    constexpr std::array inorder{1, 2, 3, 4, 6, 7, 9};
    auto rebuilt = Trees::buildFromPreorderInorder(preorder, inorder);
    check(Trees::sameTree(root.get(), rebuilt.get()),
          "buildFromPreorderInorder constructs expected tree");
    constexpr std::array mismatched{1, 2};
    constexpr std::array other{1, 3};
    checkThrows<std::invalid_argument>(
        [&] { (void)Trees::buildFromPreorderInorder(mismatched, other); },
        "tree builder rejects different traversal contents");
    constexpr std::array duplicate{1, 1};
    checkThrows<std::invalid_argument>(
        [&] { (void)Trees::buildFromPreorderInorder(duplicate, duplicate); },
        "tree builder rejects duplicate traversal values");
    constexpr std::array inconsistentPreorder{2, 3, 1};
    constexpr std::array inconsistentInorder{1, 2, 3};
    checkThrows<std::invalid_argument>(
        [&] {
            (void)Trees::buildFromPreorderInorder(inconsistentPreorder,
                                                  inconsistentInorder);
        },
        "tree builder rejects inconsistent traversal order");

    const std::string encoded = Trees::serialize(root.get());
    auto decoded = Trees::deserialize(encoded);
    check(Trees::sameTree(root.get(), decoded.get()),
          "serialize and deserialize round-trip");
    checkThrows<std::invalid_argument>([] { (void)Trees::deserialize("#,1"); },
                                       "deserialize rejects trailing tokens");
    Node* inverted = Trees::invertTree(root.get());
    check(inverted == root.get() && root->left->value == 7 &&
              root->right->value == 2,
          "invertTree swaps children in place");
}

void testGraphs()
{
    using dsa::graphs::Graphs;

    const std::vector<std::vector<char>> grid{
        {'1', '1', '0', '0'}, {'1', '0', '0', '1'}, {'0', '0', '1', '1'}};
    check(Graphs::numberOfIslands(grid) == 2, "numberOfIslands counts components");
    check(Graphs::maximumIslandArea(grid) == 3, "maximumIslandArea counts largest land");
    check(Graphs::numberOfIslands(std::vector<std::vector<char>>{}) == 0,
          "numberOfIslands accepts empty grid");
    check(Graphs::maximumIslandArea(std::vector<std::vector<char>>{{'0', '0'}}) == 0,
          "maximumIslandArea returns zero with no land");
    checkThrows<std::invalid_argument>(
        [] { (void)Graphs::numberOfIslands(std::vector<std::vector<char>>{{'1'}, {'0', '1'}}); },
        "island APIs reject ragged grids");
    checkThrows<std::invalid_argument>(
        [] { (void)Graphs::maximumIslandArea(std::vector<std::vector<char>>{{'x'}}); },
        "island APIs reject invalid cell convention");

    const std::vector<std::vector<int>> oranges{
        {2, 1, 1}, {1, 1, 0}, {0, 1, 1}};
    check(Graphs::rottingOranges(oranges) == 4,
          "rottingOranges counts simultaneous spread minutes");
    check(Graphs::rottingOranges(std::vector<std::vector<int>>{{0, 2}}) == 0,
          "rottingOranges returns zero without fresh oranges");
    check(Graphs::rottingOranges(std::vector<std::vector<int>>{{2, 0, 1}}) ==
              -1,
          "rottingOranges reports fresh oranges blocked from rot");
    check(Graphs::rottingOranges(std::vector<std::vector<int>>{{1}}) == -1,
          "rottingOranges reports fresh fruit when no rotten source exists");
    check(Graphs::rottingOranges(std::vector<std::vector<int>>{}) == 0,
          "rottingOranges accepts empty grid");
    checkThrows<std::invalid_argument>(
        [] { (void)Graphs::rottingOranges(std::vector<std::vector<int>>{{1}, {1, 0}}); },
        "rottingOranges rejects ragged grids");
    checkThrows<std::invalid_argument>(
        [] { (void)Graphs::rottingOranges(std::vector<std::vector<int>>{{3}}); },
        "rottingOranges rejects unsupported cell values");

    const std::vector<std::vector<int>> heights{
        {1, 2, 2, 3, 5}, {3, 2, 3, 4, 4}, {2, 4, 5, 3, 1},
        {6, 7, 1, 4, 5}, {5, 1, 1, 2, 4}};
    const auto bothOceans = Graphs::pacificAtlantic(heights);
    check(bothOceans.size() == 7 &&
              bothOceans.front() == std::pair<std::size_t, std::size_t>{0, 4},
          "pacificAtlantic finds flow-reachable cells in row-major order");
    check(Graphs::pacificAtlantic(std::vector<std::vector<int>>{}).empty(),
          "pacificAtlantic accepts empty grid");
    checkThrows<std::invalid_argument>(
        [] { (void)Graphs::pacificAtlantic(std::vector<std::vector<int>>{{1}, {1, 2}}); },
        "pacificAtlantic rejects ragged grids");

    dsa::graphs::Graph graph;
    auto* first = graph.addNode(10);
    auto* second = graph.addNode(20);
    graph.addUndirectedEdge(10, 20);
    graph.addDirectedEdge(10, 10);
    check(graph.findNode(10) == first && graph.findNode(99) == nullptr,
          "Graph looks up stable labels");
    const dsa::graphs::Graph& constGraph = graph;
    check(constGraph.findNode(20) == second && constGraph.findNode(99) == nullptr,
          "const Graph lookup preserves constness and missing-label behavior");
    check(first->neighbors.size() == 2 && second->neighbors.size() == 1,
          "Graph stores directed and undirected adjacency");
    checkThrows<std::invalid_argument>([&] { (void)graph.addNode(10); },
                                       "Graph rejects duplicate IDs");
    checkThrows<std::invalid_argument>([&] { graph.addDirectedEdge(10, 99); },
                                       "Graph rejects missing edge endpoint");
    checkThrows<std::invalid_argument>([&] { graph.addUndirectedEdge(10, 99); },
                                       "Graph rejects undirected edge with missing endpoint");
    check(first->neighbors.size() == 2,
          "failed undirected edge insertion does not partially mutate graph");
    const auto clone = Graphs::cloneGraph(graph);
    check(clone.nodes.size() == 2 && clone.entry != first &&
              clone.entry->id == 10 && clone.entry->neighbors.size() == 2,
          "cloneGraph owns independent reachable clones");
    dsa::graphs::Graph emptyGraph;
    check(Graphs::cloneGraph(emptyGraph).nodes.empty(),
          "cloneGraph accepts graph without an entry node");

    constexpr std::array prerequisites{
        std::pair<std::size_t, std::size_t>{1, 0},
        std::pair<std::size_t, std::size_t>{2, 0},
        std::pair<std::size_t, std::size_t>{3, 1},
        std::pair<std::size_t, std::size_t>{3, 2}};
    check(Graphs::canFinishCourses(4, prerequisites),
          "canFinishCourses accepts acyclic prerequisites");
    check(Graphs::courseOrder(4, prerequisites) ==
              std::vector<std::size_t>{0, 1, 2, 3},
          "courseOrder deterministically resolves ties");
    constexpr std::array cycle{
        std::pair<std::size_t, std::size_t>{0, 1},
        std::pair<std::size_t, std::size_t>{1, 0}};
    check(!Graphs::canFinishCourses(2, cycle) &&
              Graphs::courseOrder(2, cycle).empty(),
          "course APIs detect cycles");
    check(Graphs::courseOrder(0, std::span<const std::pair<std::size_t, std::size_t>>{}).empty(),
          "courseOrder handles zero courses");
    checkThrows<std::invalid_argument>(
        [] {
            constexpr std::array invalid{std::pair<std::size_t, std::size_t>{0, 2}};
            (void)Graphs::courseOrder(2, invalid);
        },
        "courseOrder validates prerequisite IDs");

    const std::array<std::string, 6> words{
        "hot", "dot", "dog", "lot", "log", "cog"};
    check(Graphs::wordLadderLength("hit", "cog", words) == 5,
          "wordLadderLength finds shortest chain");
    check(Graphs::wordLadderLength("same", "same",
                                   std::span<const std::string>{}) == 1,
          "wordLadderLength defines identical endpoints");
    check(Graphs::wordLadderLength("hit", "zzz", words) == 0,
          "wordLadderLength reports unreachable endpoint");
    check(Graphs::wordLadderLength("ab", "a", words) == 0,
          "wordLadderLength rejects unequal lengths");
    checkThrows<std::invalid_argument>(
        [&] { (void)Graphs::wordLadderLength("Hit", "cog", words); },
        "wordLadderLength validates lowercase ASCII input");

    const std::array<std::pair<std::string, std::string>, 2> equations{
        std::pair<std::string, std::string>{"a", "b"},
        std::pair<std::string, std::string>{"b", "c"}};
    constexpr std::array ratios{2.0, 3.0};
    const std::array<std::pair<std::string, std::string>, 5> queries{
        std::pair<std::string, std::string>{"a", "c"},
        {"c", "a"}, {"a", "a"}, {"x", "x"}, {"a", "x"}};
    const auto values = Graphs::evaluateDivision(equations, ratios, queries);
    check(values.size() == 5 && std::abs(values[0] - 6.0) < 1e-9 &&
              std::abs(values[1] - 1.0 / 6.0) < 1e-9 && values[2] == 1.0 &&
              values[3] == -1.0 && values[4] == -1.0,
          "evaluateDivision handles connected, reverse, self, and unknown queries");
    const std::array<std::pair<std::string, std::string>, 1> disconnectedQuery{
        std::pair<std::string, std::string>{"a", "d"}};
    const std::array<std::pair<std::string, std::string>, 3> disconnectedEquations{
        std::pair<std::string, std::string>{"a", "b"},
        std::pair<std::string, std::string>{"b", "c"},
        std::pair<std::string, std::string>{"d", "e"}};
    constexpr std::array disconnectedRatios{2.0, 3.0, 4.0};
    check(Graphs::evaluateDivision(disconnectedEquations, disconnectedRatios,
                                  disconnectedQuery) ==
              std::vector<double>{-1.0},
          "evaluateDivision returns minus one for known disconnected variables");
    const std::array<std::pair<std::string, std::string>, 3>
        extremeEquations{
            std::pair<std::string, std::string>{"a", "b"},
            std::pair<std::string, std::string>{"c", "d"},
            std::pair<std::string, std::string>{"b", "d"}};
    constexpr std::array extremeRatios{1e-308, 1e-308, 1e308};
    const std::array<std::pair<std::string, std::string>, 1> extremeQuery{
        std::pair<std::string, std::string>{"c", "a"}};
    const auto extremeValues = Graphs::evaluateDivision(
        extremeEquations, extremeRatios, extremeQuery);
    check(extremeValues.size() == 1 && extremeValues[0] > 0.0 &&
              std::abs(extremeValues[0] / 1e-308 - 1.0) < 1e-12,
          "evaluateDivision avoids overflowing representable intermediate ratios");
    constexpr std::array invalidRatio{0.0};
    const std::array<std::pair<std::string, std::string>, 1> oneEquation{
        std::pair<std::string, std::string>{"a", "b"}};
    checkThrows<std::invalid_argument>(
        [&] { (void)Graphs::evaluateDivision(oneEquation, invalidRatio, queries); },
        "evaluateDivision rejects nonpositive ratios");
    checkThrows<std::invalid_argument>(
        [&] {
            (void)Graphs::evaluateDivision(
                equations, std::span<const double>(ratios).first<1>(), queries);
        },
        "evaluateDivision rejects mismatched equation counts");

    constexpr std::array edges{
        dsa::graphs::WeightedEdge{1, 2, 4},
        dsa::graphs::WeightedEdge{1, 3, 1},
        dsa::graphs::WeightedEdge{3, 2, 2},
        dsa::graphs::WeightedEdge{2, 4, 1}};
    const auto distances = Graphs::shortestPathDistances(5, edges, 1);
    check(distances ==
              std::vector<std::optional<long long>>{0, 3, 1, 4, std::nullopt},
          "shortestPathDistances computes shortest paths and safe unreachable");
    constexpr std::array noEdges{dsa::graphs::WeightedEdge{1, 2, 0}};
    checkThrows<std::invalid_argument>(
        [&] { (void)Graphs::shortestPathDistances(2, noEdges, 1); },
        "Dijkstra rejects nonpositive edge weights");
    checkThrows<std::invalid_argument>(
        [&] {
            (void)Graphs::shortestPathDistances(
                2, std::span<const dsa::graphs::WeightedEdge>{}, 0);
        },
        "Dijkstra validates source indexing");
}

void testHeaps()
{
    using dsa::heaps::Heaps;
    using dsa::heaps::MedianFinder;

    constexpr std::array values{3, 2, 3, -5, 1, 5, 6, 4};
    check(Heaps::kthLargest(values, 1) == 6, "kthLargest returns maximum");
    check(Heaps::kthLargest(values, 3) == 4, "kthLargest counts duplicates");
    checkThrows<std::invalid_argument>([&] { (void)Heaps::kthLargest(values, 0); },
                                       "kthLargest rejects zero k");
    checkThrows<std::invalid_argument>([&] { (void)Heaps::kthLargest(values, 9); },
                                       "kthLargest rejects oversized k");
    constexpr std::array initial{4, 5, 8, 2};
    constexpr std::array additions{3, 5, 10, 9, 4};
    check(Heaps::kthLargestInStream(initial, additions, 3) ==
              std::vector<std::optional<int>>{4, 5, 5, 8, 8},
          "kthLargestInStream reports each addition");
    constexpr std::array smallInitial{1};
    constexpr std::array smallAdditions{2, 3};
    check(Heaps::kthLargestInStream(smallInitial, smallAdditions, 3) ==
              std::vector<std::optional<int>>{std::nullopt, 1},
          "kthLargestInStream reports when stream has too few elements");
    checkThrows<std::invalid_argument>(
        [&] { (void)Heaps::kthLargestInStream(initial, additions, 0); },
        "kthLargestInStream rejects zero k");

    constexpr std::array stones{2, 7, 4, 1, 8, 1};
    check(Heaps::lastStoneWeight(stones) == 1, "lastStoneWeight smashes heaviest stones");
    check(Heaps::lastStoneWeight(std::span<const int>{}) == 0,
          "lastStoneWeight handles empty input");
    constexpr std::array negativeStone{-1};
    checkThrows<std::invalid_argument>(
        [&] { (void)Heaps::lastStoneWeight(negativeStone); },
        "lastStoneWeight rejects negative weights");

    constexpr std::array<dsa::heaps::Point, 4> points{
        dsa::heaps::Point{1, 3}, dsa::heaps::Point{-2, 2},
        dsa::heaps::Point{2, -2}, dsa::heaps::Point{0, 0}};
    check(Heaps::kClosestPoints(points, 2) ==
              std::vector<dsa::heaps::Point>{{0, 0}, {-2, 2}},
          "kClosestPoints sorts nearest points with stable ties");
    check(Heaps::kClosestPoints(points, 0).empty(),
          "kClosestPoints accepts zero k");
    checkThrows<std::invalid_argument>(
        [&] { (void)Heaps::kClosestPoints(points, 5); },
        "kClosestPoints rejects excessive k");
    constexpr std::array<dsa::heaps::Point, 1> extremePoints{
        dsa::heaps::Point{std::numeric_limits<int>::min(),
                          std::numeric_limits<int>::min()}};
    check(Heaps::kClosestPoints(extremePoints, 1) ==
              std::vector<dsa::heaps::Point>{{std::numeric_limits<int>::min(),
                                              std::numeric_limits<int>::min()}},
          "kClosestPoints safely handles int coordinate bounds");

    const std::vector<std::vector<int>> sortedLists{
        {1, 4, 5}, {}, {1, 3, 4}, {2, 6}};
    check(Heaps::mergeKSortedLists(sortedLists) ==
              std::vector<int>{1, 1, 2, 3, 4, 4, 5, 6},
          "mergeKSortedLists merges duplicates and empty lists");
    check(Heaps::mergeKSortedLists(
              std::span<const std::vector<int>>{}).empty(),
          "mergeKSortedLists accepts no lists");
    check(Heaps::mergeKSortedLists(
              std::vector<std::vector<int>>{{}, {}}).empty(),
          "mergeKSortedLists accepts only empty lists");
    check(Heaps::mergeKSortedLists(
              std::vector<std::vector<int>>{{-2, 0, 9}}) ==
              std::vector<int>{-2, 0, 9},
          "mergeKSortedLists accepts one sorted list");
    checkThrows<std::invalid_argument>(
        [] {
            const std::vector<std::vector<int>> unsorted{{2, 1}};
            (void)Heaps::mergeKSortedLists(unsorted);
        },
        "mergeKSortedLists rejects unsorted input");

    constexpr std::array tasks{'A', 'A', 'A', 'B', 'B', 'B'};
    check(Heaps::leastTaskIntervals(tasks, 2) == 8,
          "leastTaskIntervals includes required idle slots");
    check(Heaps::leastTaskIntervals(std::span<const char>{}, 100) == 0,
          "leastTaskIntervals handles empty task list");
    constexpr std::array uniqueTasks{'A', 'B', 'C'};
    check(Heaps::leastTaskIntervals(uniqueTasks, 0) == 3,
          "leastTaskIntervals handles zero cooldown");

    const auto reorganized = Heaps::reorganizeString("aaabbc");
    check(reorganized.has_value() && reorganized->size() == 6 &&
              std::adjacent_find(reorganized->begin(), reorganized->end()) ==
                  reorganized->end(),
          "reorganizeString separates repeated characters");
    std::string originalCounts = "aaabbc";
    std::string reorganizedCounts = reorganized.value_or("");
    std::ranges::sort(originalCounts);
    std::ranges::sort(reorganizedCounts);
    check(originalCounts == reorganizedCounts,
          "reorganizeString preserves the byte multiset");
    check(!Heaps::reorganizeString("aaaaab"),
          "reorganizeString reports impossible frequencies");
    check(Heaps::reorganizeString("") == std::optional<std::string>{""},
          "reorganizeString accepts empty input");
    check(Heaps::reorganizeString("x") == std::optional<std::string>{"x"},
          "reorganizeString accepts a single byte");
    check(Heaps::reorganizeString("aaabb") == std::optional<std::string>{"ababa"},
          "reorganizeString deterministically resolves frequency ties");
    const std::string binaryBytes{"\0\0\xff\xff", 4};
    const auto binaryReorganized = Heaps::reorganizeString(binaryBytes);
    check(binaryReorganized.has_value() &&
              std::adjacent_find(binaryReorganized->begin(),
                                 binaryReorganized->end()) ==
                  binaryReorganized->end(),
          "reorganizeString treats bytes as unsigned distinct values");

    MedianFinder median;
    check(!median.findMedian(), "MedianFinder empty stream has no median");
    median.addNum(-1);
    check(median.findMedian() == -1.0, "MedianFinder accepts a negative value");
    median.addNum(-1);
    check(median.findMedian() == -1.0, "MedianFinder averages duplicates");
    median.addNum(5);
    check(median.findMedian() == -1.0, "MedianFinder maintains odd-size median");
    median.addNum(3);
    check(median.findMedian() == 1.0, "MedianFinder averages even middle values");
    median.addNum(3);
    check(median.findMedian() == 3.0, "MedianFinder handles repeated middle values");
}

} // namespace

int main()
{
    try {
        testTrees();
        testGraphs();
        testHeaps();
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }
    std::cout << "Passed " << checks << " checks.\n";
    return 0;
}
