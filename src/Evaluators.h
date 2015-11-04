#ifndef EVALUATORS_H_
#define EVALUATORS_H_

#include "DomineeringState.h"

#include <functional>
#include <utility>

using DS = DomineeringState;

struct Evaluator {
    using score_t = int;

    static const char MARKEDSYM = '!';

    /**
     * Mark that's used to indicate that a grid is already checked.
     * Note that the `clear_checked' method should be called after all
     * operations are done so that there are no temporary marks on the board
     * after Searcher::evaluate exits.
     *
     * \param[in] r the row to be marked.
     *
     * \param[in] c the column to be marked.
     *
     * \param[out] state the state that contains the board to be modified.
     */
    inline void mark(const int r, const int c, DS& state) {
        if (state.getCell(r, c) != state.EMPTYSYM) {
            return;
        }

        state.setCell(r, c, MARKEDSYM);
    }

    /**
     * Unmarks the checked symbol and revert back to the empty state.
     */
    void clear_marks(DS& state) {
        for (int i = 0; i < state.ROWS; i++) {
            for (int j = 0; j < state.COLS; j++) {
                if (state.getCell(i, j) == MARKEDSYM) {
                    state.setCell(i, j, state.EMPTYSYM);
                }
            }
        }
    }

    /**
     * Checks if the given grid is empty or not.
     * Note that this method does boundary checks first, which returns false
     * if any of the given parameter is invalid.
     */
    inline bool grid_empty(const int r, const int c, const DS& state) const {
        if (r < 0 || r >= state.ROWS || c < 0 || c >= state.COLS) {
            return false;
        }

        return state.getCell(r, c) == state.EMPTYSYM;
    }

    /**
     * Checks if a tile is placable on the given location.
     * A tile (i.e. domino) is placable if both grids are empty. Even though
     * this method does boundary checking, it does not check if the two grids
     * are adjacent to each other.
     */
    bool placable(const int r1, const int c1, const int r2, const int c2,
                  const DS& state) const {
        return grid_empty(r1, c1, state) && grid_empty(r2, c2, state);
    }

    /**
     * Checks if HOME has a reserved spot.
     * Home places dominoes horizontally. For example,
     *
     * +---+---+---+---+---+
     * | H | H | 1 | 2 |
     * +---+---+---+---+---+
     * |   |   | A | A |
     *
     * When 1 and 2 are (r1, c1) and (r2, c2) respectively, this method
     * returns true since the spot is "reserved" for HOME.
     */
    bool reserved_for_home(const int r1,
                           const int c1,
                           const int r2,
                           const int c2,
                           const DS& state) const {
        // Check if 1, 2 (in the docs) are actually empty
        if (!placable(r1, c1, r2, c2, state)) {
            return false;
        }

        bool no_space_above = (!grid_empty(r1 + 1, c1, state) &&
                               !grid_empty(r2 + 1, c2, state));

        bool no_space_below = (!grid_empty(r1 - 1, c1, state) &&
                               !grid_empty(r2 - 1, c2, state));

        return no_space_above && no_space_below;
    }

    /**
     * Checks if the given grid is reserved for AWAY.
     * Away places vertical dominoes.
     */
    bool reserved_for_away(const int r1,
                           const int c1,
                           const int r2,
                           const int c2,
                           const DS& state) const {
        if (!placable(r1, c1, r2, c2, state)) {
            return false;
        }

        bool no_space_to_left = (!grid_empty(r1, c1 - 1, state) &&
                                 !grid_empty(r2, c2 - 1, state));

        bool no_space_to_right = (!grid_empty(r1, c1 + 1, state) &&
                                  !grid_empty(r2, c2 + 1, state));

        return no_space_to_left && no_space_to_right;
    }
};

struct EvalHomeReserved : public Evaluator {
    score_t operator()(DS* state) {
        score_t home_count = 0;

        for (int r = 0; r < state->ROWS; r++) {
            for (int c = 0; c < state->COLS; c++) {
                // Check if HOME has reserved spot here
                // Note: the method does boundary check
                if (reserved_for_home(r, c, r, c + 1, *state)) {
                    home_count++;
                    // Mark grids so that we don't check again
                    mark(r, c, *state);
                    mark(r, c + 1, *state);
                }
            }
        }

        return home_count;
    }
};

struct EvalHomeOpen : public Evaluator {
    score_t operator()(DS* state) {
        score_t home_count = 0;

        for (int c = 0; c < state->COLS; c++) {
            for (int r = 0; r < state->ROWS; r++) {
                // Check if AWAY has reserved spot here
                // Note: the method does boundary check
                if (placable(r, c, r, c + 1, *state)) {
                    home_count++;
                    // Mark grids so that we don't check again
                    mark(r, c, *state);
                    mark(r, c + 1, *state);
                }
            }
        }

        return home_count;
    }
};

struct EvalAwayReserved : public Evaluator {
    score_t operator()(DS* state) {
        score_t away_count = 0;

        for (int c = 0; c < state->COLS; c++) {
            for (int r = 0; r < state->ROWS; r++) {
                // Check if AWAY has reserved spot here
                // Note: the method does boundary check
                if (reserved_for_away(r, c, r + 1, c, *state)) {
                    away_count++;
                    // Mark grids so that we don't check again
                    mark(r, c, *state);
                    mark(r + 1, c, *state);
                }
            }
        }

        return away_count;
    }
};

struct EvalAwayOpen : public Evaluator {
    score_t operator()(DS* state) {
        score_t away_count = 0;

        for (int c = 0; c < state->COLS; c++) {
            for (int r = 0; r < state->ROWS; r++) {
                // Check if AWAY has reserved spot here
                // Note: the method does boundary check
                if (placable(r, c, r + 1, c, *state)) {
                    away_count++;
                    // Mark grids so that we don't check again
                    mark(r, c, *state);
                    mark(r + 1, c, *state);
                }
            }
        }

        return away_count;
    }
};

/**
 * Housekeeping class that clears the marks indicated on the board.
 */
struct ClearMarks : public Evaluator {
    score_t operator()(DS* state) {
        clear_marks(*state);
        return 0;
    }
};

using EvalScore = std::function<Evaluator::score_t(DS*)>;
using EvalFactor = std::function<Evaluator::score_t(const DS&)>;
// vector of pairs to indicate semantics that the order must be followed
static const std::vector<std::pair<EvalScore, EvalFactor>> evaluators = {
    /* Evaluators for HOME */
    {
        // First count the reserved, marking each reserved spot so that we
        // don't double count.
        std::make_pair(EvalHomeReserved(), [](const DS& state) {
                       return 2;
                       })
    },
    {
        // Then count the remaining open spots for HOME
        std::make_pair(EvalHomeOpen(), [](const DS& state) {
                       return 1;
                       })
    },
    {
        // Clear the marks on the board
        std::make_pair(ClearMarks(), [](const DS& state) { return 0; })
    },

    /* Evaluators for AWAY */
    {
        // First count the reserved, marking each reserved spot so that we
        // don't double count.
        std::make_pair(EvalAwayReserved(), [](const DS& state) {
                       return -2;
                       })
    },
    {
        // Then count the remaining open spots for HOME
        std::make_pair(EvalAwayOpen(), [](const DS& state) {
                       return -1;
                       })
    },
    {
        // Clear the marks on the board
        std::make_pair(ClearMarks(), [](const DS& state) { return 0; })
    },
};

#endif /* end of include guard */

/* vim: tw=78:et:ts=4:sts=4:sw=4 */
