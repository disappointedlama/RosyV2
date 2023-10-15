#include "RosyV2.hpp"
U64 lookedAt = 0ULL;
U64 mates = 0ULL;
U64 captures = 0ULL;
U64 en_passant = 0ULL;
U64 castles = 0ULL;
U64 promotions = 0ULL;
void printTimingInfo(Position& pos) {
#if timingPosition
    std::cout << "Time spend:\nPawn generation: " << ((double)pos.pawnGeneration / pos.totalTime) * 100.0 << "%\n";
    std::cout << "Sliding generation: " << ((double)pos.slidingGeneration / pos.totalTime) * 100.0 << "%\n";
    std::cout << "Pinned generation: " << ((double)pos.PinnedGeneration / pos.totalTime) * 100.0 << "%\n";
    std::cout << "Move making: " << ((double)pos.moveMaking / pos.totalTime) * 100.0 << "%\n";
    std::cout << "Move unmaking: " << ((double)pos.moveUnmaking / pos.totalTime) * 100.0 << "%\n";
#endif
}
void tree(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, Position& pos, const int depth, const int ind, std::vector<unsigned long long>* nodes_ptr) {
    const int number_of_moves = pos.get_legal_moves(moves[move_index]); 
    if (depth == 0) {
        (nodes_ptr->at(ind)) += 1;
        mates += (number_of_moves == 0);
    }
    if (depth > 0) {
        for (int i = 0; i < number_of_moves; i++) {
            if (depth == 1) {
                if (get_capture_flag(moves[move_index][i])) {
                    captures++;
                    if (get_enpassant_flag(moves[move_index][i])) en_passant++;
                }
                else if (get_castling_flag(moves[move_index][i])) castles++;
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
            else if (get_castling_flag(moves[move_index][i])) castles++;
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
    std::cout << "\tTime: " << totalTime / 1000000000.0 << "s (" << 1000.0 * total_nodes / totalTime << " MHz)\n";
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
    printTimingInfo(pos);
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(moves, 0, pos, 4);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    printTimingInfo(pos);
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(moves, 0, pos, 5);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    printTimingInfo(pos);
    lookedAt = 0;
    mates = 0;
    pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    perf(moves, 0, pos, 6);
    std::cout << "Positions: " << lookedAt << "\nMates: " << mates << "\n";
    printTimingInfo(pos);
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
    printTimingInfo(pos);
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
    printTimingInfo(pos);
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
    printTimingInfo(pos);
    pos = Position{ "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1" };
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
    printTimingInfo(pos);
    if (passedAllTests) {
        std::cout<< "All Tests passed" << std::endl;
    }
    else {
        std::cout << "Not all Tests passed" << std::endl;
    }
}
#define testingMoveGen false
int main()
{
    try {
#if testingMoveGen
    //test();
    position_test();
#else
        Engine rosy{false};
        rosy.uci_loop();
#endif
    }
    catch(Position_Error e) {
        std::cout << e.what() << std::endl;
        std::cout << "Whoopsie" << std::endl;
    }
    return 0;
}

/*
OLD ISSUE POSITIONS
8/2k4p/1b6/3P3p/p7/5K1P/P4P2/5q2 w - - 0 39
position startpos moves e2e4 d7d5 e4d5 d8d5 b1c3 d5f5 d2d4 g8f6 f1d3 f5a5 d1f3 b8c6 d3b5 c8d7 g1e2 e8c8 c1e3 a7a6 b5c6 d7c6 d4d5 c6d5

POTENTIAL ISSUE POSITIONS
7R/3Pk1pp/8/p7/1p6/PP6/1B3PPP/6K1 b - - 2 33
1B6/5p1p/8/6P1/6PP/3k1K2/4p3/8 w - - 0 48
*/
/*
ISSUE POSITION
position fen 1r2k2r/p3pp1p/2p3p1/1pn5/8/1BP2P1P/P1P2P2/3R1RK1 b k - 3 17
illegal move h1h3

position fen 3rk1Q1/p1p1n3/2pb3B/8/3p4/1PPB4/P7/1K2R3 b - - 0 33
illegal move e7g8
position fen 2r4k/8/1p3n2/p4P1p/P1B4P/1P3K2/8/4R3 w - - 5 51
illegal move f1g2 and g1f3

position startpos moves d2d4 e7e6 c2c4 g8f6 g1f3 d7d5 b1c3 c7c6 c1g5 b8d7 e2e3 d8a5 f3d2 d5c4 g5f6 d7f6 d2c4 a5c7 a1c1 f8e7 g2g3 e8g8 f1g2 f8d8 e1g1 b7b6 c4e5 c8b7 c3b5 c7c8 e5c6 b7c6 g2c6 a8b8 b5a7 c8a6 d1a4 a6a4 c6a4 d8f8 a7c6
NOT YET REPLICATED
position startpos moves d2d4 g8f6 c1f4 c7c5 e2e3 d8b6 b1c3 d7d6 f1b5 c8d7 a2a4 d7b5 a4b5 c5d4 e3d4 e7e6 g1f3 d6d5 e1g1 f8d6 c3a4 b6c7 f4d6 c7d6 a4c5 b7b6 c5a4 b8d7 a4c3 d7f8 a1a6 f8g6 d1a1 e8g8 a6a7 a8a7 a1a7 e6e5 d4e5 g6e5 f3e5 d6e5 a7b6 f8c8 b6e3 e5d6 f1d1 d6h2 g1h2 f6g4 h2g3 g4e3 f2e3 f7f5 b5b6 h7h5 d1d5 c8b8 d5d6 g7g5 c3d5 g8g7 d6d7 g7h6 b6b7 f5f4 e3f4 g5f4 d5f4 h5h4 g3g4
*/