#include"position.cpp"
#include"position.hpp"
#include"tuneData.hpp"
#include"move.cpp"
#include <cmath>
#include<fstream>
U64 lookedAt = 0ULL;
U64 mates = 0ULL;
U64 captures = 0ULL;
U64 en_passant = 0ULL;
U64 castles = 0ULL;
U64 promotions = 0ULL;
void printTimingInfo(Position& pos) {
#define timingPosition false
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
    std::array<U64, 6> expected_nodes{ 48,2039,97862 ,4085603,193690690,8031647685 };
    std::array<U64, 6> expected_mates{ 0, 0, 1, 43, 30171, 360003 };
    std::array<U64, 6> expected_captures{ 8, 351, 17102, 757163, 35043416, 1558445089 };
    std::array<U64, 6> expected_castles{ 2, 91, 3162, 128013, 4993637, 184513607 };
    std::array<U64, 6> expected_promotions{ 0, 0, 0, 15172, 8392, 56627920 };
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
    expected_nodes = std::array<U64, 6>{ 14, 191, 2812, 43238, 674624, 11030083 };
    expected_mates = std::array<U64, 6>{ 0, 0, 0, 17, 0, 2733 };
    expected_captures = std::array<U64, 6>{ 1, 14, 209, 3348, 52051, 940350 };
    expected_castles = std::array<U64, 6>{ 0, 0, 0, 0, 0, 0 };
    expected_promotions = std::array<U64, 6>{ 0, 0, 0, 0, 0, 7552 };
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
    expected_nodes = std::array<U64, 6>{ 6, 264, 9467, 422333, 15833292, 706045033 };
    expected_mates = std::array<U64, 6>{ 0, 0, 22, 5, 50562, 81076 };
    expected_captures = std::array<U64, 6>{ 0, 87, 1021, 131393, 2046173, 210369132 };
    expected_castles = std::array<U64, 6>{ 0, 6, 0, 7795, 0, 10882006 };
    expected_promotions = std::array<U64, 6>{ 0, 48, 120, 60032, 329464, 81102984 };
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
size_t getTotalTunePositionCount(){
    size_t ret{0};
    std::ifstream file{};
    file.open("./tuneData2.txt");
    if (file.is_open()) {
        std::string fen = "";
        int goal = 0;
        while (file >> fen >> goal) {
            ++ret;
        }
        file.close();
    }
    return ret;
}
void tune() {
    size_t total_positions{getTotalTunePositionCount()};
    std::cout<<"TOTAL POSITIONS: "<<total_positions<<std::endl;
    std::array<double,16> gradient{};
    Position pos;
    double previous_relative_err = 100000000000;
    double total_relative_err = 100000000000;
    int iterations = 0;
    const double h{0.01};
    while (previous_relative_err > total_relative_err || iterations==0) {
        std::cout << "Iteration: " << ++iterations << std::endl;
        U64 count = 0;
        previous_relative_err = total_relative_err;
        total_relative_err = 0;
        std::ifstream file{};
        file.open("./tuneData2.txt");
        if (file.is_open()) {
            std::string fen = "";
            int goal = 0;
            while (file >> fen >> goal) {
                if (goal == 0) continue;
                ++count;
                if (count % 100000 == 0 && count) {
                    std::cout << "count: " << count << " ("<<100*(double)count/total_positions<< "%)\n";
                    std::cout << "average relative error so far: " << (std::sqrt(total_relative_err/ count)) << std::endl;
                }
                std::string trueFen = "";
                for (int i = 0; i < fen.length(); ++i) {
                    if (fen[i] != '?') {
                        trueFen += fen[i];
                    }
                    else {
                        trueFen += ' ';
                    }
                }
                fen = trueFen;
                pos = Position{ fen };
                const int current = pos.evaluate(false,false);
                const int diff = goal - current;
                const double abs_diff = std::max(diff,-diff);
                double relative_error = (current) / (double)(goal);
                relative_error = std::max(relative_error, -relative_error);
                total_relative_err+=relative_error*relative_error;
                //std::cout << relative_error << std::endl;
                for (int i = 0; i < pos.wheights.size(); ++i) {
                    if(i==4) continue;
                    pos.wheights[i]+=h;
                    int after_change = pos.evaluate(false,false);
                    double factor{(after_change-current)/h * (1-2*(abs_diff<std::max(goal-after_change,after_change-goal)))};
                    pos.wheights[i]-=h;
                    gradient[i]-=factor;
                }
                //for (int i = 0; i < pos.wheights.size(); i++) {
                //	std::cout << gradient[i] << ",";
                //}
                //std::cout << std::endl;
                //if (count == 10000) break;
            }
            file.close();
        }
        total_relative_err=std::sqrt(total_relative_err);
        for(int i=0;i<gradient.size();++i){
            gradient[i]=(!(gradient[i]<0 && pos.wheights[i]==0)) * gradient[i];
        }
        double abs{0};
        for (int i = 0; i < gradient.size(); ++i) {
            abs+=gradient[i]*gradient[i];
        }
        abs=std::sqrt(abs);
        std::cout<<"ABSOLUTE VALUE OF GRADIENT "<<abs<<"\n";
        std::cout<<"Normalized gradient\n";
        for (int i = 0; i < gradient.size(); i++) {
            gradient[i]/=count;
            if(i) std::cout<<",";
            std::cout<<gradient[i];
        }
        std::cout<<std::endl;
        std::cout << total_relative_err << std::endl;
        std::cout << "average relative error: " << (total_relative_err / std::sqrt(count)) << std::endl;
        for (int i = 0; i < pos.wheights.size(); i++) {
            pos.wheights[i] += gradient[i];
            pos.wheights[i] = std::max(0.0,pos.wheights[i]);
            std::cout << pos.wheights[i] << ",";
            gradient[i] = 0;
        }
        std::cout << std::endl;
    }
}
int main(){
    generateTuneData();
    //tune();
    return 0;
}