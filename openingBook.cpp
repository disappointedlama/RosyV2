#include "openingBook.hpp"
std::istream& operator>>(std::istream& is, book_entry& en)
{
    is >> en.key;
    is >> en.entry[0].move;
    is >> en.entry[0].weight;
    is >> en.entry[1].move;
    is >> en.entry[1].weight;
    is >> en.entry[2].move;
    is >> en.entry[2].weight;
    is >> en.entry[3].move;
    is >> en.entry[3].weight;
    is >> en.entry[4].move;
    is >> en.entry[4].weight;
    return is;
}
std::ostream& operator<<(std::ostream& os, const book_entry& en)
{
    os << en.key << " "
        << en.entry[0].move << " "
        << en.entry[0].weight << " "
        << en.entry[1].move << " "
        << en.entry[1].weight << " "
        << en.entry[2].move << " "
        << en.entry[2].weight << " "
        << en.entry[3].move << " "
        << en.entry[3].weight << " "
        << en.entry[4].move << " "
        << en.entry[4].weight << "\n";
    return os;
}