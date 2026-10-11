#pragma once

#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace dsa::intervals {

/// An interval with integer endpoints and start <= end.
///
/// Whether the endpoints are inclusive is decided by each algorithm and
/// documented on it: merging, insertion and balloons treat intervals as
/// closed [start, end]; meeting scheduling and overlap removal treat them as
/// half-open [start, end).
struct Interval {
    int start{};
    int end{};

    friend constexpr bool operator==(const Interval&, const Interval&) = default;
};

/// Static solutions for interval interview problems (checklist 48-53).
///
/// No method modifies its input; methods that need ordering sort a copy.
/// std::invalid_argument is thrown for malformed intervals (start > end).
class Intervals final {
public:
    Intervals() = delete;

    /// Problem 48 - Merge Intervals.
    ///
    /// Policy: CLOSED intervals, so intervals that merely touch ([1,2] and
    /// [2,3]) are merged into [1,3]. Input may be unsorted. Returns disjoint
    /// intervals sorted by start. Empty input gives empty output.
    ///
    /// Technique: sort by start, then sweep, extending the last result
    /// interval while the next start <= its end.
    /// Time: O(n log n). Extra space: O(n) for the sorted copy and output.
    /// Example: [[1,3],[2,6],[8,10],[15,18]] -> [[1,6],[8,10],[15,18]].
    /// Once sorted, extend the current output interval whenever ranges touch.
    [[nodiscard]] static std::vector<Interval> mergeIntervals(
        std::span<const Interval> intervals)
    {
        std::vector<Interval> sorted = validatedCopy(intervals, "mergeIntervals");
        std::ranges::sort(sorted, byStart);

        std::vector<Interval> merged;
        merged.reserve(sorted.size());
        for (const Interval& current : sorted) {
            if (!merged.empty() && current.start <= merged.back().end) {
                merged.back().end = std::max(merged.back().end, current.end);
            } else {
                merged.push_back(current);
            }
        }
        return merged;
    }

    /// Problem 49 - Insert Interval.
    ///
    /// Policy: CLOSED intervals; the new interval absorbs every interval it
    /// overlaps or touches. Precondition: existing intervals are sorted by
    /// start and pairwise disjoint and non-touching (prev.end < next.start);
    /// otherwise std::invalid_argument is thrown. The newcomer may lie before,
    /// after, or around all existing intervals. The result has the same
    /// invariants.
    ///
    /// Technique: copy intervals ending before the newcomer, merge the
    /// overlapping run into it, copy the rest.
    /// Time: O(n). Extra space: O(n) for the output.
    /// Example: intervals = [[1,2],[6,9]], new = [2,5] -> [[1,5],[6,9]].
    /// Copy earlier ranges, absorb every touching range, then append the rest.
    [[nodiscard]] static std::vector<Interval> insertInterval(
        std::span<const Interval> intervals, Interval newInterval)
    {
        requireValid(newInterval, "insertInterval");
        for (std::size_t i = 0; i < intervals.size(); ++i) {
            requireValid(intervals[i], "insertInterval");
            if (i > 0 && intervals[i - 1].end >= intervals[i].start) {
                throw std::invalid_argument(
                    "insertInterval: intervals must be sorted and disjoint");
            }
        }

        std::vector<Interval> result;
        result.reserve(intervals.size() + 1);
        std::size_t i = 0;
        while (i < intervals.size() && intervals[i].end < newInterval.start) {
            result.push_back(intervals[i++]);
        }
        while (i < intervals.size() && intervals[i].start <= newInterval.end) {
            newInterval.start = std::min(newInterval.start, intervals[i].start);
            newInterval.end = std::max(newInterval.end, intervals[i].end);
            ++i;
        }
        result.push_back(newInterval);
        while (i < intervals.size()) {
            result.push_back(intervals[i++]);
        }
        return result;
    }

    /// Problem 50 - Non-overlapping Intervals.
    ///
    /// Policy: HALF-OPEN [start, end); intervals that only touch ([1,2],[2,3])
    /// do not overlap. Returns the minimum number of intervals to remove so the
    /// rest are pairwise non-overlapping. Intervals must be non-empty
    /// (start < end) or std::invalid_argument is thrown. Identical intervals
    /// overlap each other.
    ///
    /// Technique: greedy interval scheduling - sort by end and keep every
    /// interval starting at or after the last kept end; the earliest end
    /// leaves the most room (exchange argument).
    /// Time: O(n log n). Extra space: O(n) for the sorted copy.
    /// Example: [[1,2],[2,3],[3,4],[1,3]] -> 1 removal. Keep the compatible
    /// interval with the earliest end, maximizing room for later meetings.
    [[nodiscard]] static std::size_t eraseOverlapIntervals(
        std::span<const Interval> intervals)
    {
        std::vector<Interval> sorted = validatedCopy(intervals, "eraseOverlapIntervals");
        requireNonEmptyIntervals(sorted, "eraseOverlapIntervals");
        std::ranges::sort(sorted, byEnd);

        std::size_t removed = 0;
        bool haveKept = false;
        int lastEnd = 0;
        for (const Interval& current : sorted) {
            if (!haveKept || current.start >= lastEnd) {
                haveKept = true;
                lastEnd = current.end;
            } else {
                ++removed;
            }
        }
        return removed;
    }

