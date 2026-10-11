#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dsa::graphs {

/// Unweighted adjacency-list vertex. Neighbor pointers are non-owning and
/// must point to live vertices owned by the containing Graph.
struct GraphNode {
    explicit GraphNode(int nodeId) : id(nodeId) {}

    int id;
    std::vector<GraphNode*> neighbors;
};

/// Owns its nodes; adjacency and entry pointers are non-owning. IDs are unique
/// labels, not vector indices. Add nodes before adding edges; edges may point
/// in one or both directions as requested. Do not manually change node IDs or
/// adjacency pointers.
struct Graph {
    std::vector<std::unique_ptr<GraphNode>> nodes;
    GraphNode* entry = nullptr;

    /// Adds a uniquely identified owned node; duplicate IDs throw.
    /// Time: O(V); auxiliary space: O(1) apart from the new node.
    /// Example: addNode(7) returns the owned vertex labeled 7.
    GraphNode* addNode(int id)
    {
        if (findNode(id) != nullptr) {
            throw std::invalid_argument("graph node IDs must be unique");
        }
        nodes.push_back(std::make_unique<GraphNode>(id));
        if (entry == nullptr) {
            entry = nodes.back().get();
        }
        return nodes.back().get();
    }

    /// Looks up a label, returning nullptr when it does not exist.
    /// Time: O(V); auxiliary space: O(1).
    /// Example: findNode(7) returns its pointer, or nullptr if label 7 is absent.
    [[nodiscard]] GraphNode* findNode(int id) noexcept
    {
        for (const auto& node : nodes) {
            if (node->id == id) {
                return node.get();
            }
        }
        return nullptr;
    }

    /// Const lookup by label; returns nullptr when absent.
    /// Time: O(V); auxiliary space: O(1).
    /// Example: constGraph.findNode(7) performs the same lookup without
    /// allowing the returned pointer to modify the vertex.
    [[nodiscard]] const GraphNode* findNode(int id) const noexcept
    {
        for (const auto& node : nodes) {
            if (node->id == id) {
                return node.get();
            }
        }
        return nullptr;
    }

    /// Adds a directed edge (parallel edges are allowed). Unknown labels throw
    /// invalid_argument.
    /// Time: O(V); auxiliary space: O(1) beyond the adjacency entry.
    /// Example: addDirectedEdge(7, 9) adds 9 to node 7's outgoing neighbors.
    void addDirectedEdge(int fromId, int toId)
    {
        GraphNode* from = findNode(fromId);
        GraphNode* to = findNode(toId);
        if (from == nullptr || to == nullptr) {
            throw std::invalid_argument("edge endpoints must exist");
        }
        from->neighbors.push_back(to);
    }

    /// Adds an undirected edge by adding both directed adjacency entries.
    /// Time: O(V); auxiliary space: O(1) beyond the adjacency entries.
    /// Example: addUndirectedEdge(7, 9) records each node beside the other.
    void addUndirectedEdge(int firstId, int secondId)
    {
        GraphNode* first = findNode(firstId);
        GraphNode* second = findNode(secondId);
        if (first == nullptr || second == nullptr) {
            throw std::invalid_argument("edge endpoints must exist");
        }
        first->neighbors.push_back(second);
        second->neighbors.push_back(first);
    }
};

/// Directed weighted edge for shortest-path input. Vertex labels in
/// shortestPathDistances are one-based indices [1, vertexCount]; weights must
/// be strictly positive. Evaluate-division ratios use a separate equation type.
struct WeightedEdge {
    std::size_t from;
    std::size_t to;
    long long weight;
};

/// Static solutions for graph and grid interview problems.
class Graphs final {
public:
    Graphs() = delete;

