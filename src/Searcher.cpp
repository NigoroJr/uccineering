#include "Searcher.h"

/* Constructors, destructor, and assignment operator {{{ */
Searcher::Searcher() {
}

Searcher::Searcher(std::ifstream& ifs) {
    tp_table.populate(ifs);
}

Searcher::Searcher(const Searcher& other)
    : root{other.root}
    , best_moves{other.best_moves}
    , ordered_moves{other.ordered_moves}
    , tp_table{other.tp_table}
{
}

Searcher::Searcher(Searcher&& other)
    : root{std::move(other.root)}
    , best_moves{std::move(other.best_moves)}
    , ordered_moves{std::move(other.ordered_moves)}
    , tp_table{std::move(other.tp_table)}
{
}

Searcher::~Searcher() {
}

Searcher& Searcher::operator=(const Searcher& other) {
    root = other.root;
    best_moves = other.best_moves;
    ordered_moves = other.ordered_moves;
    tp_table = other.tp_table;

    return *this;
}

Searcher& Searcher::operator=(Searcher&& other) {
    root = std::move(other.root);
    best_moves = std::move(other.best_moves);
    ordered_moves = std::move(other.ordered_moves);
    tp_table = std::move(other.tp_table);

    return *this;
}
/* }}} */

Node Searcher::search(const DomineeringState& state,
        const unsigned depth_limit) {
    if (move_thread.joinable()) {
        move_thread.join();
    }
    // Initialize best moves
    best_moves.resize(depth_limit + 1);
    std::fill(best_moves.begin(), best_moves.end(), Node());

    AlphaBeta ab(AlphaBeta::NEG_INF, AlphaBeta::POS_INF);
    search_under(root, ab, state, depth_limit);
    move_thread = std::thread(&Searcher::move_order, this, root.team);

    return best_moves.front();
}

void Searcher::search_under(const Node& parent,
                            AlphaBeta ab,
                            const DomineeringState& current_state,
                            const unsigned depth_limit) {
    // Base case
    if (parent.depth >= depth_limit) {
        best_moves[parent.depth] = parent;
        best_moves[parent.depth].set_score(evaluate(current_state));
        return;
    }

    std::vector<Node> children;
    auto ordered = ordered_moves.find(current_state);
    // Use move-ordered children if possible
    if (parent.depth == 0 && ordered != ordered_moves.end()) {
        children = ordered->second;
        ordered_moves.clear();
    }
    else {
        children = expand(parent, current_state);
    }

    Node& current_best = best_moves[parent.depth];

    // `parent' is a terminal node
    if (children.empty()) {
        current_best = parent;
        current_best.set_as_terminal(current_state);
        return;
    }

    /*
     * Reset the score to POS_INF or NEG_INF depending on which team this node
     * belongs to.
     */
    current_best.set_score(parent.team == Who::HOME
                           ? AlphaBeta::NEG_INF
                           : AlphaBeta::POS_INF);

    DomineeringState next_state{current_state};
    next_state.togglePlayer();

    // create a branch queue vector for every possible future root node
    for (Node& child : children) {
        // Update board to simulate placing the child.
        // Done so that we don't need to make a copy of state for each child.
        tap(child, next_state);

        // bool found;
        // Check for transpositions that were already explored
        // std::tie(result, found) = tp_table.check(next_state);
        // if (!found) {
            // Recursive call
            search_under(child, ab, next_state, depth_limit);
            // Add result to transposition table
        //     tp_table.insert(next_state, result);
        // }

        // Rewind to board before placing the child
        untap(child, next_state);

        const Node& next_move{best_moves[parent.depth + 1]};
        const Evaluator::score_t result = next_move.score();
        child.set_score(result);

        if (next_move.is_terminal()) {
            current_best = child;
            current_best.set_as_terminal(next_state);
            return;
        }

        // The move that our opponent made (our starting point) is at depth 0.
        // We make the best move at depth 1. Our opponent will make one of the
        // moves at depth 2. Thus, we want to store the depth-3 children at
        // depth 2, which will be our next moves, and move order them so that
        // we maximize pruning.
        if (parent.depth == 2) {
            ordered_moves[current_state].push_back(child);
        }

        bool result_better = parent.team == Who::HOME
            ? result > current_best.score()
            : result < current_best.score();
        if (result_better || current_best.is_unset) {
            current_best = child;

            ab.update_if_needed(result, parent.team);
            if (ab.can_prune(result, parent.team)) {
                return;
            }
        }
    }

    return;
}

Evaluator::score_t Searcher::evaluate(const DomineeringState& state) {
    // A copy of the state so that we can mark places temporarily and pass
    // that around to various evaluators
    DomineeringState state_copy{state};
    Evaluator::score_t total = 0;
    for (auto&& p : evaluators) {
        auto&& eval_func = p.first;
        auto&& factor_func = p.second;
        total += factor_func(state) * eval_func(&state_copy);
    }

    return total;
}

void Searcher::cleanup() {
    if (move_thread.joinable()) {
        move_thread.join();
    }
}

/* Private methods */

std::vector<Node> Searcher::expand(const Node& parent,
        const DomineeringState& current_state) {
    // Toggle player
    Who child_team = parent.team == Who::HOME ? Who::AWAY : Who::HOME;
    unsigned child_depth = parent.depth + 1;
    std::vector<Node> children;

    // Create one instance and modify the values so that we don't have to
    // instantiate a ton of vectors in GameMove constructor.
    DomineeringMove parent_move{0, 0, 0, 0};
    for (unsigned r1 = 0; r1 < current_state.ROWS; r1++) {
        for (unsigned c1 = 0; c1 < current_state.COLS; c1++) {
            // Home places horizontally, Away places vertically
            unsigned r2 = parent.team == Who::HOME ? r1 : r1 + 1;
            unsigned c2 = parent.team == Who::HOME ? c1 + 1 : c1;

            parent_move.setMv(r1, c1, r2, c2);

            if (current_state.moveOK(parent_move)) {
                // Note: my_move is HOW I got to this state i.e. parent's move
                children.push_back(Node(child_team,
                                        child_depth,
                                        Location(parent_move)));
            }
        }
    }

    return children;
}

void Searcher::move_order(Who team) {
    for (auto& p : ordered_moves) {
        auto& moves = p.second;
        if (team == Who::HOME) {
            std::sort(moves.begin(), moves.end(), std::greater<Node>());
        }
        else {
            std::sort(moves.begin(), moves.end(), std::less<Node>());
        }
    }
}

void Searcher::tap(const Node& node, DomineeringState& state) {
    // To reflect the parent's team, flip the symbols
    char c = node.team == Who::HOME ? state.AWAYSYM : state.HOMESYM;
    state.setCell(node.parent_move.r1, node.parent_move.c1, c);
    state.setCell(node.parent_move.r2, node.parent_move.c2, c);
}

void Searcher::untap(const Node& node, DomineeringState& state) {
    char c = state.EMPTYSYM;
    state.setCell(node.parent_move.r1, node.parent_move.c1, c);
    state.setCell(node.parent_move.r2, node.parent_move.c2, c);
}

/* vim: tw=78:et:ts=4:sts=4:sw=4 */
