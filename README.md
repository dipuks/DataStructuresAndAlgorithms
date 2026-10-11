# Data Structures and Algorithms

A C++23 interview-preparation project built around a 140-problem coding
interview checklist.
Solutions are organized by problem-solving topic. Each solution documents its
API, assumptions, algorithm, and time and space complexity.

All 140 checklist problems are implemented in topic-based C++23 headers, with
public API contracts, algorithm notes, Big-O complexity, and tests.

## Build and run

Requirements: CMake 3.20 or newer and a C++23 compiler (for example, the Visual
Studio 2026 C++ workload, a recent GCC, or a recent Clang toolchain).

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the small demonstration program with:

```text
build/dsa_demo
```

On Windows, the executable is normally at `build\Debug\dsa_demo.exe` for a
multi-configuration generator such as Visual Studio.

## Repository layout

```text
include/dsa/   Reusable, topic-based solution APIs
tests/         Dependency-free example and edge-case tests
```

Each topic lives in its own header and namespace. This keeps solutions easy to
reuse and test without tying them to a particular online judge's `Solution`
class or input/output format.

## How each solution is documented

Each public solution API documents the problem it solves, input assumptions and
edge cases, a representative input/output example, the key algorithmic
invariant or decision, and time and auxiliary-space complexity. Read the
example first, trace the stated invariant through the implementation, then
compare the result with the corresponding test.

Source comments explain meaningful algorithm steps and non-obvious decisions.
They intentionally do not repeat punctuation, braces, or straightforward
syntax line by line: such comments quickly become stale and make the algorithm
harder to scan. Every API is documented at its declaration, with inline
comments reserved for the implementation details that benefit from extra
clarification.

## Recommended study order

Follow the numbered order first: it moves from array manipulation and common
linear-time patterns toward graph/tree traversal, exhaustive search, stateful
dynamic programming, and greedy proofs. Within a topic, compare problems that
look similar but need different invariants:

- **Two pointers vs. sliding window:** sorted-pair or inward-moving boundaries
  versus a maintained contiguous range.
- **Binary search vs. answer-space search:** locating a value versus finding a
  feasible optimum with a monotone predicate.
- **Stacks vs. heaps:** nearest unresolved element versus repeatedly selecting
  the next priority item.
- **BFS vs. DFS:** shortest paths in unweighted graphs versus reachability,
  component exploration, and tree recursion.
- **Backtracking vs. dynamic programming:** enumerate candidate choices versus
  reuse results for overlapping subproblems.
- **Greedy vs. dynamic programming:** justify a locally optimal choice versus
  compare alternatives across states.

Do not treat a passing example as a proof. Be ready to explain the invariant
and why it is preserved, what makes the algorithm terminate, and what each
input element or state contributes to the complexity.

## Interview-preparation steps

For each problem, practice the following routine:

1. Restate the task and clarify input limits, duplicates, ordering, and edge
   cases.
2. Work a small example by hand and describe a straightforward baseline.
3. Identify the useful pattern (for example, a hash table, sliding window, or
   binary search) and explain why it applies.
4. State the key invariant before coding; use it to justify correctness.
5. Implement the documented API and walk through a normal case and a boundary
   case.
6. Give time and auxiliary-space complexity in Big-O notation, including the
   assumptions behind average-case data-structure costs.
7. Review the code for overflow, empty inputs, and any constraints from the
   problem statement.

Try each problem yourself before reading its implementation. Afterward, explain
the pattern, invariant, complexity, and trade-offs aloud as if in an interview.
Revisit missed problems after a day and again after a week.

The roadmap records implementation coverage, not study completion. After
solving a topic once, revisit it without reading the implementation, then
explain one representative solution under a short interview-style time limit.

## Problem roadmap

Problems are numbered to match the source checklist. Every listed topic has
implementations and tests in this repository.

For the complete, versioned 1–140 problem checklist, see
[PROBLEMS.md](PROBLEMS.md). It records the supplied checklist's problem names
without committing the source PDF.