    /// Problem 51 - Meeting Rooms (can one person attend all meetings).
    ///
    /// Policy: HALF-OPEN [start, end); a meeting may start exactly when the
    /// previous one ends. Meetings must be non-empty (start < end) or
    /// std::invalid_argument is thrown. No meetings (or one) is attendable.
    ///
    /// Technique: sort by start; any adjacent pair with next.start < prev.end
    /// is a conflict.
    /// Time: O(n log n). Extra space: O(n) for the sorted copy.
    /// Example: [[0,30],[5,10],[15,20]] -> false. In start order, any meeting
    /// starting before its predecessor ends creates a conflict.
    [[nodiscard]] static bool canAttendMeetings(std::span<const Interval> meetings)
    {
        std::vector<Interval> sorted = validatedCopy(meetings, "canAttendMeetings");
        requireNonEmptyIntervals(sorted, "canAttendMeetings");
        std::ranges::sort(sorted, byStart);
        for (std::size_t i = 1; i < sorted.size(); ++i) {
            if (sorted[i].start < sorted[i - 1].end) {
                return false;
            }
        }
        return true;
    }

    /// Problem 52 - Meeting Rooms II (minimum rooms).
    ///
    /// Policy: HALF-OPEN [start, end); a room freed at time t can host a
    /// meeting starting at t. Meetings must be non-empty (start < end) or
    /// std::invalid_argument is thrown. Returns 0 for no meetings, otherwise
    /// the maximum number of simultaneously running meetings.
    ///
    /// Technique: sort starts and ends separately and sweep; a start at or
    /// after the earliest unreleased end reuses that room, else a room is added.
    /// Time: O(n log n). Extra space: O(n).
    /// Example: [[0,30],[5,10],[15,20]] -> 2 rooms. Reuse a room when the
    /// next start is at least the earliest current meeting end.
    [[nodiscard]] static std::size_t minMeetingRooms(std::span<const Interval> meetings)
    {
        const std::vector<Interval> checked = validatedCopy(meetings, "minMeetingRooms");
        requireNonEmptyIntervals(checked, "minMeetingRooms");

        std::vector<int> starts;
        std::vector<int> ends;
        starts.reserve(checked.size());
        ends.reserve(checked.size());
        for (const Interval& meeting : checked) {
            starts.push_back(meeting.start);
            ends.push_back(meeting.end);
        }
        std::ranges::sort(starts);
        std::ranges::sort(ends);

        std::size_t rooms = 0;
        std::size_t released = 0;
        for (const int start : starts) {
            if (start >= ends[released]) {
                ++released;
            } else {
                ++rooms;
            }
        }
        return rooms;
    }

    /// Problem 53 - Minimum Number of Arrows to Burst Balloons.
    ///
    /// Policy: CLOSED intervals [start, end] (balloon horizontal extents); an
    /// arrow at x bursts every balloon with start <= x <= end, so balloons
    /// that merely touch ([1,2],[2,3]) share one arrow. Returns the minimum
    /// arrows; 0 for no balloons. No subtraction is performed, so extreme int
    /// endpoints cannot overflow.
    ///
    /// Technique: greedy - sort by end, shoot at the first unburst balloon's
    /// end, which is the rightmost position that still hits it.
    /// Time: O(n log n). Extra space: O(n) for the sorted copy.
    /// Example: [[10,16],[2,8],[1,6],[7,12]] -> 2 arrows. Shoot at each
    /// first unburst balloon's end, the rightmost point that still hits it.
    [[nodiscard]] static std::size_t minArrowsToBurstBalloons(
        std::span<const Interval> balloons)
    {
        std::vector<Interval> sorted = validatedCopy(balloons, "minArrowsToBurstBalloons");
        std::ranges::sort(sorted, byEnd);

        std::size_t arrows = 0;
        int arrowPosition = 0;
        for (const Interval& balloon : sorted) {
            if (arrows == 0 || balloon.start > arrowPosition) {
                ++arrows;
                arrowPosition = balloon.end;
            }
        }
        return arrows;
    }

private:
    [[nodiscard]] static bool byStart(const Interval& a, const Interval& b) noexcept
    {
        return a.start != b.start ? a.start < b.start : a.end < b.end;
    }

    [[nodiscard]] static bool byEnd(const Interval& a, const Interval& b) noexcept
    {
        return a.end != b.end ? a.end < b.end : a.start < b.start;
    }

    static void requireValid(const Interval& interval, const char* caller)
    {
        if (interval.start > interval.end) {
            throw std::invalid_argument(std::string(caller) +
                                        ": interval start must not exceed end");
        }
    }

    [[nodiscard]] static std::vector<Interval> validatedCopy(
        std::span<const Interval> intervals, const char* caller)
    {
        for (const Interval& interval : intervals) {
            requireValid(interval, caller);
        }
        return {intervals.begin(), intervals.end()};
    }

    static void requireNonEmptyIntervals(
        const std::vector<Interval>& intervals, const char* caller)
    {
        for (const Interval& interval : intervals) {
            if (interval.start == interval.end) {
                throw std::invalid_argument(std::string(caller) +
                                            ": half-open intervals must be non-empty");
            }
        }
    }
};

}  // namespace dsa::intervals
