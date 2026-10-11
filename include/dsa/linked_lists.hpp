#pragma once

#include <algorithm>
#include <cstddef>
#include <list>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace dsa::linked_lists {

/// Singly linked list node used by problems 78-83.
///
/// Ownership convention: a list is owned by whoever holds its head pointer,
/// and nodes are allocated with `new` / released with `delete`. Release a
/// list with LinkedLists::destroy. Functions documented as "consuming" take
/// over the nodes of their arguments: the caller must stop using the old
/// head pointers and only use the returned head.
struct ListNode {
    int value;
    ListNode* next;

    explicit ListNode(int v = 0, ListNode* n = nullptr) noexcept : value(v), next(n) {}
};

/// Node with an arbitrary `random` pointer for problem 84.
/// Same ownership convention as ListNode; release with
/// LinkedLists::destroyRandomList.
struct RandomNode {
    int value;
    RandomNode* next;
    RandomNode* random;

    explicit RandomNode(int v = 0) noexcept : value(v), next(nullptr), random(nullptr) {}
};

/// Static solutions for linked-list interview problems (78-85).
///
/// All list arguments must be well-formed: nullptr-terminated with no shared
/// or dangling nodes, unless a method explicitly accepts cycles (problem 82).
/// Passing a cyclic list to any other method is undefined (infinite loop).
class LinkedLists final {
public:
    LinkedLists() = delete;

    /// Builds a new list holding values in order; returns nullptr if empty.
    /// The caller owns the result (release with destroy). Strong exception
    /// safety: nothing leaks if allocation fails.
    /// Time: O(n). Extra space: O(n) for the nodes.
    /// Example: [1,2,3] -> a new 1->2->3 list; an empty span returns nullptr.
    [[nodiscard]] static ListNode* fromValues(std::span<const int> values)
    {
        ListNode* head = nullptr;
        ListNode* tail = nullptr;
        try {
            for (const int value : values) {
                ListNode* node = new ListNode(value);
                (tail ? tail->next : head) = node;
                tail = node;
            }
        } catch (...) {
            destroy(head);
            throw;
        }
        return head;
    }

    /// Copies the values of a list into a vector; the list is not modified.
    /// Time: O(n). Extra space: O(n).
    /// Example: a list 1->2->3 becomes vector [1,2,3]; nodes stay unchanged.
    [[nodiscard]] static std::vector<int> toVector(const ListNode* head)
    {
        std::vector<int> values;
        for (; head != nullptr; head = head->next) {
            values.push_back(head->value);
        }
        return values;
    }

    /// Deletes every node of the list iteratively (no recursion depth issue).
    /// Accepts nullptr. Afterwards all pointers into the list dangle.
    /// Time: O(n). Extra space: O(1).
    /// Example: destroy(1->2->3) deletes each node and leaves no owned list.
    static void destroy(ListNode* head) noexcept
    {
        while (head != nullptr) {
            ListNode* next = head->next;
            delete head;
            head = next;
        }
    }

    /// Deletes every node of a RandomNode list (follows `next` only).
    /// Accepts nullptr. Time: O(n). Extra space: O(1).
    /// Example: releases a RandomNode chain by following next pointers only.
    static void destroyRandomList(RandomNode* head) noexcept
    {
        while (head != nullptr) {
            RandomNode* next = head->next;
            delete head;
            head = next;
        }
    }

    /// Problem 78: Reverse Linked List. Consuming: relinks nodes in place.
    ///
    /// Returns the new head (the old tail); the old head becomes the tail.
    /// nullptr and single-node lists are returned unchanged.
    /// Technique: iterative pointer reversal; invariant: `previous` heads the
    /// reversed prefix. Time: O(n). Extra space: O(1).
    /// Example: 1->2->3 becomes 3->2->1. Save the next node before reversing
    /// the current link, so the unprocessed suffix remains reachable.
    [[nodiscard]] static ListNode* reverseList(ListNode* head) noexcept
    {
        ListNode* previous = nullptr;
        while (head != nullptr) {
            ListNode* next = head->next;
            head->next = previous;
            previous = head;
            head = next;
        }
        return previous;
    }