    /// Counts 4-connected land components in a rectangular grid containing
    /// only '0' (water) and '1' (land). Ragged or non-binary grids throw before
    /// processing; input is unchanged. Time: O(rows*columns); auxiliary space:
    /// O(rows*columns).
    /// Example: [["1","1","0"],["0","1","0"],["1","0","1"]] -> 3 islands.
    /// DFS marks each connected land cell so it cannot start another island.
    [[nodiscard]] static std::size_t numberOfIslands(
        std::span<const std::vector<char>> grid)
    {
        const std::size_t columns = validateBinaryGrid(grid);
        if (grid.empty() || columns == 0) {
            return 0;
        }
        std::vector<std::vector<bool>> seen(grid.size(),
                                            std::vector<bool>(columns, false));
        std::size_t islands = 0;
        std::vector<std::pair<std::size_t, std::size_t>> pending;
        for (std::size_t row = 0; row < grid.size(); ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                if (grid[row][column] != '1' || seen[row][column]) {
                    continue;
                }
                ++islands;
                seen[row][column] = true;
                pending.emplace_back(row, column);
                while (!pending.empty()) {
                    const auto [r, c] = pending.back();
                    pending.pop_back();
                    forEachNeighbor(r, c, grid.size(), columns,
                                    [&](std::size_t nr, std::size_t nc) {
                                        if (!seen[nr][nc] &&
                                            grid[nr][nc] == '1') {
                                            seen[nr][nc] = true;
                                            pending.emplace_back(nr, nc);
                                        }
                                    });
                }
            }
        }
        return islands;
    }

    /// Returns the largest 4-connected land area, or zero for no land.
    /// Grid conventions, validation, and immutability match numberOfIslands.
    /// Time: O(rows*columns); auxiliary space: O(rows*columns).
    /// Example: [["1","1","0"],["0","1","0"],["1","0","1"]] -> max area 3.
    /// Flood-fill one component at a time and count the cells reached.
    [[nodiscard]] static std::size_t maximumIslandArea(
        std::span<const std::vector<char>> grid)
    {
        const std::size_t columns = validateBinaryGrid(grid);
        if (grid.empty() || columns == 0) {
            return 0;
        }
        std::vector<std::vector<bool>> seen(grid.size(),
                                            std::vector<bool>(columns, false));
        std::vector<std::pair<std::size_t, std::size_t>> pending;
        std::size_t maximum = 0;
        for (std::size_t row = 0; row < grid.size(); ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                if (grid[row][column] != '1' || seen[row][column]) {
                    continue;
                }
                std::size_t area = 0;
                seen[row][column] = true;
                pending.emplace_back(row, column);
                while (!pending.empty()) {
                    const auto [r, c] = pending.back();
                    pending.pop_back();
                    ++area;
                    forEachNeighbor(r, c, grid.size(), columns,
                                    [&](std::size_t nr, std::size_t nc) {
                                        if (!seen[nr][nc] &&
                                            grid[nr][nc] == '1') {
                                            seen[nr][nc] = true;
                                            pending.emplace_back(nr, nc);
                                        }
                                    });
                }
                maximum = std::max(maximum, area);
            }
        }
        return maximum;
    }

    /// Computes the minutes until all fresh oranges rot by 4-neighbor spread.
    /// Input is a rectangular grid of 0 (empty), 1 (fresh), and 2 (rotten);
    /// invalid cells or ragged rows throw invalid_argument. Returns 0 when no
    /// fresh oranges exist and -1 when some fresh orange is unreachable.
    /// Input is unchanged; throws overflow_error if the minute count cannot
    /// fit in int. Time: O(rows*columns); auxiliary space:
    /// O(rows*columns).
    /// Example: [[2,1,1],[1,1,0],[0,1,1]] -> 4 minutes. Multi-source BFS
    /// processes one queue level per minute, spreading from every rotten cell.
    [[nodiscard]] static int rottingOranges(
        std::span<const std::vector<int>> grid)
    {
        if (grid.empty()) {
            return 0;
        }
        const std::size_t rows = grid.size();
        const std::size_t columns = grid.front().size();
        std::queue<std::pair<std::size_t, std::size_t>> pending;
        std::vector<std::vector<bool>> rotted(
            rows, std::vector<bool>(columns, false));
        std::size_t fresh = 0;
        for (const auto& row : grid) {
            if (row.size() != columns) {
                throw std::invalid_argument("orange grid must be rectangular");
            }
            for (int cell : row) {
                if (cell < 0 || cell > 2) {
                    throw std::invalid_argument("orange cells must be 0, 1, or 2");
                }
            }
        }
        for (std::size_t row = 0; row < rows; ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                if (grid[row][column] == 1) {
                    ++fresh;
                } else if (grid[row][column] == 2) {
                    rotted[row][column] = true;
                    pending.emplace(row, column);
                }
            }
        }
        if (fresh == 0) {
            return 0;
        }
        int minutes = 0;
        while (!pending.empty() && fresh > 0) {
            const std::size_t levelSize = pending.size();
            bool rottedThisMinute = false;
            for (std::size_t index = 0; index < levelSize; ++index) {
                const auto [row, column] = pending.front();
                pending.pop();
                forEachNeighbor(row, column, rows, columns,
                                [&](std::size_t nextRow, std::size_t nextColumn) {
                                    if (grid[nextRow][nextColumn] == 1) {
                                        // The caller's grid remains immutable;
                                        // track visited fresh cells separately.
                                        if (!rotted[nextRow][nextColumn]) {
                                            rotted[nextRow][nextColumn] = true;
                                            --fresh;
                                            rottedThisMinute = true;
                                            pending.emplace(nextRow, nextColumn);
                                        }
                                    }
                                });
            }
            if (rottedThisMinute) {
                if (minutes == std::numeric_limits<int>::max()) {
                    throw std::overflow_error("orange-rotting time exceeds int");
                }
                ++minutes;
            }
        }
        return fresh == 0 ? minutes : -1;
    }

    /// Returns the nodes that can reach both the top/left and bottom/right
    /// borders by moving only to equal-or-higher heights. Input must be
    /// rectangular; empty dimensions yield an empty result. Result coordinates
    /// are in row-major order. Time: O(rows*columns); auxiliary space:
    /// O(rows*columns).
    [[nodiscard]] static std::vector<std::pair<std::size_t, std::size_t>>
    /// Example: [[1,2],[3,4]] -> all four coordinates. Traverse backward from
    /// both ocean borders over nondecreasing heights, then intersect results.
    pacificAtlantic(std::span<const std::vector<int>> heights)
    {
        if (heights.empty()) {
            return {};
        }
        const std::size_t rows = heights.size();
        const std::size_t columns = heights.front().size();
        for (const auto& row : heights) {
            if (row.size() != columns) {
                throw std::invalid_argument("height grid must be rectangular");
            }
        }
        if (columns == 0) {
            return {};
        }
        auto flood = [&](bool pacific) {
            std::vector<std::vector<bool>> reached(
                rows, std::vector<bool>(columns, false));
            std::vector<std::pair<std::size_t, std::size_t>> stack;
            auto push = [&](std::size_t row, std::size_t column) {
                if (!reached[row][column]) {
                    reached[row][column] = true;
                    stack.emplace_back(row, column);
                }
            };
            for (std::size_t row = 0; row < rows; ++row) {
                push(row, pacific ? 0 : columns - 1);
            }
            for (std::size_t column = 0; column < columns; ++column) {
                push(pacific ? 0 : rows - 1, column);
            }
            while (!stack.empty()) {
                const auto [row, column] = stack.back();
                stack.pop_back();
                forEachNeighbor(row, column, rows, columns,
                                [&](std::size_t nr, std::size_t nc) {
                                    if (!reached[nr][nc] &&
                                        heights[nr][nc] >= heights[row][column]) {
                                        reached[nr][nc] = true;
                                        stack.emplace_back(nr, nc);
                                    }
                                });
            }
            return reached;
        };
        const auto pacific = flood(true);
        const auto atlantic = flood(false);
        std::vector<std::pair<std::size_t, std::size_t>> result;
        for (std::size_t row = 0; row < rows; ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                if (pacific[row][column] && atlantic[row][column]) {
                    result.emplace_back(row, column);
                }
            }
        }
        return result;
    }

    /// Clones all vertices reachable from graph.entry; every reachable vertex
    /// must be owned by graph and have a unique ID. The returned Graph owns
    /// every cloned node and preserves IDs, edge direction, duplicates, and
    /// cycles. Nodes not reachable from entry are omitted. Time: O(V+E);
    /// auxiliary space: O(V).
    /// Example: edges 1->2 and 2->1 produce independent cloned vertices
    /// with the same cycle and labels.
    [[nodiscard]] static Graph cloneGraph(const Graph& graph)
    {
        Graph clone;
        if (graph.entry == nullptr) {
            return clone;
        }
        std::unordered_map<const GraphNode*, GraphNode*> copies;
        std::queue<const GraphNode*> pending;
        const auto addCopy = [&](const GraphNode* source) {
            const auto found = copies.find(source);
            if (found != copies.end()) {
                return found->second;
            }
            GraphNode* copy = clone.addNode(source->id);
            copies.emplace(source, copy);
            pending.push(source);
            return copy;
        };
        clone.entry = addCopy(graph.entry);
        while (!pending.empty()) {
            const GraphNode* source = pending.front();
            pending.pop();
            GraphNode* destination = copies.at(source);
            for (const GraphNode* neighbor : source->neighbors) {
                if (neighbor == nullptr) {
                    throw std::invalid_argument("graph neighbors cannot be null");
                }
                destination->neighbors.push_back(addCopy(neighbor));
            }
        }
        return clone;
    }

    /// Determines whether all courses can be completed using Kahn's
    /// topological-sort invariant: a cycle remains when no vertex is ready.
    /// Prerequisites are
    /// (course, prerequisite) pairs with zero-based IDs in [0, courseCount);
    /// invalid IDs throw. Time: O((V+E)log V); auxiliary space: O(V+E).
    /// Example: 2 courses and prerequisite (1,0) -> true; the directed
    /// prerequisite graph has no cycle, so both courses can be completed.
    [[nodiscard]] static bool canFinishCourses(
        std::size_t courseCount,
        std::span<const std::pair<std::size_t, std::size_t>> prerequisites)
    {
        return !courseOrder(courseCount, prerequisites).empty() ||
               courseCount == 0;
    }

    /// Produces a deterministic valid topological course order via Kahn's
    /// indegree invariant, choosing the
    /// smallest currently available course ID first. Returns empty on cycles
    /// (also for zero courses); prerequisites are (course, prerequisite), IDs
    /// zero-based. Invalid IDs throw. Time: O((V+E)log V); auxiliary space:
    /// O(V+E).
    /// Example: 4 courses and [(1,0),(2,0),(3,1),(3,2)] -> [0,1,2,3].
    /// Emit the smallest zero-indegree course, then release its outgoing edges.
    [[nodiscard]] static std::vector<std::size_t> courseOrder(
        std::size_t courseCount,
        std::span<const std::pair<std::size_t, std::size_t>> prerequisites)
    {
        std::vector<std::vector<std::size_t>> outgoing(courseCount);
        std::vector<std::size_t> indegree(courseCount, 0);
        for (const auto [course, prerequisite] : prerequisites) {
            if (course >= courseCount || prerequisite >= courseCount) {
                throw std::invalid_argument("course ID is out of range");
            }
            outgoing[prerequisite].push_back(course);
            ++indegree[course];
        }
        std::priority_queue<std::size_t, std::vector<std::size_t>,
                            std::greater<>> ready;
        for (std::size_t course = 0; course < courseCount; ++course) {
            if (indegree[course] == 0) {
                ready.push(course);
            }
        }
        std::vector<std::size_t> order;
        order.reserve(courseCount);
        while (!ready.empty()) {
            const std::size_t prerequisite = ready.top();
            ready.pop();
            order.push_back(prerequisite);
            for (std::size_t course : outgoing[prerequisite]) {
                if (--indegree[course] == 0) {
                    ready.push(course);
                }
            }
        }
        if (order.size() != courseCount) {
            return {};
        }
        return order;
    }

    /// Uses breadth-first search to return the shortest word-ladder length
    /// including both endpoints, or
    /// zero when unreachable. Endpoints and dictionary entries must contain
    /// lowercase ASCII letters only (otherwise invalid_argument is thrown).
    /// Only same-length dictionary words are considered; they may be
    /// intermediate/end words.
    /// begin==end returns 1 even if absent from the dictionary. Time:
    /// O(D*L^2) with string hashing, where D is dictionary size and L word
    /// length; auxiliary space O(D*L).
    /// Example: hit -> cog through hot, dot, dog -> length 5. BFS explores
    /// one-letter transformations in increasing path length.
    [[nodiscard]] static std::size_t wordLadderLength(
        std::string_view begin, std::string_view end,
        std::span<const std::string> wordList)
    {
        const auto lowercase = [](std::string_view word) {
            return std::ranges::all_of(word, [](char letter) {
                return letter >= 'a' && letter <= 'z';
            });
        };
        if (!lowercase(begin) || !lowercase(end)) {
            throw std::invalid_argument("word endpoints must be lowercase ASCII");
        }
        for (const std::string& word : wordList) {
            if (!lowercase(word)) {
                throw std::invalid_argument(
                    "dictionary words must be lowercase ASCII");
            }
        }
        if (begin == end) {
            return 1;
        }
        if (begin.size() != end.size()) {
            return 0;
        }
        std::unordered_set<std::string> remaining;
        remaining.reserve(wordList.size());
        for (const std::string& word : wordList) {
            if (word.size() == begin.size()) {
                remaining.insert(word);
            }
        }
        if (!remaining.contains(std::string(end))) {
            return 0;
        }
        std::queue<std::pair<std::string, std::size_t>> pending;
        pending.emplace(begin, 1);
        while (!pending.empty()) {
            auto [word, length] = std::move(pending.front());
            pending.pop();
            for (std::size_t index = 0; index < word.size(); ++index) {
                const char original = word[index];
                for (char letter = 'a'; letter <= 'z'; ++letter) {
                    if (letter == original) {
                        continue;
                    }
                    word[index] = letter;
                    auto found = remaining.find(word);
                    if (found == remaining.end()) {
                        continue;
                    }
                    if (word == end) {
                        return length + 1;
                    }
                    pending.emplace(*found, length + 1);
                    remaining.erase(found);
                }
                word[index] = original;
            }
        }
        return 0;
    }

    /// Uses weighted disjoint-set union with path compression to evaluate
    /// positive, finite, mutually consistent variable ratios. Equation entries pair
    /// variable names with corresponding ratios; queries return -1.0 for an
    /// unknown/empty variable or disconnected pair, and 1.0 for a known variable
    /// divided by itself. Unequal equation/ratio counts or invalid ratios throw
    /// invalid_argument; derived floating-point overflow or underflow throws
    /// overflow_error.
    /// Time: O((E+Q)*α(V)) amortized; auxiliary space: O(V).
    /// Example: a/b=2 and b/c=3, query a/c -> 6. Weighted DSU stores each
    /// variable's ratio to its component root.
    [[nodiscard]] static std::vector<double> evaluateDivision(
        std::span<const std::pair<std::string, std::string>> equations,
        std::span<const double> ratios,
        std::span<const std::pair<std::string, std::string>> queries)
    {
        if (equations.size() != ratios.size()) {
            throw std::invalid_argument("equation and ratio counts must match");
        }
        RatioSets sets;
        for (std::size_t index = 0; index < equations.size(); ++index) {
            if (equations[index].first.empty() || equations[index].second.empty() ||
                !std::isfinite(ratios[index]) || ratios[index] <= 0.0) {
                throw std::invalid_argument(
                    "variables must be nonempty and ratios positive and finite");
            }
            sets.unite(equations[index].first, equations[index].second,
                       ratios[index]);
        }
        std::vector<double> answers;
        answers.reserve(queries.size());
        for (const auto& [numerator, denominator] : queries) {
            if (numerator.empty() || denominator.empty()) {
                answers.push_back(-1.0);
                continue;
            }
            const auto first = sets.lookup(numerator);
            const auto second = sets.lookup(denominator);
            if (!first || !second || first->first != second->first) {
                answers.push_back(-1.0);
            } else {
                const double quotient = first->second / second->second;
                if (!std::isfinite(quotient) || quotient <= 0.0) {
                    throw std::overflow_error("derived ratio is not representable");
                }
                answers.push_back(quotient);
            }
        }
        return answers;
    }

    /// Computes single-source shortest distances using Dijkstra's
    /// nonnegative-distance priority-queue invariant on a directed
    /// graph. Vertices are one-based [1, vertexCount], source is in-range, and
    /// all edges must have positive weights; invalid data throws. Unreachable
    /// distances are nullopt. Throws overflow_error if a finite path sum cannot
    /// fit in long long. Time: O((V+E)log V); auxiliary space: O(V+E).
    [[nodiscard]] static std::vector<std::optional<long long>>
    /// Example: edges 1->2 (4), 1->3 (10), 2->3 (2), source 1 -> [0,4,6].
    /// Relax outgoing edges from the cheapest known frontier first.
    shortestPathDistances(std::size_t vertexCount,
                          std::span<const WeightedEdge> edges,
                          std::size_t source)
    {
        if (source == 0 || source > vertexCount) {
            throw std::invalid_argument("source must be a one-based vertex ID");
        }
        std::vector<std::vector<std::pair<std::size_t, long long>>> adjacency(
            vertexCount + 1);
        for (const WeightedEdge& edge : edges) {
            if (edge.from == 0 || edge.from > vertexCount || edge.to == 0 ||
                edge.to > vertexCount || edge.weight <= 0) {
                throw std::invalid_argument(
                    "edge endpoints must be valid and weights positive");
            }
            adjacency[edge.from].emplace_back(edge.to, edge.weight);
        }
        std::vector<std::optional<long long>> distances(vertexCount + 1);
        using QueueEntry = std::pair<long long, std::size_t>;
        std::priority_queue<QueueEntry, std::vector<QueueEntry>,
                            std::greater<>> pending;
        distances[source] = 0;
        pending.emplace(0, source);
        while (!pending.empty()) {
            const auto [distance, vertex] = pending.top();
            pending.pop();
            if (!distances[vertex] || distance != *distances[vertex]) {
                continue;
            }
            for (const auto [neighbor, weight] : adjacency[vertex]) {
                if (distance > std::numeric_limits<long long>::max() - weight) {
                    throw std::overflow_error("shortest-path distance overflow");
                }
                const long long candidate = distance + weight;
                if (!distances[neighbor] || candidate < *distances[neighbor]) {
                    distances[neighbor] = candidate;
                    pending.emplace(candidate, neighbor);
                }
            }
        }
        distances.erase(distances.begin());
        return distances;
    }

