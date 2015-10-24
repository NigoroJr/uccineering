#include "Searcher.h"

/* Constructors, destructor, and assignment operator {{{ */
Searcher::Searcher() {
}

Searcher::Searcher(const Searcher& other)
    : root{other.root}
    , best_moves{other.best_moves}
{
}

Searcher::Searcher(const Searcher&& other)
    : root{std::move(other.root)}
    , best_moves{std::move(other.best_moves)}
{
}

Searcher::~Searcher() {
}

Searcher& Searcher::operator=(const Searcher& other) {
    root = other.root;
    best_moves = other.best_moves;

    return *this;
}
/* }}} */

Node Searcher::search(const DomineeringState& state,
        const unsigned depth_limit) {
    best_moves.resize(depth_limit + 1);
    std::fill(best_moves.begin(), best_moves.end(), Node());

    AlphaBeta ab(AlphaBeta::NEG_INF, AlphaBeta::POS_INF);
    search_under(root, ab, state, depth_limit);

    return best_moves.front();
}

double Searcher::search_under(const Node& parent, AlphaBeta ab,
        const DomineeringState& current_state, const unsigned depth_limit) {
    // Base case
    if (parent.depth >= depth_limit) {
        return evaluate(current_state);
    }
    std::vector<Node> expanded;
    auto exp = ordered_moves.find(std::hash<DomineeringState>()(current_state));
    if (parent.depth == 0 && exp != ordered_moves.end()) {
        expanded = exp->second;
    } else {
        expanded = expand(parent, current_state);
    }

    Node& current_best = best_moves[parent.depth];

    // `parent' is a terminal node
    if (expanded.empty()) {
        current_best.set_as_terminal();
        // POS_INF or NEG_INF
        return current_best.score();
    }
    DomineeringState next_state(current_state);
    next_state.togglePlayer();

    // create a branch queue vector for every possible future root node
    for (const Node& child : expanded) {
        // Update board to simulate placing the child.
        // Done so that we don't need to make a copy of state for each child.
        tap(child, next_state);

        // Recursive call
        double result = search_under(child, ab, next_state, depth_limit);

        // Rewind to board before placing the child
        untap(child, next_state);

        // Found path to a terminal state
        if (result == AlphaBeta::POS_INF || result == AlphaBeta::NEG_INF) {
            current_best = child;
            current_best.set_as_terminal();
            // POS_INF or NEG_INF
            return current_best.score();
        }

        if (result > current_best.score() || current_best.is_unset) {
            // TODO: Change best_moves to vector<vector<Node>> and push to that?
            current_best = child;
            current_best.set_score(result);

            ab.update_if_needed(result, parent.team);

            if (can_prune(current_best, ab)) {
                break;
            }
        }
    }
    if (parent.depth == 2) {
        ordered_moves[std::hash<DomineeringState>()(current_state)] = expanded;
    }

    return current_best.score();
}

/* Private methods */

std::vector<Node> Searcher::expand(const Node& parent,
        const DomineeringState& state) {
    // Player has been toggled already
    Who my_team = state.getWho();
    unsigned my_depth = parent.depth + 1;
    std::vector<Node> children;

    // Create one instance and modify the values so that we don't have to
    // instantiate a ton of vectors in GameMove constructor.
    DomineeringMove my_move(0, 0, 0, 0);
    for (unsigned r1 = 0; r1 < state.ROWS; r1++) {
        for (unsigned c1 = 0; c1 < state.COLS; c1++) {
            // Home places horizontally, Away places vertically
            unsigned r2 = my_team == Who::HOME ? r1 : r1 + 1;
            unsigned c2 = my_team == Who::HOME ? c1 + 1 : c1;

            my_move.setMv(r1, c1, r2, c2);

            if (state.moveOK(my_move)) {
                children.push_back(Node(my_team, my_depth, Location(my_move)));
            }
        }
    }

    return children;
}

void Searcher::move_order() {
    for (auto&& moves : ordered_moves) {
        std::sort(moves.second.begin(), moves.second.end(), Searcher::ScoreSort());
    }
}

bool Searcher::can_prune(const Node& node, const AlphaBeta& ab) {
    if (node.is_unset) {
        return false;
    }

    return node.team == Who::HOME
        ? node.score() >= ab.beta
        : node.score() <= ab.alpha;
}

void Searcher::tap(const Node& node, DomineeringState& state) {
    char c = node.team == Who::HOME ? state.HOMESYM : state.AWAYSYM;
    state.setCell(node.location.r1, node.location.c1, c);
    state.setCell(node.location.r2, node.location.c2, c);
}

void Searcher::untap(const Node& node, DomineeringState& state) {
    char c = state.EMPTYSYM;
    state.setCell(node.location.r1, node.location.c1, c);
    state.setCell(node.location.r2, node.location.c2, c);
}

double Searcher::evaluate(const DomineeringState& state) {
    double total = 0;
    for (auto&& p : evaluators) {
        auto&& eval_func = p.first;
        auto&& factor_func = p.second;
        total += factor_func(state) * eval_func(state);
    }

    return total;
}

/* vim: tw=78:et:ts=4:sts=4:sw=4 */