    /// Problem 79: Merge Two Sorted Lists. Consuming: splices both inputs.
    ///
    /// Both lists must be sorted ascending (not checked). Nodes are reused,
    /// not copied; the merge is stable (ties take from `first`). Either
    /// argument may be nullptr. Do not use the arguments afterwards.
    /// Technique: tail-pointer splice with a stack-allocated sentinel.
    /// Time: O(n+m). Extra space: O(1).
    /// Example: 1->3->5 and 2->4 -> 1->2->3->4->5. Repeatedly splice the
    /// smaller head after the output tail, then append the remaining suffix.
    [[nodiscard]] static ListNode* mergeTwoLists(ListNode* first, ListNode* second) noexcept
    {
        ListNode sentinel;
        ListNode* tail = &sentinel;
        while (first != nullptr && second != nullptr) {
            if (second->value < first->value) {
                tail->next = second;
                second = second->next;
            } else {
                tail->next = first;
                first = first->next;
            }
            tail = tail->next;
        }
        tail->next = first != nullptr ? first : second;
        return sentinel.next;
    }

    /// Problem 80: Reorder List L0->L1->...->Ln-1 into L0->Ln-1->L1->Ln-2...
    ///
    /// Mutates in place; the head node stays the head (so the caller's
    /// pointer remains valid). nullptr and lists of 1-2 nodes are unchanged.
    /// Technique: find middle (slow/fast), reverse second half, interleave.
    /// Time: O(n). Extra space: O(1).
    /// Example: 1->2->3->4->5 becomes 1->5->2->4->3. Split at the middle,
    /// reverse the second half, then alternate nodes from both halves.
    static void reorderList(ListNode* head) noexcept
    {
        if (head == nullptr || head->next == nullptr) {
            return;
        }
        ListNode* slow = head;
        ListNode* fast = head;
        while (fast->next != nullptr && fast->next->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;
        }
        ListNode* second = reverseList(slow->next);
        slow->next = nullptr;
        ListNode* first = head;
        while (second != nullptr) {
            ListNode* firstNext = first->next;
            ListNode* secondNext = second->next;
            first->next = second;
            second->next = firstNext;
            first = firstNext;
            second = secondNext;
        }
    }

    /// Problem 81: Remove Nth Node From End. Consuming: mutates and deletes.
    ///
    /// n is 1-based (1 removes the tail). The removed node is deleted and the
    /// new head returned (it differs from `head` when the head was removed).
    /// If n == 0 or n exceeds the length, std::out_of_range is thrown and the
    /// list is left untouched.
    /// Technique: two pointers separated by n nodes.
    /// Time: O(L) single pass. Extra space: O(1).
    /// Example: 1->2->3->4->5, n = 2 -> 1->2->3->5. Keep two pointers n
    /// nodes apart so the trailing pointer reaches the removal predecessor.
    [[nodiscard]] static ListNode* removeNthFromEnd(ListNode* head, std::size_t n)
    {
        if (n == 0) {
            throw std::out_of_range("n must be at least 1");
        }
        ListNode* ahead = head;
        for (std::size_t i = 0; i < n; ++i) {
            if (ahead == nullptr) {
                throw std::out_of_range("n exceeds list length");
            }
            ahead = ahead->next;
        }
        if (ahead == nullptr) { // removing the head
            ListNode* newHead = head->next;
            delete head;
            return newHead;
        }
        ListNode* behind = head;
        while (ahead->next != nullptr) {
            ahead = ahead->next;
            behind = behind->next;
        }
        ListNode* doomed = behind->next;
        behind->next = doomed->next;
        delete doomed;
        return head;
    }

