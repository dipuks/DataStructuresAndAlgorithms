#pragma once

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dsa::trees {

/// A binary-tree node. Children are owned by their parent with unique_ptr;
/// any Node* passed to an API is a non-owning pointer and must remain valid
/// for the duration of the call. Roots returned by builders/deserializers own
/// their complete trees.
struct Node {
    explicit Node(int nodeValue) : value(nodeValue) {}

    int value;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
};

/// Static solutions for binary-tree interview problems.
class Trees final {
public:
    Trees() = delete;

    /// Swaps child ownership recursively to invert a tree in place and returns
    /// the unchanged root pointer.
    /// Time: O(n); auxiliary space: O(h) recursion stack, h is tree height.
    /// Example: root [4,2,7,1,3,6,9] becomes [4,7,2,9,6,3,1]. Swap children
    /// at every node; recursive calls visit each owned subtree once.
    [[nodiscard]] static Node* invertTree(Node* root)
    {
        if (root == nullptr) {
            return nullptr;
        }
        std::swap(root->left, root->right);
        (void)invertTree(root->left.get());
        (void)invertTree(root->right.get());
        return root;
    }

    /// Recursively returns one plus the larger child depth, the number of nodes
    /// on the longest root-to-leaf path.
    /// Empty tree depth is zero. Time: O(n); auxiliary space: O(h).
    /// Example: [3,9,20,null,null,15,7] -> depth 3. The deepest path is the
    /// larger child depth plus the current root.
    [[nodiscard]] static std::size_t maximumDepth(const Node* root)
    {
        if (root == nullptr) {
            return 0;
        }
        return 1 + std::max(maximumDepth(root->left.get()),
                            maximumDepth(root->right.get()));
    }

    /// Computes child heights bottom-up and combines them at every node to find
    /// the diameter measured in edges (zero for empty/singleton).
    /// Time: O(n); auxiliary space: O(h).
    /// Example: [1,2,3,4,5] -> diameter 3 edges (path 4-2-1-3). At each node,
    /// combine the left and right subtree heights as a possible longest path.
    [[nodiscard]] static std::size_t diameter(const Node* root)
    {
        std::size_t longest = 0;
        (void)heightAndDiameter(root, longest);
        return longest;
    }

    /// Computes child heights bottom-up, propagating an imbalance sentinel;
    /// reports whether each node's subtree heights differ by at most one.
    /// Time: O(n); auxiliary space: O(h).
    /// Example: [3,9,20,null,null,15,7] -> true. A subtree reports its height
    /// or an imbalance sentinel, allowing early propagation of failure.
    [[nodiscard]] static bool isBalanced(const Node* root)
    {
        return balancedHeight(root) >= 0;
    }

    /// Recursively compares shape and values; two null trees are equal.
    /// Time: O(min(n,m)); auxiliary space: O(h).
    /// Example: trees [1,2,3] and [1,2,3] -> true; changing one child value
    /// makes them unequal. Compare corresponding null shapes and node values.
    [[nodiscard]] static bool sameTree(const Node* first, const Node* second)
    {
        if (first == nullptr || second == nullptr) {
            return first == second;
        }
        return first->value == second->value &&
               sameTree(first->left.get(), second->left.get()) &&
               sameTree(first->right.get(), second->right.get());
    }

    /// Reports whether candidate is a subtree of root. An empty candidate is
    /// a subtree of every tree. Time: O(n*m) worst case; auxiliary space O(h).
    /// Example: root [3,4,5,1,2], candidate [4,1,2] -> true. Try each root
    /// node as a match, then compare both shapes recursively.
    [[nodiscard]] static bool isSubtree(const Node* root, const Node* candidate)
    {
        if (candidate == nullptr) {
            return true;
        }
        if (root == nullptr) {
            return false;
        }
        return sameTree(root, candidate) ||
               isSubtree(root->left.get(), candidate) ||
               isSubtree(root->right.get(), candidate);
    }

    /// First verifies both target pointer identities occur in the tree, then
    /// follows BST value bounds to find their lowest common ancestor.
    /// BST values must be unique and ordered strictly; returns nullptr unless
    /// both exact node pointers occur in the tree. Time: O(n+h); auxiliary
    /// space: O(h), including identity-presence checks.
    /// Example: in BST [6,2,8,0,4,7,9], nodes 2 and 8 have LCA 6. First
    /// confirm both exact nodes belong to the tree, then follow value bounds.
    [[nodiscard]] static Node* lowestCommonAncestorBst(
        Node* root, const Node* first, const Node* second)
    {
        if (first == nullptr || second == nullptr ||
            !containsIdentity(root, first) || !containsIdentity(root, second)) {
            return nullptr;
        }

        Node* current = root;
        while (current != nullptr) {
            if (first->value < current->value &&
                second->value < current->value) {
                current = current->left.get();
            } else if (first->value > current->value &&
                       second->value > current->value) {
                current = current->right.get();
            } else {
                return current;
            }
        }
        return nullptr;
    }

    /// Finds the lowest common ancestor by node identity in an arbitrary
    /// binary tree (no BST ordering assumed). Returns nullptr when either
    /// target is null or either exact pointer is absent; if both arguments
    /// identify the same present node, returns that node. The tree is not
    /// modified. Time: O(n); auxiliary space: O(h).
    /// Example: in tree [3,5,1,6,2,0,8], nodes 5 and 1 have LCA 3. Search
    /// both subtrees by pointer identity; the first node whose branches split
    /// the targets is their lowest common ancestor.
    [[nodiscard]] static Node* lowestCommonAncestor(
        Node* root, const Node* first, const Node* second)
    {
        if (root == nullptr || first == nullptr || second == nullptr) {
            return nullptr;
        }
        const SearchResult result = findCommonAncestor(root, first, second);
        return result.foundFirst && result.foundSecond ? result.ancestor : nullptr;
    }

    /// Uses a breadth-first queue to return values grouped by depth, left to
    /// right. Time: O(n);
    /// auxiliary space: O(w) queue, excluding the returned levels.
    /// Example: [3,9,20,null,null,15,7] -> [[3],[9,20],[15,7]]. Process
    /// exactly the nodes already queued at each breadth-first level.
    [[nodiscard]] static std::vector<std::vector<int>> levelOrder(
        const Node* root)
    {
        std::vector<std::vector<int>> result;
        if (root == nullptr) {
            return result;
        }

        std::vector<const Node*> queue{root};
        for (std::size_t head = 0; head < queue.size();) {
            const std::size_t end = queue.size();
            auto& level = result.emplace_back();
            level.reserve(end - head);
            while (head < end) {
                const Node* node = queue[head++];
                level.push_back(node->value);
                if (node->left) {
                    queue.push_back(node->left.get());
                }
                if (node->right) {
                    queue.push_back(node->right.get());
                }
            }
        }
        return result;
    }

    /// Returns breadth-first levels alternating left-to-right and
    /// right-to-left, beginning left-to-right. Empty input returns no levels.
    /// Time: O(n); auxiliary space: O(w) beyond the returned levels.
    /// Example: [3,9,20,null,null,15,7] -> [[3],[20,9],[15,7]]. Reverse
    /// every other level after collecting ordinary breadth-first levels.
    [[nodiscard]] static std::vector<std::vector<int>> zigzagLevelOrder(
        const Node* root)
    {
        auto result = levelOrder(root);
        for (std::size_t level = 1; level < result.size(); level += 2) {
            std::ranges::reverse(result[level]);
        }
        return result;
    }

    /// Visits nodes left-root-right iteratively, returning values in inorder.
    /// Time: O(n); auxiliary space: O(h), excluding the returned vector.
    /// Example: [1,null,2,3] -> [1,3,2]. Descend left while stacking parents,
    /// then visit each parent and its right subtree.
    [[nodiscard]] static std::vector<int> inorderTraversal(const Node* root)
    {
        std::vector<int> result;
        std::vector<const Node*> stack;
        const Node* current = root;
        while (current != nullptr || !stack.empty()) {
            while (current != nullptr) {
                stack.push_back(current);
                current = current->left.get();
            }
            current = stack.back();
            stack.pop_back();
            result.push_back(current->value);
            current = current->right.get();
        }
        return result;
    }

    /// Takes the final value of each breadth-first level, visible when viewed
    /// from the right.
    /// Time: O(n); auxiliary space: O(w).
    /// Example: [1,2,3,null,5,null,4] -> [1,3,4]. The last node in each
    /// breadth-first level is visible from the right side.
    [[nodiscard]] static std::vector<int> rightSideView(const Node* root)
    {
        std::vector<int> result;
        for (const auto& level : levelOrder(root)) {
            result.push_back(level.back());
        }
        return result;
    }

    /// Carries the maximum root-path value downward and counts nodes whose
    /// value is at least that maximum.
    /// Time: O(n); auxiliary space: O(h).
    /// Example: [3,1,4,3] -> 4 good nodes (each is no smaller than all values
    /// on its root-to-node path). Carry the path maximum into both children.
    [[nodiscard]] static std::size_t countGoodNodes(const Node* root)
    {
        return countGood(root, std::optional<int>{});
    }

    /// Rewires the owned children in preorder so every node has no left child
    /// and its right child is the next preorder node. Preserves node addresses
    /// and values; returns the original root. Time: O(n); auxiliary space:
    /// O(h) recursion stack.
    /// Example: [1,2,5,3,4,null,6] becomes the right chain 1->2->3->4->5->6.
    /// Recursively flatten both subtrees and splice the flattened left chain
    /// between the root and the flattened right chain.
    [[nodiscard]] static Node* flattenPreorder(Node* root)
    {
        flattenSubtree(root);
        return root;
    }

    /// Finds the maximum sum of any nonempty path, where a path may begin and
    /// end at any nodes and follows parent-child edges. Empty tree returns
    /// nullopt; all-negative trees return their largest node value. Throws
    /// overflow_error if the sum is not representable as long long. Time:
    /// O(n); auxiliary space: O(h).
    /// Example: [1,2,3] -> 6 along path 2->1->3. At each node, update the
    /// global best using both positive child gains, but return only one branch.
    [[nodiscard]] static std::optional<long long> maximumPathSum(
        const Node* root)
    {
        if (root == nullptr) {
            return std::nullopt;
        }
        long long best = std::numeric_limits<long long>::lowest();
        (void)maximumGain(root, best);
        return best;
    }

    /// Recursively carries exclusive lower/upper bounds to validate a strict
    /// BST (duplicates are rejected). Time: O(n);
    /// auxiliary space: O(h).
    /// Example: [2,1,3] -> true; [5,1,4,null,null,3,6] -> false. Every node
    /// must remain strictly between all inherited lower and upper bounds.
    [[nodiscard]] static bool isValidBst(const Node* root)
    {
        return validBst(root, std::nullopt, std::nullopt);
    }

    /// Iterative inorder traversal visits strict-BST values in sorted order
    /// and returns the one-based kth smallest value, or nullopt
    /// when k is zero or exceeds the node count. Time: O(n) worst case;
    /// auxiliary space: O(h).
    /// Example: BST [3,1,4,2], k = 1 -> 1. Inorder visits BST values in
    /// ascending order, so stop at the kth visited node.
    [[nodiscard]] static std::optional<int> kthSmallest(
        const Node* root, std::size_t k)
    {
        if (k == 0) {
            return std::nullopt;
        }
        std::vector<const Node*> stack;
        const Node* current = root;
        while (current != nullptr || !stack.empty()) {
            while (current != nullptr) {
                stack.push_back(current);
                current = current->left.get();
            }
            current = stack.back();
            stack.pop_back();
            if (--k == 0) {
                return current->value;
            }
            current = current->right.get();
        }
        return std::nullopt;
    }

    /// Splits the inorder range at each preorder root to build a tree from
    /// preorder and inorder traversals. Values must be
    /// unique, both ranges must contain the same values and have equal length;
    /// otherwise throws invalid_argument. Time: O(n); auxiliary space: O(n).
    /// Example: preorder = [3,9,20,15,7], inorder = [9,3,15,20,7] builds the
    /// tree rooted at 3. Inorder positions split each root's left and right ranges.
    [[nodiscard]] static std::unique_ptr<Node> buildFromPreorderInorder(
        std::span<const int> preorder, std::span<const int> inorder)
    {
        if (preorder.size() != inorder.size()) {
            throw std::invalid_argument("traversal lengths must match");
        }
        std::unordered_map<int, std::size_t> positions;
        positions.reserve(inorder.size());
        for (std::size_t index = 0; index < inorder.size(); ++index) {
            if (!positions.emplace(inorder[index], index).second) {
                throw std::invalid_argument("traversal values must be unique");
            }
        }
        std::unordered_set<int> seen;
        seen.reserve(preorder.size());
        for (int value : preorder) {
            if (!positions.contains(value) || !seen.insert(value).second) {
                throw std::invalid_argument("traversals must contain the same unique values");
            }
        }
        if (seen.size() != positions.size()) {
            throw std::invalid_argument("traversals must contain the same unique values");
        }

        std::size_t preorderIndex = 0;
        return build(preorder, positions, preorderIndex, 0, inorder.size());
    }

    /// Emits preorder values as comma-delimited decimal integers, using
    /// '#' for every null child (including a null root). Time: O(n);
    /// auxiliary space: O(h), excluding the output.
    /// Example: root 1 with children 2 and 3 -> "1,2,#,#,3,#,#". Preorder
    /// writes a value for each node and a marker for each null child.
    [[nodiscard]] static std::string serialize(const Node* root)
    {
        std::string output;
        writePreorder(root, output);
        return output;
    }

    /// Recursively consumes serialize()'s preorder stream; malformed,
    /// truncated, or trailing
    /// tokens throw invalid_argument. Time: O(n); auxiliary space: O(n).
    /// Example: "1,2,#,#,3,#,#" reconstructs the tree with root 1 and
    /// children 2 and 3. Consume preorder tokens recursively until each
    /// subtree's two children have been described.
    [[nodiscard]] static std::unique_ptr<Node> deserialize(std::string_view data)
    {
        if (data.empty() || data.back() == ',') {
            throw std::invalid_argument("serialized tree is empty or ends with a delimiter");
        }
        std::size_t position = 0;
        auto root = readPreorder(data, position);
        if (position != data.size()) {
            throw std::invalid_argument("serialized tree has trailing tokens");
        }
        return root;
    }

private:
    struct SearchResult {
        Node* ancestor;
        bool foundFirst;
        bool foundSecond;
    };

    static SearchResult findCommonAncestor(
        Node* root, const Node* first, const Node* second)
    {
        if (root == nullptr) {
            return {nullptr, false, false};
        }
        const SearchResult left =
            findCommonAncestor(root->left.get(), first, second);
        const SearchResult right =
            findCommonAncestor(root->right.get(), first, second);
        const bool foundFirst =
            root == first || left.foundFirst || right.foundFirst;
        const bool foundSecond =
            root == second || left.foundSecond || right.foundSecond;
        Node* ancestor = left.ancestor != nullptr
                             ? left.ancestor
                             : right.ancestor;
        if (ancestor == nullptr && foundFirst && foundSecond) {
            ancestor = root;
        }
        return {ancestor, foundFirst, foundSecond};
    }

    static Node* flattenSubtree(Node* root)
    {
        if (root == nullptr) {
            return nullptr;
        }
        Node* leftTail = flattenSubtree(root->left.get());
        Node* rightTail = flattenSubtree(root->right.get());
        if (root->left != nullptr) {
            auto right = std::move(root->right);
            root->right = std::move(root->left);
            leftTail->right = std::move(right);
        }
        return rightTail != nullptr ? rightTail
                                    : (leftTail != nullptr ? leftTail : root);
    }

    static long long maximumGain(const Node* node, long long& best)
    {
        if (node == nullptr) {
            return 0;
        }
        const long long left = std::max(0LL, maximumGain(node->left.get(), best));
        const long long right =
            std::max(0LL, maximumGain(node->right.get(), best));
        const long long through = addPathChecked(
            addPathChecked(static_cast<long long>(node->value), left), right);
        best = std::max(best, through);
        return addPathChecked(static_cast<long long>(node->value),
                              std::max(left, right));
    }

    static long long addPathChecked(long long first, long long second)
    {
        if ((second > 0 &&
             first > std::numeric_limits<long long>::max() - second) ||
            (second < 0 &&
             first < std::numeric_limits<long long>::min() - second)) {
            throw std::overflow_error("maximum path sum does not fit in long long");
        }
        return first + second;
    }

    static std::size_t heightAndDiameter(
        const Node* node, std::size_t& longest)
    {
        if (node == nullptr) {
            return 0;
        }
        const std::size_t left = heightAndDiameter(node->left.get(), longest);
        const std::size_t right = heightAndDiameter(node->right.get(), longest);
        longest = std::max(longest, left + right);
        return 1 + std::max(left, right);
    }

    static int balancedHeight(const Node* node)
    {
        if (node == nullptr) {
            return 0;
        }
        const int left = balancedHeight(node->left.get());
        if (left < 0) {
            return -1;
        }
        const int right = balancedHeight(node->right.get());
        if (right < 0 || left - right > 1 || right - left > 1) {
            return -1;
        }
        return 1 + std::max(left, right);
    }

    static bool containsIdentity(const Node* root, const Node* target)
    {
        return root != nullptr &&
               (root == target || containsIdentity(root->left.get(), target) ||
                containsIdentity(root->right.get(), target));
    }

    static std::size_t countGood(
        const Node* node, std::optional<int> pathMaximum)
    {
        if (node == nullptr) {
            return 0;
        }
        const bool good = !pathMaximum || node->value >= *pathMaximum;
        const int maximum = pathMaximum
                                ? std::max(node->value, *pathMaximum)
                                : node->value;
        return static_cast<std::size_t>(good) +
               countGood(node->left.get(), maximum) +
               countGood(node->right.get(), maximum);
    }

    static bool validBst(
        const Node* node, std::optional<long long> lower,
        std::optional<long long> upper)
    {
        if (node == nullptr) {
            return true;
        }
        const long long value = node->value;
        return (!lower || value > *lower) && (!upper || value < *upper) &&
               validBst(node->left.get(), lower, value) &&
               validBst(node->right.get(), value, upper);
    }

    static std::unique_ptr<Node> build(
        std::span<const int> preorder,
        const std::unordered_map<int, std::size_t>& positions,
        std::size_t& preorderIndex, std::size_t left, std::size_t right)
    {
        if (left == right) {
            return nullptr;
        }
        if (preorderIndex >= preorder.size()) {
            throw std::invalid_argument("traversal orders are inconsistent");
        }
        const int value = preorder[preorderIndex++];
        const std::size_t middle = positions.at(value);
        if (middle < left || middle >= right) {
            throw std::invalid_argument("traversal orders are inconsistent");
        }
        auto node = std::make_unique<Node>(value);
        node->left = build(preorder, positions, preorderIndex, left, middle);
        node->right = build(preorder, positions, preorderIndex, middle + 1, right);
        return node;
    }

    static void writePreorder(const Node* node, std::string& output)
    {
        if (!output.empty()) {
            output.push_back(',');
        }
        if (node == nullptr) {
            output.push_back('#');
            return;
        }
        output += std::to_string(node->value);
        writePreorder(node->left.get(), output);
        writePreorder(node->right.get(), output);
    }

    static std::unique_ptr<Node> readPreorder(
        std::string_view data, std::size_t& position)
    {
        const std::size_t end = data.find(',', position);
        const std::size_t tokenEnd =
            end == std::string_view::npos ? data.size() : end;
        const std::string_view token = data.substr(position, tokenEnd - position);
        if (token.empty()) {
            throw std::invalid_argument("serialized tree contains an empty token");
        }
        position = end == std::string_view::npos ? data.size() : end + 1;

        if (token == "#") {
            return nullptr;
        }
        int value = 0;
        const auto [parsedEnd, error] =
            std::from_chars(token.data(), token.data() + token.size(), value);
        if (error != std::errc{} || parsedEnd != token.data() + token.size()) {
            throw std::invalid_argument("serialized tree contains an invalid value");
        }

        auto node = std::make_unique<Node>(value);
        if (position >= data.size()) {
            throw std::invalid_argument("serialized tree is truncated");
        }
        node->left = readPreorder(data, position);
        if (position >= data.size()) {
            throw std::invalid_argument("serialized tree is truncated");
        }
        node->right = readPreorder(data, position);
        return node;
    }
};

} // namespace dsa::trees