| # | Topic | Problems | Topic header | Progress |
|---|---|---:|---|---|
| 1 | Arrays, hashing, and prefix sums | 1–14 | `include/dsa/arrays_hashing.hpp` | Implemented |
| 2 | Two pointers | 15–22 | `include/dsa/two_pointers.hpp` | Implemented |
| 3 | Sliding window | 23–33 | `include/dsa/sliding_window.hpp` | Implemented |
| 4 | Binary search and answer space | 34–47 | `include/dsa/binary_search.hpp` | Implemented |
| 5 | Intervals | 48–53 | `include/dsa/intervals.hpp` | Implemented |
| 6 | Stack and monotonic stack | 54–63 | `include/dsa/stack.hpp` | Implemented |
| 7 | Strings: parsing, palindromes, and manipulation | 64–77 | `include/dsa/strings.hpp` | Implemented |
| 8 | Linked lists | 78–85 | `include/dsa/linked_lists.hpp` | Implemented |
| 9 | Trees | 86–99 | `include/dsa/trees.hpp` | Implemented |
| 10 | Graphs | 100–106 | `include/dsa/graphs.hpp` | Implemented |
| 11 | Heaps and priority queues | 107–112 | `include/dsa/heaps.hpp` | Implemented |
| 12 | Backtracking | 113–118 | `include/dsa/backtracking.hpp` | Implemented |
| 13 | Dynamic programming | 119–128 | `include/dsa/dynamic_programming.hpp` | Implemented |
| 14 | Greedy algorithms | 129–140 | `include/dsa/greedy.hpp` | Implemented |

## Problems 1–14: arrays, hashing, and prefix sums

| # | Problem | API |
|---|---|---|
| 1 | Two Sum | `arrays_hashing::ArraysHashing::twoSum` |
| 2 | Contains Duplicate | `arrays_hashing::ArraysHashing::containsDuplicate` |
| 3 | Valid Anagram | `arrays_hashing::ArraysHashing::isAnagram` |
| 4 | Group Anagrams | `arrays_hashing::ArraysHashing::groupAnagrams` |
| 5 | Top K Frequent Elements | `arrays_hashing::ArraysHashing::topKFrequentElements` |
| 6 | Product of Array Except Self | `arrays_hashing::ArraysHashing::productExceptSelf` |
| 7 | Longest Consecutive Sequence | `arrays_hashing::ArraysHashing::longestConsecutiveSequence` |
| 8 | Subarray Sum Equals K | `arrays_hashing::ArraysHashing::subarraySumEqualsK` |
| 9 | Maximum Subarray | `arrays_hashing::ArraysHashing::maximumSubarraySum` |
| 10 | Majority Element | `arrays_hashing::ArraysHashing::majorityElement` |
| 11 | Find the Duplicate Number | `arrays_hashing::ArraysHashing::findDuplicateNumber` |
| 12 | First Missing Positive | `arrays_hashing::ArraysHashing::firstMissingPositive` |
| 13 | Set Matrix Zeroes | `arrays_hashing::ArraysHashing::setMatrixZeroes` |
| 14 | Rotate Image | `arrays_hashing::ArraysHashing::rotateImage` |

## FAQ

### Where should I begin?

Start with the first topic in [PROBLEMS.md](PROBLEMS.md), solve a problem
without looking at the implementation, then compare your approach with the
documented API, invariant, and tests.

### Why are the solutions header-only?

Each solution can be included independently, making it easy to experiment in a
small program or reuse an algorithm without linking a separate library.

### How do I run the tests?

Use the CMake commands in [Build and run](#build-and-run). Each test executable
is focused on one or more topic headers, while the complete CTest run executes
all of them.

### Do the APIs match an online judge's `Solution` class?

No. The library uses reusable C++ APIs such as `std::span`, `std::vector`, and
`std::optional`. Adapt the documented method you need to an online judge's
required wrapper.

### What should I study after an incorrect solution?

Trace the stated invariant against a small counterexample, rerun the relevant
test executable, and then solve the same problem again without reading the
implementation.
