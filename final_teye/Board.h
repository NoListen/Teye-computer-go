#include <set>
#include <bitset>
#define FIXED_SIZE 169


struct Board
{
	int board[FIXED_SIZE];
	int next_stone[FIXED_SIZE];
	int final_status[FIXED_SIZE];
	int ko_i;
	int ko_j;
	int last;
	int uf_set[FIXED_SIZE];
	std::set<int> string_lib[FIXED_SIZE];
	std::bitset<FIXED_SIZE> pos2play[2];
	//int string_lib[FIXED_SIZE];
	Board() // Pos is passed by cpboard
	{
		ko_i = ko_j = last = -1;
	};
	Board(const Board &another_board)
	{
		for (int i = 0; i < FIXED_SIZE; ++i)
		{
			board[i] = another_board.board[i];
			next_stone[i] = another_board.next_stone[i];
			string_lib[i] = another_board.string_lib[i];
			uf_set[i] = another_board.uf_set[i];
		}
		pos2play[0] |= another_board.pos2play[0];
		pos2play[1] |= another_board.pos2play[1];

		ko_i = another_board.ko_i;
		ko_j = another_board.ko_j;
		last = another_board.last;

	}
	~Board(){};
};