private:
    template <typename Function>
    static void forEachNeighbor(std::size_t row, std::size_t column,
                                std::size_t rows, std::size_t columns,
                                Function&& function)
    {
        if (row > 0) {
            function(row - 1, column);
        }
        if (row + 1 < rows) {
            function(row + 1, column);
        }
        if (column > 0) {
            function(row, column - 1);
        }
        if (column + 1 < columns) {
            function(row, column + 1);
        }
    }

    static std::size_t validateBinaryGrid(
        std::span<const std::vector<char>> grid)
    {
        if (grid.empty()) {
            return 0;
        }
        const std::size_t columns = grid.front().size();
        for (const auto& row : grid) {
            if (row.size() != columns) {
                throw std::invalid_argument("binary grid must be rectangular");
            }
            for (char cell : row) {
                if (cell != '0' && cell != '1') {
                    throw std::invalid_argument("grid cells must be '0' or '1'");
                }
            }
        }
        return columns;
    }

    /// Weighted disjoint-set forest stores weight[x] = x / parent[x].
    class RatioSets {
    public:
        void unite(const std::string& numerator, const std::string& denominator,
                   double ratio)
        {
            ensure(numerator);
            ensure(denominator);
            const auto [rootA, weightA] = findRoot(numerator);
            const auto [rootB, weightB] = findRoot(denominator);
            if (rootA == rootB) {
                return;
            }
            if (rank_[rootA] < rank_[rootB]) {
                const double attachedWeight =
                    multiplyDivideChecked(ratio, weightB, weightA);
                parent_[rootA] = rootB;
                weight_[rootA] = attachedWeight;
            } else {
                const double attachedWeight =
                    divideByProductChecked(weightA, ratio, weightB);
                parent_[rootB] = rootA;
                weight_[rootB] = attachedWeight;
                if (rank_[rootA] == rank_[rootB]) {
                    ++rank_[rootA];
                }
            }
        }

        [[nodiscard]] std::optional<std::pair<std::string, double>> lookup(
            const std::string& variable)
        {
            if (!parent_.contains(variable)) {
                return std::nullopt;
            }
            const auto [root, weight] = findRoot(variable);
            return std::pair{root, weight};
        }

    private:
        static void validateWeight(double weight)
        {
            if (!std::isfinite(weight) || weight <= 0.0) {
                throw std::overflow_error("derived ratio is not representable");
            }
        }

        static double multiplyDivideChecked(
            double first,
            double second,
            double divisor)
        {
            int firstExponent = 0;
            int secondExponent = 0;
            int divisorExponent = 0;
            const double mantissa =
                std::frexp(first, &firstExponent) *
                std::frexp(second, &secondExponent) /
                std::frexp(divisor, &divisorExponent);
            const double result = std::scalbn(
                mantissa, firstExponent + secondExponent - divisorExponent);
            validateWeight(result);
            return result;
        }

        static double divideByProductChecked(
            double numerator,
            double firstDivisor,
            double secondDivisor)
        {
            int numeratorExponent = 0;
            int firstDivisorExponent = 0;
            int secondDivisorExponent = 0;
            const double mantissa =
                std::frexp(numerator, &numeratorExponent) /
                (std::frexp(firstDivisor, &firstDivisorExponent) *
                 std::frexp(secondDivisor, &secondDivisorExponent));
            const double result = std::scalbn(
                mantissa,
                numeratorExponent - firstDivisorExponent - secondDivisorExponent);
            validateWeight(result);
            return result;
        }

        void ensure(const std::string& variable)
        {
            if (parent_.try_emplace(variable, variable).second) {
                weight_[variable] = 1.0;
                rank_[variable] = 0;
            }
        }

        std::pair<std::string, double> findRoot(const std::string& variable)
        {
            if (parent_.at(variable) == variable) {
                return {variable, 1.0};
            }
            const std::string oldParent = parent_.at(variable);
            const auto [root, parentWeight] = findRoot(oldParent);
            const double combinedWeight = weight_.at(variable) * parentWeight;
            validateWeight(combinedWeight);
            weight_[variable] = combinedWeight;
            parent_[variable] = root;
            return {root, weight_.at(variable)};
        }

        std::unordered_map<std::string, std::string> parent_;
        std::unordered_map<std::string, double> weight_;
        std::unordered_map<std::string, std::size_t> rank_;
    };
};

} // namespace dsa::graphs
