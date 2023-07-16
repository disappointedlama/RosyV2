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
    if (depth > 0) {
        for (int i = 0; i < number_of_moves; i++) {
            if (depth == 1) {
                if (get_capture_flag(moves[move_index][i])) {
                    captures++;
                    if (get_enpassant_flag(moves[move_index][i])) en_passant++;
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
        if (depth == 1) {
            if (get_capture_flag(moves[move_index][i])) {
                captures++;
                if (get_enpassant_flag(moves[move_index][i])) en_passant++;
            }
            if (get_castling_flag(moves[move_index][i])) castles++;
            if (get_promotion_type(moves[move_index][i]) != 15) promotions++;
        }
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
void reset_test_parameters() {
    lookedAt = 0;
    mates = 0;
    captures = 0ULL;
    en_passant = 0ULL;
    castles = 0ULL;
    promotions = 0ULL;
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
    bool passedAllTests = true;
    Position pos{ "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0" };
    std::array<std::array<unsigned int, 128>, 40> moves{};
    std::array<int, 6> expected_nodes{ 48,2039,97862 ,4085603,193690690,8031647685 };
    std::array<int, 6> expected_mates{ 0, 0, 1, 43, 30171, 360003 };
    std::array<int, 6> expected_captures{ 8, 351, 17102, 757163, 35043416, 1558445089 };
    std::array<int, 6> expected_castles{ 2, 91, 3162, 128013, 4993637, 184513607 };
    std::array<int, 6> expected_promotions{ 0, 0, 0, 15172, 8392, 56627920 };
    for (int i = 0; i < 4; i++) {
        reset_test_parameters();
        std::string out = "";
        perf(moves, 0, pos, i + 1);
        std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
        std::cout << "captures: " << captures << "\n";
        std::cout << "en_passant: " << en_passant << "\n";
        std::cout << "castles: " << castles << "\n";
        std::cout << "promotions: " << promotions << "\n";
        const bool correct = lookedAt == expected_nodes[i] && mates == expected_mates[i] && captures==expected_captures[i] && castles==expected_castles[i] && promotions==expected_promotions[i];
        passedAllTests &= correct;
        out = (correct) ? ("Passed Test") : ("Failed Test");
        std::cout << out << "\n";
    }
    pos = Position{ "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0" };
    expected_nodes = std::array<int, 6>{ 14, 191, 2812, 43238, 674624, 11030083 };
    expected_mates = std::array<int, 6>{ 0, 0, 0, 17, 0, 2733 };
    expected_captures = std::array<int, 6>{ 1, 14, 209, 3348, 52051, 940350 };
    expected_castles = std::array<int, 6>{ 0, 0, 0, 0, 0, 0 };
    expected_promotions = std::array<int, 6>{ 0, 0, 0, 0, 0, 7552 };
    for (int i = 0; i < 6; i++) {
        reset_test_parameters();
        std::string out = "";
        perf(moves, 0, pos, i + 1);
        std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
        std::cout << "captures: " << captures << "\n";
        std::cout << "en_passant: " << en_passant << "\n";
        std::cout << "castles: " << castles << "\n";
        std::cout << "promotions: " << promotions << "\n";
        const bool correct = lookedAt == expected_nodes[i] && mates == expected_mates[i] && captures == expected_captures[i] && castles == expected_castles[i] && promotions == expected_promotions[i];
        passedAllTests &= correct;
        out = (correct) ? ("Passed Test") : ("Failed Test");
        std::cout << out << "\n";
    }
    pos = Position{ "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1" };
    expected_nodes = std::array<int, 6>{ 6, 264, 9467, 422333, 15833292, 706045033 };
    expected_mates = std::array<int, 6>{ 0, 0, 22, 5, 50562, 81076 };
    expected_captures = std::array<int, 6>{ 0, 87, 1021, 131393, 2046173, 210369132 };
    expected_castles = std::array<int, 6>{ 0, 6, 0, 7795, 0, 10882006 };
    expected_promotions = std::array<int, 6>{ 0, 48, 120, 60032, 329464, 81102984 };
    for (int i = 0; i < 5; i++) {
        reset_test_parameters();
        std::string out = "";
        perf(moves, 0, pos, i + 1);
        std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
        std::cout << "captures: " << captures << "\n";
        std::cout << "en_passant: " << en_passant << "\n";
        std::cout << "castles: " << castles << "\n";
        std::cout << "promotions: " << promotions << "\n";
        const bool correct = lookedAt == expected_nodes[i] && mates == expected_mates[i] && captures == expected_captures[i] && castles == expected_castles[i] && promotions == expected_promotions[i];
        passedAllTests &= correct;
        out = (correct) ? ("Passed Test") : ("Failed Test");
        std::cout << out << "\n";
    }
    pos = Position{ "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 " };
    for (int i = 0; i < 5; i++) {
        reset_test_parameters();
        std::string out = "";
        perf(moves, 0, pos, i + 1);
        std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
        std::cout << "captures: " << captures << "\n";
        std::cout << "en_passant: " << en_passant << "\n";
        std::cout << "castles: " << castles << "\n";
        std::cout << "promotions: " << promotions << "\n";
        const bool correct = lookedAt == expected_nodes[i] && mates == expected_mates[i] && captures == expected_captures[i] && castles == expected_castles[i] && promotions == expected_promotions[i];
        passedAllTests &= correct;
        out = (correct) ? ("Passed Test") : ("Failed Test");
        std::cout << out << "\n";
    }
    if (passedAllTests) {
        std::cout<< "All Tests passed" << std::endl;
    }
    else {
        std::cout << "Not all Tests passed" << std::endl;
    }
}
int main()
{
    //test();
    //position_test();
    try {
        Engine rosy{false};
        rosy.uci_loop();
    }
    catch(std::exception e) {
        std::cout << e.what() << std::endl;
        std::cout << "Whoopsie" << std::endl;
    }
    return 0;
}

/*
OLD ISSUE POSITIONS
8/2k4p/1b6/3P3p/p7/5K1P/P4P2/5q2 w - - 0 39

POTENTIAL ISSUE POSITIONS
7R/3Pk1pp/8/p7/1p6/PP6/1B3PPP/6K1 b - - 2 33
1B6/5p1p/8/6P1/6PP/3k1K2/4p3/8 w - - 0 48
*/
/*
ISSUE POSITION
FEN
8/8/5K2/3q3p/7k/8/5p2/4Q3 b - - 1 93
position startpos moves c2c4 b8c6 b1c3 e7e5 g1f3 g8f6 g2g3 f8c5 f1g2 d7d6 e1g1 e8g8 d2d3 c8e6 c3a4 c5b4 a2a3 b4a5 b2b4 a5b6 b4b5 c6e7 a4b6 c7b6 d1d2 h7h6 f1e1 a7a6 b5a6 b7a6 f3h4 a8c8 g2b7 c8b8 b7a6 e7c6 h4f3 e5e4 d3e4 f6e4 d2e3 e4c5 a6b5 c6a5 c1b2 a5c4 b5c4 e6c4 e3c3 d8f6 c3c4 f6b2 e1b1 b2f6 c4d5 f8e8 b1c1 e8e2 h2h4 c5e4 d5d3 e2f2 d3e4 f2f3 e4g4 h6h5 c1c8 b8c8 g4c8 g8h7 c8c2 g7g6 g1g2 f3g3 g2g3 f6a1 a3a4 a1e5 g3g2 h7g7 c2d2 e5e4 g2g1 e4a4 d2b2 f7f6 b2b6 a4g4 g1f1 g4f3 f1g1 f3g3 g1f1 g3f4 f1g2 f4e4 g2g1 e4e5 b6c7 g7h6 c7d8 e5g3 g1f1 g3f3 f1e1 f3e3 e1d1 e3d3 d1c1 d3c3 c1b1 c3b3 b1c1 b3c4 c1b2 c4d4 b2b3 d4d3 b3b4 d3e4 b4b5 e4h4 d8d6 h4f2 d6f8 h6g5 f8a3 f2e2 b5b6 e2e6 b6b7 e6d5 b7c8 g5f4 a3c1 f4f5 c1c2 f5g5 c2c1 g5g4 c1g1 g4f5 g1f2 f5e6 f2e3 d5e5 e3b3 e6f5 b3d3 f5g5 d3d2 g5g4 d2g2 g4f5 c8d7 e5e6 d7d8 e6e3 g2c2 f5g5 c2g2 g5h6 g2c6 e3e5 c6a8 f6f5 a8a3 h6g5 d8d7 e5d5 d7e7 f5f4 a3c3 g5h4 e7f6 f4f3 f6g6 d5e6 g6g7 e6d5 g7f6 f3f2 c3e1 f2e1
*/