    /// Problem 82: Linked List Cycle detection. Read-only; the only method
    /// here that accepts cyclic lists. nullptr has no cycle.
    /// Technique: Floyd tortoise and hare. Time: O(n). Extra space: O(1).
    /// Example: 1->2->3->2 (cycle) -> true; the fast pointer eventually
    /// catches the slow pointer inside the cycle.
    [[nodiscard]] static bool hasCycle(const ListNode* head) noexcept
    {
        return findCycleStart(head) != nullptr;
    }

    /// Problem 82 (extension): returns the first node of the cycle, or
    /// nullptr if the list is acyclic. Read-only; the pointer aliases the list.
    /// Technique: Floyd, then restart one pointer from the head.
    /// Time: O(n). Extra space: O(1).
    /// Example: 1->2->3->2 returns the second node (value 2), the cycle entry.
    [[nodiscard]] static const ListNode* findCycleStart(const ListNode* head) noexcept
    {
        const ListNode* slow = head;
        const ListNode* fast = head;
        while (fast != nullptr && fast->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;
            if (slow == fast) {
                const ListNode* probe = head;
                while (probe != slow) {
                    probe = probe->next;
                    slow = slow->next;
                }
                return probe;
            }
        }
        return nullptr;
    }

    /// Problem 83: Add Two Numbers stored as little-endian digit lists.
    ///
    /// Inputs are read-only and may be nullptr (treated as 0). Every digit
    /// must be 0-9, else std::invalid_argument (nothing leaks). Returns a
    /// freshly allocated list the caller owns (release with destroy); the
    /// result for two empty lists is a single node 0 so a number is always
    /// represented. Leading (high-order) zeros of the inputs are preserved
    /// only as far as needed: the result has no extra high-order zeros.
    /// Time: O(max(n,m)). Extra space: O(max(n,m)) for the result.
    /// Example: 2->4->3 plus 5->6->4 -> 7->0->8 (342 + 465 = 807).
    /// Add corresponding digits and carry each tens digit into the next place.
    [[nodiscard]] static ListNode* addTwoNumbers(const ListNode* first, const ListNode* second)
    {
        ListNode* head = nullptr;
        ListNode* tail = nullptr;
        try {
            int carry = 0;
            while (first != nullptr || second != nullptr || carry != 0) {
                int total = carry;
                for (const ListNode** list : {&first, &second}) {
                    if (*list != nullptr) {
                        if ((*list)->value < 0 || (*list)->value > 9) {
                            throw std::invalid_argument("digits must be in [0,9]");
                        }
                        total += (*list)->value;
                        *list = (*list)->next;
                    }
                }
                ListNode* node = new ListNode(total % 10);
                (tail ? tail->next : head) = node;
                tail = node;
                carry = total / 10;
            }
            if (head == nullptr) {
                head = new ListNode(0);
            }
            // Drop high-order zero nodes produced by zero-padded inputs.
            ListNode* lastNonZero = nullptr;
            for (ListNode* node = head; node != nullptr; node = node->next) {
                if (node->value != 0) {
                    lastNonZero = node;
                }
            }
            if (lastNonZero == nullptr) {
                destroy(head->next);
                head->next = nullptr;
            } else {
                destroy(lastNonZero->next);
                lastNonZero->next = nullptr;
            }
        } catch (...) {
            destroy(head);
            throw;
        }
        return head;
    }

