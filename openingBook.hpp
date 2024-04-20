#pragma once
#include "bitboard.hpp"
#include <array>
#include <vector>
#include <fstream>
#include <random>
using std::array,std::vector;

struct move_weight_pair {
    int move = 0;
    int weight = 0;
    constexpr move_weight_pair() {
    }
    constexpr move_weight_pair(int t_move, int t_weight) {
        move = t_move;
        weight = t_weight;
    }
    constexpr bool is_null() {
        return (move != 0) && (weight != 0);
    }
};
struct book_entry {
    U64 key = 0ULL;
    array<move_weight_pair, 5> entry{};
    constexpr book_entry() { }
    constexpr book_entry(const U64 t_key, array<move_weight_pair, 5> t_entry) {
        key = t_key;
        entry = t_entry;
    }
    constexpr bool operator==(book_entry& other) const {
        return key == other.key;
    }
    constexpr bool operator==(const book_entry& other) const {
        return key == other.key;
    }
    constexpr bool is_null() {
        return (key == 0ULL) && entry[0].is_null() && entry[1].is_null() && entry[2].is_null() && entry[3].is_null() && entry[4].is_null();
    }
};

std::istream& operator>>(std::istream& is, book_entry& en);
std::ostream& operator<<(std::ostream& os, const book_entry& en);

struct OpeningBook {
    OpeningBook() {
        size = 0;
    }
    OpeningBook(string& file_path) {
        load((char*)(file_path.c_str()));
    }
    OpeningBook(const char* file_path) {
        load((char*)file_path);
    }
    OpeningBook(char* file_path) {
        load(file_path);
    }
    void save(char* file_path) {
        std::fstream save;
        save.open(file_path, std::ios::out);
        save << entries.size() << "\n";
        for (int i = 0; i < entries.size(); i++) {
            save << entries[i];
        }
        save.close();
    }
    void load(char* file_path) {
        std::fstream file;
        file.open(file_path, std::ios::in);
        file >> size;
        entries = vector<book_entry>(size);
        for (int i = 0; i < size; i++) {
            book_entry e;
            file >> e;
            entries[i] = e;
        }
        size = entries.size();
        file.close();
    }
    void set_entries(vector<book_entry>& t_entries) {
        entries = t_entries;
        size = entries.size();
    }
    vector<book_entry> get_entries() const {
        return entries;
    }
    constexpr size_t get_size() {
        return size;
    }
    book_entry& operator[](const int ind) {
        return entries[ind];
    }
    void join(OpeningBook other) {
        for (int i = 0; i < other.get_size(); i++) {
            if (std::find(entries.begin(), entries.end(), other[i]) == entries.end()) {
                entries.push_back(other[i]);
            }
        }
        size = entries.size();
    }
    inline book_entry& find(const int key) {
        for (int i = 0; i < size; i++) {
            if (entries[i].key == key) {
                return entries[i];
            }
        }
        return nullentry;
    }
    int find_move(const U64 key) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(1, 100);
        size = entries.size();
        for (int i = 0; i < size; i++) {
            if (entries[i].key == key) {
                const int rand = distr(gen);
                if (rand < entries[i].entry[0].weight) {
                    return entries[i].entry[0].move;
                }
                if (rand < entries[i].entry[1].weight + entries[i].entry[0].weight) {
                    return entries[i].entry[1].move;
                }
                if (rand < entries[i].entry[2].weight + entries[i].entry[1].weight + entries[i].entry[0].weight) {
                    return entries[i].entry[2].move;
                }
                if (rand < entries[i].entry[3].weight + entries[i].entry[2].weight + entries[i].entry[1].weight + entries[i].entry[0].weight) {
                    return entries[i].entry[3].move;
                }
                return entries[i].entry[4].move;
            }
        }
        return 0;
    }
private:
    vector<book_entry> entries{};
    size_t size = 0;
    book_entry nullentry{};
};