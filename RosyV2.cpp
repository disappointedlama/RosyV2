#include "RosyV2.hpp"
U64 lookedAt = 0ULL;
U64 mates = 0ULL;
void tree(Position& pos, const int depth, const int ind, std::vector<unsigned long long>* nodes_ptr) {
    (nodes_ptr->at(ind)) += (depth == 0);
    std::vector<int> moves{};
    pos.get_legal_moves(moves);
    mates += (depth == 0) * (moves.size() == 0);
    /*
    if (depth == 0) {
        mates += (moves.size() == 0);
        return;
    }
    */
    //print_position(pos);
    if (depth > 0) {
        for (int i = 0; i < moves.size(); i++) {
            pos.make_move(moves[i]);
            tree(pos, depth - 1, ind, nodes_ptr);
            pos.unmake_move();
        }
    }
}
void perf(Position& pos, const int depth) {
    pos.print();
    std::cout << "\tNodes from different branches:\n";
    std::vector<unsigned long long> nodes_from_branches{};
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::vector<int> moves{};
    pos.get_legal_moves(moves);
    unsigned long long total_nodes = 0ULL;
    for (int i = 0; i < moves.size(); i++) {
        nodes_from_branches.push_back(0ULL);
        pos.make_move(moves[i]);
        tree(pos, depth - 1, i, &nodes_from_branches);
        pos.unmake_move();
        total_nodes += nodes_from_branches[i];
        std::cout << "\t\t" << uci(moves[i]) << ": " << nodes_from_branches[i] << " Nodes\n";
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    U64 totalTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "\tTotal Nodes: " << total_nodes << "\n";
    std::cout << "\tTime: " << totalTime << "  (" << 1000000000 * total_nodes / totalTime << " 1/s)\n";
}

void test() {
    Position pos;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(pos, 3);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(pos, 4);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(pos, 5);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(pos, 6);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
}
int main()
{
    test();
    /*
    U64 test = 1ULL + 16ULL;
    int ind = 5;
    std::cout << (int)_bittest64((long long*)&test, ind) << std::endl;
    */
    //Engine rosy{};
    //rosy.set_max_depth(7);
    //rosy.set_debug(true);
    //rosy.uci_loop();
    //rosy.parse_position("2k3r1/8/1q6/8/8/8/5PBP/7K b - - 0 1");
    //rosy.bestMove();
    //simd_tests();
    return 0;
}
//1648673 1/s is current move gen speed
//position fen 2k3r1/8/1q6/8/8/8/5PBP/7K b - - 0 1 moves b6b1 g2f1 b1f1