    /// Problem 84: Copy List with Random Pointer (deep copy).
    ///
    /// Returns a new list with equal values whose next/random pointers all
    /// point inside the copy; the input is not modified and shares nothing
    /// with the result. The caller owns the copy (destroyRandomList).
    /// A random pointer to a node outside the list throws
    /// std::invalid_argument (nothing leaks).
    /// Technique: map original -> copy, then wire pointers.
    /// Time: O(n) average. Extra space: O(n).
    /// Example: each original node's random pointer to another list node is
    /// redirected to its corresponding copied node, never to the original.
    [[nodiscard]] static RandomNode* copyRandomList(const RandomNode* head)
    {
        std::vector<std::unique_ptr<RandomNode>> copies;
        std::unordered_map<const RandomNode*, RandomNode*> mapping;
        for (const RandomNode* node = head; node != nullptr; node = node->next) {
            copies.push_back(std::make_unique<RandomNode>(node->value));
            mapping.emplace(node, copies.back().get());
        }
        std::size_t index = 0;
        for (const RandomNode* node = head; node != nullptr; node = node->next, ++index) {
            if (index + 1 < copies.size()) {
                copies[index]->next = copies[index + 1].get();
            }
            if (node->random != nullptr) {
                const auto it = mapping.find(node->random);
                if (it == mapping.end()) {
                    throw std::invalid_argument("random pointer leaves the list");
                }
                copies[index]->random = it->second;
            }
        }
        // Ownership transfers to the caller; the vector no longer deletes.
        RandomNode* result = copies.empty() ? nullptr : copies.front().get();
        for (auto& copy : copies) {
            (void)copy.release();
        }
        return result;
    }

    /// Problem 85: LRU Cache with O(1) get and put.
    ///
    /// Holds at most `capacity` int->int entries; touching an entry (get hit
    /// or put) makes it most recently used and exceeding capacity evicts the
    /// least recently used one. Capacity 0 is valid: put stores nothing and
    /// get always misses. Not copyable (internal iterators); movable. Not
    /// thread-safe. Invariant: order_ and index_ hold the same keys.
    /// Time: O(1) average per operation. Extra space: O(capacity).
    /// Example: capacity 2; put(1), put(2), get(1), put(3) evicts key 2.
    class LRUCache final {
    public:
        explicit LRUCache(std::size_t capacity) : capacity_(capacity)
        {
            if (capacity_ <= index_.max_size()) {
                index_.reserve(std::min<std::size_t>(capacity_, 1024));
            }
        }

        LRUCache(const LRUCache&) = delete;
        LRUCache& operator=(const LRUCache&) = delete;
        LRUCache(LRUCache&&) = default;
        LRUCache& operator=(LRUCache&&) = default;

        /// Value for key (marking it most recent) or std::nullopt on a miss.
        /// Example: get(1) returns its value and moves key 1 to the front.
        [[nodiscard]] std::optional<int> get(int key)
        {
            const auto found = index_.find(key);
            if (found == index_.end()) {
                return std::nullopt;
            }
            order_.splice(order_.begin(), order_, found->second);
            return found->second->second;
        }

        /// Inserts or updates key and marks it most recent, evicting the
        /// least recent entry if full. No-op when capacity is 0.
        /// Example: put(3,30) at capacity 2 evicts the least-recent key first.
        void put(int key, int value)
        {
            if (capacity_ == 0) {
                return;
            }
            if (const auto found = index_.find(key); found != index_.end()) {
                found->second->second = value;
                order_.splice(order_.begin(), order_, found->second);
                return;
            }
            if (order_.size() == capacity_) {
                index_.erase(order_.back().first);
                order_.pop_back();
            }
            order_.emplace_front(key, value);
            try {
                index_.emplace(key, order_.begin());
            } catch (...) {
                order_.pop_front();
                throw;
            }
        }

        /// Number of cached entries, from zero through capacity().
        /// Example: after two successful insertions, size() returns 2.
        [[nodiscard]] std::size_t size() const noexcept { return order_.size(); }
        /// Maximum number of entries this cache retains.
        /// Example: a cache constructed with capacity 4 reports 4.
        [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

    private:
        std::size_t capacity_;
        std::list<std::pair<int, int>> order_; // front = most recent
        std::unordered_map<int, std::list<std::pair<int, int>>::iterator> index_;
    };
};

} // namespace dsa::linked_lists