#include "RosyV2.hpp"
U64 lookedAt = 0ULL;
U64 mates = 0ULL;
U64 captures = 0ULL;
U64 en_passant = 0ULL;
U64 castles = 0ULL;
U64 promotions = 0ULL;
void tree(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, Position& pos, const int depth, const int ind, std::vector<unsigned long long>* nodes_ptr) {

    const int number_of_moves = pos.get_legal_moves(moves[move_index]); 
    if (depth == 0) {
        (nodes_ptr->at(ind)) += 1;
        mates += (number_of_moves == 0);
    }
    /*
    if (depth == 0) {
        mates += (moves.size() == 0);
        return;
    }
    */
    //print_position(pos);
    if (depth > 0) {
        for (int i = 0; i < number_of_moves; i++) {
            if (depth == 1) {
                if (get_capture_flag(moves[move_index][i])) {
                    captures++;
                    //pos.print();
                    //print_move(moves[move_index][i]);
                }
                if (get_castling_flag(moves[move_index][i])) castles++;
                if (get_promotion_type(moves[move_index][i]) != 15) promotions++;
            }
            pos.make_move(moves[move_index][i]);
            tree(moves, move_index +1, pos, depth - 1, ind, nodes_ptr);
            pos.unmake_move();
        }
    }
}
void perf(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, Position& pos, const int depth) {
    pos.print();
    std::cout << "\tNodes from different branches:\n";
    std::vector<unsigned long long> nodes_from_branches{};
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    const int number_of_moves = pos.get_legal_moves(moves[move_index]);
    unsigned long long total_nodes = 0ULL;
    for (int i = 0; i < number_of_moves; i++) {
        nodes_from_branches.push_back(0ULL);
        pos.make_move(moves[move_index][i]);
        tree(moves, move_index +1, pos, depth - 1, i, &nodes_from_branches);
        pos.unmake_move();
        total_nodes += nodes_from_branches[i];
        std::cout << "\t\t" << uci(moves[move_index][i]) << ": " << nodes_from_branches[i] << " Nodes\n";
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    U64 totalTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "\tTotal Nodes: " << total_nodes << "\n";
    std::cout << "\tTime: " << totalTime << "  (" << 1000000000 * total_nodes / totalTime << " 1/s)\n";
    lookedAt = total_nodes;
}

void test() {
    Position pos;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    std::array<std::array<unsigned int, 128>, 40> moves{};
    perf(moves, 0, pos, 3);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(moves, 0, pos, 4);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(moves, 0, pos, 5);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(moves, 0, pos, 6);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
}
void position_test() {
    Position pos{ "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0" };
    std::array<std::array<unsigned int, 128>, 40> moves{};
    std::array<int, 6> first_nodes{ 48,2039,97862 ,4085603,193690690,8031647685 };
    std::array<int, 6> first_mates{ 0,0,1,43,30171,360003 };
    for (int i = 0; i < 5; i++) {
        std::string out = "";
        perf(moves, 0, pos, i + 1);
        std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
        out = ((lookedAt == first_nodes[i]) && (mates == first_mates[i])) ? ("Passed Test") : ("Failed Test");
        std::cout << "captures: " << captures << "\n";
        std::cout << "en_passant: " << en_passant << "\n";
        std::cout << "castles: " << castles << "\n";
        std::cout << "promotions: " << promotions << "\n";
        std::cout << out << "\n";
        lookedAt = 0;
        mates = 0;
        captures = 0ULL;
        en_passant = 0ULL;
        castles = 0ULL;
        promotions = 0ULL;
    }
    pos = Position{ "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0" };

    std::array<int, 6> second_nodes{ 14,191,2812,43238,674624,11030083 };
    std::array<int, 6> second_mates{ 0,0,0,17,0,2733 };
    for (int i = 0; i < 5; i++) {
        std::string out = "";
        perf(moves, 0, pos, i + 1);
        std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
        out = ((lookedAt == second_nodes[i]) && (mates == second_mates[i])) ? ("Passed Test") : ("Failed Test");
        std::cout << "captures: " << captures << "\n";
        std::cout << "en_passant: " << en_passant << "\n";
        std::cout << "castles: " << castles << "\n";
        std::cout << "promotions: " << promotions << "\n";
        std::cout << out << "\n";
        lookedAt = 0;
        mates = 0;
        captures = 0ULL;
        en_passant = 0ULL;
        castles = 0ULL;
        promotions = 0ULL;
    }
}
int main()
{
    /*
    Position pos{};
    std::array<int, 128> moves;
    int number = pos.get_legal_moves(moves);
    std::cout << number << "\n";
    for (int i = 0; i < 30; i++) {
        print_move(moves[i]);
    }
    */
    //test();
    //print_bitboard(137438953472);
    //Position pos{ "8/8/8/K7/1R3p1k/6P1/8/8 b - - 0 1" };
    //std::array<int, 128> moves;
    //int number=pos.get_legal_moves(moves);
    //for (int i = 0; i < number; i++) {
    //    print_move(moves[i]);
    //}
    //position_test();
    //rosy.parse_position("2k3r1/8/1q6/8/8/8/5PBP/7K b - - 0 1");
    //rosy.bestMove();
    //simd_tests();
    //print_bitboard(Position::infinity);
    try {
        Engine rosy{false};
        //rosy.set_max_depth(7);
        //rosy.set_debug(true);
        rosy.uci_loop();
    }
    catch(std::exception e) {
        std::cout << "Whoopsie" << std::endl;
    }
    return 0;
}
//1648673 1/s is current move gen speed
//position fen 2k3r1/8/1q6/8/8/8/5PBP/7K b - - 0 1 moves b6b1 g2f1 b1f1
//position startpos moves e2e4