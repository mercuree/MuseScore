/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __RENDERMIDI_H__
#define __RENDERMIDI_H__

#include "libmscore/measure.h"
#include "libmscore/synthesizerstate.h"
#include "pitchwheelrenderer.h"

namespace mu::engraving {
class EventMap;
class MasterScore;
class Staff;
class SynthesizerState;

//---------------------------------------------------------
//   RangeMap
///   Helper class to keep track of status of status of
///   certain parts of score or MIDI representation.
//---------------------------------------------------------

class RangeMap
{
    enum class Range {
        BEGIN, END
    };
    std::map<int, Range> status;

public:
    void setOccupied(int tick1, int tick2);
    void setOccupied(std::pair<int, int> range) { setOccupied(range.first, range.second); }

    int occupiedRangeEnd(int tick) const;

    void clear() { status.clear(); }
};

//---------------------------------------------------------
//   MidiRenderer
///   MIDI renderer for a score
//---------------------------------------------------------

class MidiRenderer
{
    Score* score{ nullptr };
    bool needUpdate = true;
    int minChunkSize = 0;

public:
    class Chunk
    {
        int _tickOffset;
        Measure const* first;
        Measure const* last;

    public:
        Chunk(int tickOffset, Measure const* fst, Measure const* lst)
            : _tickOffset(tickOffset), first(fst), last(lst) {}

        Chunk()     // "invalid chunk" constructor
            : _tickOffset(0), first(nullptr), last(nullptr) {}

        operator bool() const {
            return bool(first);
        }
        int tickOffset() const { return _tickOffset; }
        Measure const* startMeasure() const { return first; }
        Measure const* endMeasure() const { return last ? last->nextMeasure() : nullptr; }
        Measure const* lastMeasure() const { return last; }
        int tick1() const { return first->tick().ticks(); }
        int tick2() const { return last ? last->endTick().ticks() : tick1(); }
        int utick1() const { return tick1() + tickOffset(); }
        int utick2() const { return tick2() + tickOffset(); }
    };

private:
    std::vector<Chunk> chunks;

    void updateChunksPartition();
    static bool canBreakChunk(const Measure* last);
    void updateState();

    void renderStaffChunk(const Chunk&, EventMap* events, const Staff* sctx, PitchWheelRenderer& pitchWheelRenderer);

    void renderSpanners(const Chunk&, EventMap* events, PitchWheelRenderer& pitchWheelRenderer);

    void renderMetronome(const Chunk&, EventMap* events);
    void renderMetronome(EventMap* events, Measure const* m, const Fraction& tickOffset);

    void collectMeasureEvents(EventMap* events, Measure const* m, const Staff* sctx, int tickOffset,
                              PitchWheelRenderer& pitchWheelRenderer);

public:
    explicit MidiRenderer(Score* s);

    struct Context
    {
        SynthesizerState synthState;
        bool metronome{ true };

        Context() {}
    };

    void doCollectMeasureEvents(EventMap* events, Measure const* m, const Staff* sctx, int tickOffset,
                                PitchWheelRenderer& pitchWheelRenderer);
    void renderScore(EventMap* events, const Context& ctx);
    void renderChunk(const Chunk&, EventMap* events, const Context& ctx, PitchWheelRenderer& pitchWheelRenderer);

    void setScoreChanged() { needUpdate = true; }
    void setMinChunkSize(int sizeMeasures) { minChunkSize = sizeMeasures; needUpdate = true; }

    static const int ARTICULATION_CONV_FACTOR { 100000 };

    std::vector<Chunk> chunksFromRange(const int fromTick, const int toTick);
};
} // namespace mu::engraving

#endif