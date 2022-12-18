#include "RosyV2.hpp"
int main()
{
	Position pos{};
	pos.print();
	pos.parse_fen("1k6/1qp5/8/8/5Q2/8/1RK4r/8 w - - 14 34");
	pos.print();
	return 0;
}