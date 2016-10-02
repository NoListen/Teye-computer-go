/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is Brown, a simple go program.                           *
 *                                                               *
 * Copyright 2003 and 2004 by Gunnar Farneb鋍k.                  *
 *                                                               *
 * Permission is hereby granted, free of charge, to any person   *
 * obtaining a copy of this file gtp.c, to deal in the Software  *
 * without restriction, including without limitation the rights  *
 * to use, copy, modify, merge, publish, distribute, and/or      *
 * sell copies of the Software, and to permit persons to whom    *
 * the Software is furnished to do so, provided that the above   *
 * copyright notice(s) and this permission notice appear in all  *
 * copies of the Software and that both the above copyright      *
 * notice(s) and this permission notice appear in supporting     *
 * documentation.                                                *
 *                                                               *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY     *
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE    *
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR       *
 * PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO      *
 * EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS  *
 * NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR    *
 * CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING    *
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF    *
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT    *
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS       *
 * SOFTWARE.                                                     *
 *                                                               *
 * Except as contained in this notice, the name of a copyright   *
 * holder shall not be used in advertising or otherwise to       *
 * promote the sale, use or other dealings in this Software      *
 * without prior written authorization of the copyright holder.  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <fstream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
//#include <vecotr>
#include <algorithm>
#include "teye.h"
#include <vector>
#include <set>
#include <thread>
#include <mutex>
using namespace std;
#define INFINITE 100000

int board_size = 13;
float komi = -3.14;
int ss;
int sim;
int walks = 0;
int stage;
clock_t start_time,finish_time;
float beta_num;
float bias;

bool corners[4];

mutex gmutex;

Board gboard; 

int neighbors[MAX_BOARD*MAX_BOARD][4]; // neighbor

int pr[MAX_BOARD][MAX_BOARD];

int ir[MAX_BOARD*MAX_BOARD];

int jr[MAX_BOARD*MAX_BOARD];


vector<int> eight;

static int deltai[8] = {-1, 1, 0, 0, -1, -1, 1, 1};
static int deltaj[8] = {0, 0, -1, 1, -1, 1, -1, 1};

#define POS(i, j) ((i) * board_size + (j))
#define I(pos) ((pos) / board_size)
#define J(pos) ((pos) % board_size)

#define OTHER_COLOR(color) (WHITE + BLACK - (color))
#define MIN(x,y)	 (x<y?x:y)

int Suicide(int i, int j, int color, Board &board);

static int has_additional_liberty(int orpos, int libpos, Board &board);

void rotation(int &a1,int &a2, int &a3, int &b1,int &b2, int &b3, int &c1 , int &c2, int &c3)
{
    int A1,A2,A3,B1,B2,B3,C1,C2,C3;
    A1 = a1;A2 = a2;A3 = a3;B1 = b1;B2 = b2;B3 = b3;C1 = c1;C2 = c2;C3 = c3;
    c1 = A1;b1 = A2;a1 = A3;a2 = B3;a3 = C3;b3 = C2;c3 = C1;c2 = B1;
}

bool hane_case1(int a1,int a2, int a3, int b1,int b2, int b3, int c1 , int c2, int c3, Board &board)
{
    if ((board.board[a1]==BLACK && board.board[a2]==WHITE && board.board[a3]==BLACK)||
        (board.board[a1]==WHITE && board.board[a2]==BLACK && board.board[a3]==BLACK))
    {
        if (board.board[b1]==EMPTY && board.board[b3]==EMPTY)
            return true;
    }
    return false;
}

bool hane_case2(int a1,int a2, int a3, int b1,int b2, int b3, int c1 , int c2, int c3, Board &board)
{
    bool row11 = (board.board[a1]==BLACK && board.board[a2]==WHITE && board.board[a3]==EMPTY);
    bool row12 = (board.board[a1]==WHITE && board.board[a2]==BLACK && board.board[a3]==EMPTY);
    bool row13 = (board.board[a1]==EMPTY && board.board[a2]==BLACK && board.board[a3]==WHITE);
    bool row14 = (board.board[a1]==EMPTY && board.board[a2]==WHITE && board.board[a3]==BLACK);
    if (row11 || row12 || row13 || row14)
        if (board.board[b1]==EMPTY && board.board[b3]==EMPTY)
            if (board.board[c2]==EMPTY)
                return true;

    return false;
}

bool hane_case3(int a1,int a2, int a3, int b1,int b2, int b3, int c1 , int c2, int c3, Board &board)
{
    bool tworow1 = (board.board[a1]==BLACK && board.board[a2]==WHITE && board.board[b1]==BLACK && board.board[b3]==EMPTY);
    bool tworow2 = (board.board[a1]==WHITE && board.board[a2]==BLACK && board.board[b1]==WHITE && board.board[b3]==EMPTY);
    bool tworow3 = (board.board[a3]==BLACK && board.board[a2]==WHITE && board.board[b3]==BLACK && board.board[b1]==EMPTY);
    bool tworow4 = (board.board[a3]==WHITE && board.board[a2]==BLACK && board.board[b3]==WHITE && board.board[b1]==EMPTY);
    if (tworow1 || tworow2 || tworow3 || tworow4)
        if (board.board[c2]==EMPTY)
            return true;
    return false;
}

bool hane_case4(int a1,int a2, int a3, int b1,int b2, int b3, int c1 , int c2, int c3, int color, Board &board)
{
    bool row11 = (board.board[a1]==BLACK && color == BLACK && board.board[a2]==WHITE && board.board[a3]==WHITE);
    bool row12 = (board.board[a1]==WHITE && color == WHITE && board.board[a2]==BLACK && board.board[a3]==BLACK);
    bool row13 = (board.board[a1]==BLACK && color == WHITE && board.board[a2]==BLACK && board.board[a3]==WHITE);
    bool row14 = (board.board[a1]==WHITE && color == BLACK && board.board[a2]==WHITE && board.board[a3]==BLACK);
    if (row11 || row12 || row13 || row14)
        if (board.board[b1]==EMPTY && board.board[b3]==EMPTY)
            if(board.board[c2]==EMPTY)
                return true;
    return false;
}
// before into this fun, posi (i,j) is sure to have no stone and on_board.
bool hane_pattern(int i , int j, int color, Board &board)
{
    // no rotation
    //  symmetry has been considered in the single function.
    if (!(i>0 && i<12 && j>0 && j<12))
    return false;
    //gtp_printf("hane ");

    // a1 a2 a3
    // b1 ij b3
    // c1 c2 c3
    int a1,a2,a3,b1,b2,b3,c1,c2,c3;
    a1 = pr[i-1][j-1];
    a2 = pr[i-1][j];
    a3 = pr[i-1][j+1];
    b1 = pr[i][j-1];
    b2 = pr[i][j];
    b3 = pr[i][j+1];
    c1 = pr[i+1][j-1];
    c2 = pr[i+1][j];
    c3 = pr[i+1][j+1];
    int total_rotate = 4;
    while(total_rotate--)
    {
    if (hane_case1(a1,a2,a3,b1,b2,b3,c1,c2,c3,board))
        return true;
    if (hane_case2(a1,a2,a3,b1,b2,b3,c1,c2,c3,board))
        return true;
    if (hane_case3(a1,a2,a3,b1,b2,b3,c1,c2,c3,board))
        return true;
    if (hane_case4(a1,a2,a3,b1,b2,b3,c1,c2,c3,color,board))
        return true;
    rotation(a1,a2, a3, b1,b2, b3,c1 ,c2, c3);
    }
    // rotate first time anti clockwise

    return false;
}
////////////////////////

bool cut1_case1(int a1,int a2, int a3, int b1,int b2, int b3, int c1 , int c2, int c3, Board &board)
{
    bool tworow1 = board.board[a1]== BLACK && board.board[a2]==WHITE && board.board[b1]==WHITE ;
    bool anti_tworow1 = (board.board[b3]==WHITE && board.board[c2]==EMPTY) || (board.board[b3]==EMPTY && board.board[c2]==WHITE);
    // color change
    bool tworow2 = board.board[a1]== WHITE && board.board[a2]==BLACK && board.board[b1]==BLACK;
    bool anti_tworow2 = (board.board[b3]==BLACK && board.board[c2]==EMPTY) || (board.board[b3]==EMPTY && board.board[c2]==BLACK);
    // symmetry
    int tmp = a1;
    a1 = a3;
    a3 = tmp;
    tmp = b1;
    b1 = b3;
    b3 = tmp;
    bool tworow3 = board.board[a1]== BLACK && board.board[a2]==WHITE && board.board[b1]==WHITE ;
    bool anti_tworow3 = (board.board[b3]==WHITE && board.board[c2]==EMPTY) || (board.board[b3]==EMPTY && board.board[c2]==WHITE);

    // color change
    bool tworow4 = board.board[a1]== WHITE && board.board[a2]==BLACK && board.board[b1]==BLACK;
    bool anti_tworow4 = (board.board[b3]==BLACK && board.board[c2]==EMPTY) || (board.board[b3]==EMPTY && board.board[c2]==BLACK);

    if ((tworow1 && !anti_tworow1) || (tworow2 && !anti_tworow2) || (tworow3 && !anti_tworow3) || (tworow4 && !anti_tworow4))
        return true;
    return false;
}
bool cut1_pattern(int i , int j, int color, Board &board)
{
    // no rotation
    //  symmetry has been considered in the single function.
    if (!(i>0 && i<12 && j>0 && j<12))
    return false;
    // a1 a2 a3
    // b1 ij b3
    //gtp_printf("cut1 ");
    // c1 c2 c3
    //gtp_printf("cut1\n ");
    int a1,a2,a3,b1,b2,b3,c1,c2,c3;
    a1 = pr[i-1][j-1];
    a2 = pr[i-1][j];
    a3 = pr[i-1][j+1];
    b1 = pr[i][j-1];
    b2 = pr[i][j];
    b3 = pr[i][j+1];
    c1 = pr[i+1][j-1];
    c2 = pr[i+1][j];
    c3 = pr[i+1][j+1];
    int total_rotate = 4;
    while(total_rotate--)
    {
    if (cut1_case1(a1,a2,a3,b1,b2,b3,c1,c2,c3,board))
        return true;
    rotation(a1,a2, a3, b1,b2, b3,c1 ,c2, c3);
    }
    // rotate first time anti clockwise

    return false;
}
/////////////////////////////

bool cut2_case1(int a1,int a2, int a3, int b1,int b2, int b3, int c1 , int c2, int c3, Board &board)
{
    bool tworow1 = board.board[a2]==BLACK && board.board[b1]==WHITE && board.board[b3]==WHITE;
    bool tworow2 = board.board[a2]==WHITE && board.board[b1]==BLACK && board.board[b3]==BLACK;
    bool with_tworow1 = board.board[c1]!=WHITE && board.board[c2]!=WHITE && board.board[c3]!=WHITE;
    bool with_tworow2 = board.board[c1]!=BLACK && board.board[c2]!=BLACK && board.board[c3]!=BLACK;

    if ((tworow1 && with_tworow1)||(tworow2 && with_tworow2))
        return true;
    return false;
}
bool cut2_pattern(int i , int j, int color, Board &board)
{
    // no rotation
    //  symmetry has been considered in the single function.
    if (!(i>0 && i<12 && j>0 && j<12))
    return false;
    // a1 a2 a3
    //gtp_printf("cut2 ");
    // b1 ij b3
    // c1 c2 c3
    //gtp_printf("cut2\n ");
    int a1,a2,a3,b1,b2,b3,c1,c2,c3;
    a1 = pr[i-1][j-1];
    a2 = pr[i-1][j];
    a3 = pr[i-1][j+1];
    b1 = pr[i][j-1];
    b2 = pr[i][j];
    b3 = pr[i][j+1];
    c1 = pr[i+1][j-1];
    c2 = pr[i+1][j];
    c3 = pr[i+1][j+1];
    int total_rotate = 4;
    while(total_rotate--)
    {
    if (cut2_case1(a1,a2,a3,b1,b2,b3,c1,c2,c3,board))
        return true;
    rotation(a1,a2, a3, b1,b2, b3,c1 ,c2, c3);
    }
    // rotate first time anti clockwise

    return false;
}
/////////////////////////////


bool board_case1(int a1,int a2, int a3, int b1,int b2, int b3, int color, Board &board)
{
    bool tworow1 = board.board[a1]==BLACK && board.board[a2]==EMPTY && board.board[b1]==WHITE;
    bool tworow2 = board.board[a1]==WHITE && board.board[a2]==EMPTY && board.board[b1]==BLACK;
    int tmp = a1;
    a1 = a3;
    a3 = tmp;
    tmp = b1;
    b1 = b3;
    b3 = tmp;
    bool tworow3 = board.board[a1]==BLACK && board.board[a2]==EMPTY && board.board[b1]==WHITE;
    bool tworow4 = board.board[a1]==WHITE && board.board[a2]==EMPTY && board.board[b1]==BLACK;
    if (tworow1 || tworow2 || tworow3 || tworow4)
        return true;
    return false;
}

bool board_case2(int a1,int a2, int a3, int b1,int b2, int b3, int color, Board &board)
{
    bool tworow1 = board.board[a2]==BLACK && board.board[b1]!=BLACK && board.board[b3]==WHITE;
    bool tworow2 = board.board[a2]==WHITE && board.board[b1]!=WHITE && board.board[b3]==BLACK;
    int tmp = a1;
    a1 = a3;
    a3 = tmp;
    tmp = b1;
    b1 = b3;
    b3 = tmp;
    bool tworow3 = board.board[a2]==BLACK && board.board[b1]!=BLACK && board.board[b3]==WHITE;
    bool tworow4 = board.board[a2]==WHITE && board.board[b1]!=WHITE && board.board[b3]==BLACK;
    if (tworow1 || tworow2 || tworow3 || tworow4)
        return true;
    return false;
}

bool board_case3(int a1,int a2, int a3, int b1,int b2, int b3, int color, Board &board)
{
    bool tworow1 = board.board[a2]==BLACK && board.board[a3]==WHITE && color==BLACK;
    bool tworow2 = board.board[a2]==WHITE && board.board[a3]==BLACK && color==WHITE;
    int tmp = a1;
    a1 = a3;
    a3 = tmp;
    bool tworow3 = board.board[a2]==BLACK && board.board[a3]==WHITE && color==BLACK;
    bool tworow4 = board.board[a2]==WHITE && board.board[a3]==BLACK && color==WHITE;
    if (tworow1 || tworow2 || tworow3 || tworow4)
        return true;
    return false;
}
bool board_case4(int a1,int a2, int a3, int b1,int b2, int b3, int color, Board &board)
{
    bool tworow1 = board.board[a2]==BLACK && board.board[a3]==WHITE && color==WHITE && board.board[b3]!=BLACK;
    bool tworow2 = board.board[a2]==WHITE && board.board[a3]==BLACK && color==BLACK && board.board[b3]!=WHITE;
    int tmp = a1;
    a1 = a3;
    a3 = tmp;
    tmp = b1;
    b1 = b3;
    b3 = tmp;
    bool tworow3 = board.board[a2]==BLACK && board.board[a3]==WHITE && color==WHITE && board.board[b3]!=BLACK;
    bool tworow4 = board.board[a2]==WHITE && board.board[a3]==BLACK && color==BLACK && board.board[b3]!=WHITE;
    if (tworow1 || tworow2 || tworow3 || tworow4)
        return true;
    return false;
}

bool board_case5(int a1,int a2, int a3, int b1,int b2, int b3, int color, Board &board)
{
    bool tworow1 = board.board[a2]==BLACK && board.board[a3]==WHITE && color==WHITE && board.board[b3]==BLACK && board.board[b1]==WHITE;
    bool tworow2 = board.board[a2]==WHITE && board.board[a3]==BLACK && color==BLACK && board.board[b3]==WHITE && board.board[b1]==BLACK;
    int tmp = a1;
    a1 = a3;
    a3 = tmp;
    tmp = b1;
    b1 = b3;
    b3 = tmp;
    bool tworow3 = board.board[a2]==BLACK && board.board[a3]==WHITE && color==WHITE && board.board[b3]==BLACK && board.board[b1]==WHITE;
    bool tworow4 = board.board[a2]==WHITE && board.board[a3]==BLACK && color==BLACK && board.board[b3]==WHITE && board.board[b1]==BLACK;
    if (tworow1 || tworow2 || tworow3 || tworow4)
        return true;
    return false;
}
bool board_pattern(int i , int j, int color, Board &board)
{
    if (!(i==0 || j==0 || i==board_size-1 || j== board_size-1))
        return false;
    if((i==0 && j==0 ) || (i==0 && j==board_size-1) || (i==board_size-1 && j==0) || (i==board_size-1 && j==board_size-1) )
        return false;
    //gtp_printf("board_case ");
    int a1,a2,a3,b1,b2,b3;
    b2 = pr[i][j];
    if (i==0)
    {
        a1 = pr[i+1][j+1];
        a2 = pr[i+1][j];
        a3 = pr[i+1][j-1];
        b1 = pr[i][j+1];
        b3 = pr[i][j-1];
    }
    else if (i==board_size-1)
    {
        a1 = pr[i-1][j-1];
        a2 = pr[i-1][j];
        a3 = pr[i-1][j+1];
        b1 = pr[i][j-1];
        b3 = pr[i][j+1];
    }
    else if(j==0)
    {
        a1 = pr[i-1][j+1];
        a2 = pr[i][j+1];
        a3 = pr[i+1][j+1];
        b1 = pr[i-1][j];
        b3 = pr[i+1][j];
    }
    else if(j==board_size-1)
    {
        a1 = pr[i+1][j-1];
        a2 = pr[i][j-1];
        a3 = pr[i-1][j-1];
        b1 = pr[i+1][j];
        b3 = pr[i-1][j];
    }

    if ( board_case1(a1,a2,a3,b1,b2,b3,color,board) ||
        board_case2(a1,a2,a3,b1,b2,b3,color,board) ||
        board_case3(a1,a2,a3,b1,b2,b3,color,board) ||
        board_case4(a1,a2,a3,b1,b2,b3,color,board) ||
        board_case5(a1,a2,a3,b1,b2,b3,color,board) )
        return true;
    return false;
}


class Node
{
	public:
		int idx; // index in board
		
		Node* parent;
		//int numOfChildren;
		Node** children; 

		int Size;
		int color;
		//int step; // What is the step of this Node in the tree

		int n;
		int rn;

		//int ki;	// Ko_j in status s
		//int kj; // Ko_i in status s
		double q; // standard update in MCTS
		double rq; // amaf update in RAVE

		int numOfChildren;

		Node(int Coord, Node* Parent, int Color) // Pos is passed by cpboard
		{
			idx = Coord;
			Size = board_size*board_size;
			parent  = Parent;

			q = INFINITE; // initilaize to  infinite Xx 0.5
			n = 0;

			 // May need some heuristics in the later
			rq = 0;
			rn = 0;


			numOfChildren = 0;
			children = NULL; // NULL at first
			color = Color;
		}

		void expand(Board &board, int stage = 0)
		{
			//if (numOfChildren > 0) return;
			if (numOfChildren > 0) return;
			int other_color = OTHER_COLOR(color);
			int idx = 0;
			int cidx = color - 1;
			children = new Node*[Size];
			memset(children,NULL,sizeof(children));
			int ai;
			int aj; // how to maintain them
			
			for (ai = stage; ai < board_size-stage; ai++)
				for (aj = stage; aj < board_size-stage; aj++)
				{
					int pos = pr[ai][aj];

					if (Legal_move(ai, aj, other_color, board) && board.pos2play[cidx][pos] == 1)
					{
						children[numOfChildren++] = new Node(pos, this, other_color);
					}
				}
			if (!numOfChildren) { delete children; children = NULL;}
		
	//		int other_color = OTHER_COLOR(color);
	//		int idx = 0;
	//		
	//		//int ai;
	//		//int aj; // how to maintain them

	//		int store = -1;
	//		int ko_one = -1;
	//		int cidx = color -1;
	//		if (board.ko_i != -1)
	//		{
	//			ko_one = pr[board.ko_i][board.ko_j];
	//			store = board.pos2play[cidx][ko_one];
	//			board.pos2play[cidx][ko_one] = 0;
	//		}
	////gen<<ko_one<<' '<<store<<endl;
	//		int num2play = board.pos2play[cidx].count();

	//		if (num2play == 0) // 0 child
	//		{
	//			//delete children;
	//			children = NULL;
	//			if (ko_one != -1) board.pos2play[cidx][ko_one] = store;
	//			return;
	//		}
	//		children = new Node*[num2play];
	//		memset(children,NULL,sizeof(children));
	//		//int cidx = color - 1;
	//		for (int k = 0 ; k < board_size * board_size; ++k)
	//		{
	//			if (board.pos2play[cidx][k] == 1)
	//			{
	//				children[numOfChildren++] = new Node(k, this, other_color);
	//			}
	//		}
	//		
	//		if (ko_one != -1) board.pos2play[cidx][ko_one] = store;

		}

		~Node() // used to cut
		{
			//delete pos;
			if (numOfChildren > 0)
			{
				for (int i = 0; i < numOfChildren; ++i)
				{
					if (children[i] != NULL)
					{
						delete(children[i]);
						children[i] = NULL;
					}
				}
			/*while (!children.empty())
			{
				delete children.back();
				children.pop_back();
			}
			children.clear();
			children.swap(vector<Node*>());*/
				delete children;
				children = NULL;
			}
			parent = NULL;
		}
};

void mcts(int *xi, int *yj, int color);
bool playout(int color, Board &board, set<int> amaf[]);
bool evaluate_playout(Board &board);
float compute_variance(float winrate, float ln_part);
void update_tree(int z, Node* back_node, set<int> amaf[]);
int best_child();
int find_set(int x, Board & board)
{
	if ( x != board.uf_set[x])
	{
		board.uf_set[x] = find_set(board.uf_set[x], board);
	}
	return board.uf_set[x];
}

void link(int x, int y, Board &board)   //两个祖先,按秩合并  
{  
	if(x == y) return;  
	/*if(board.rank[x] > board.rank[y])
    {  */
    board.uf_set[y] = x;    //将深度小的合并到深度大的那棵树上  
  /*  }
    else  
    {  
       if(rank[x] == rank[y])  
       {  
           rank[y]++;  
       }  
       father[x] = y;  
    }  */
}  

Node *root;

void
init_brown()
{
  int k;
  //int i, j;

  /* The GTP specification leaves the initial board configuration as
   * well as the board configuration after a boardsize command to the
   * discretion of the engine. We choose to start with up to 20 random
   * stones on the board.
   */
  clear_board();

  for (int i = 0; i < board_size; ++i)
	  for (int j = 0; j < board_size; ++j)
	  {
		  pr[i][j] = POS(i,j);
		  ir[POS(i,j)] = i;
		  jr[POS(i,j)] = j;
		  for (k = 0; k < 8; k++) {
			int ai = i + deltai[k];
			int aj = j + deltaj[k];
			if (on_board(ai,aj))
				neighbors[POS(i,j)][k] = POS(ai,aj);
			else
				neighbors[POS(i,j)][k] = -1;
		  }
	  }
	for (int i = 0; i < 8; ++i)
		eight.push_back(i);

}

int get_board(int i, int j)
{
	return gboard.board[pr[i][j]];
}

int get_string(int i, int j, int *stonei, int *stonej)
{
	int num_stones = 0;
  	int pos = POS(i, j);
  	do {
    	stonei[num_stones] = I(pos);
    	stonej[num_stones] = J(pos);
    	num_stones++;
    	pos = gboard.next_stone[pos];
  	} while (pos != POS(i, j));

  	return num_stones;
}

int legal_move(int i, int j, int color)
{
	return Legal_move(i, j, color, gboard);
}

 bool eat_first(int *i, int *j, int color)
 {
 	int moves[174];
 	int num_moves = 0;
// 	int move;
 	int ai, aj;
// 	int k;

 	memset(moves, 0, sizeof(moves));
 	for (ai = 0; ai < board_size; ai++)
 	for (aj = 0; aj < board_size; aj++) {
 	  /* Consider moving at (ai, aj) if it is legal and not Suicide. */
		if (Legal_move(ai, aj, color, gboard)
 		  && !Suicide(ai, aj, color,gboard )) 
 	  {
 			int pos = pr[ai][aj];
 			/* Further require the move not to be Suicide for the opponent... */
 			for (int k = 0 ; k < 4; ++k)
 			{
 				int neighbor = neighbors[pos][k];
 				if (neighbor != -1 && gboard.board[neighbor] == OTHER_COLOR(color))
 				{
 					if (!has_additional_liberty(neighbor,pos,gboard))
 					{
 						*i = ai;
 						*j = aj;
 						return true;
 					}
 				}
 			}
 	}
 	}
 	return false;
 }

void generate_move(int *i, int *j, int color)
{
	if (eat_first(i,j,color)) return;
	
	if (walks < 2)
	{
		if (gboard.board[pr[10][9]] == EMPTY)
		{
			*i = 10;
			*j = 9;
			return;
		}
		else
		{
			*i = 2;
			*j = 3;
			return;
		}
	}

	start_time = clock();

	//Generate_move(i, j, color, gboard);
	mcts(i, j, color);
}

void play_move(int i, int j, int color)
{
	Play_move(i, j, color, gboard);
	++walks;
	//ofstream fb("ablack.txt",ofstream::app | ofstream::out);
	//ofstream fw("awhite.txt",ofstream::app | ofstream::out);

	////FILE * fb = fopen("black.txt","a+");
	/////FILE * fw = fopen("white.txt","a+");

	////ofstream fs("fs.txt");
	////ofstream fs("fs.txt");

	//fb<<"\n"<<walks<<"\n\n";
	//fw<<"\n"<<walks<<"\n\n";
	//for (int i = 0; i < board_size * board_size; ++i)
	//{
	//	if (!(i%13)){ fb<<endl; fw<<endl;}
	//	//int gcore = find_set(i, gboard);
	//	fb<<gboard.pos2play[BLACK-1][i]<<'\t';
	//	fw<<gboard.pos2play[WHITE-1][i]<<'\t';
	//
	//}
	//fb.close();
	//fw.close();
}

// the board being played
void
clear_board()
{
	walks = 0;
	//stage = 2;
	gboard.last = -1;
	beta_num = 1200;
	//bias = 1/5000.f;
	for (int i = 0; i < board_size*board_size; ++i)
	{
		gboard.uf_set[i]= i;
		gboard.string_lib[i].clear();

		for (int k = 0; k < 4; ++k)
		{
			int neighbor = neighbors[i][k];
			if (neighbor >= 0)
				gboard.string_lib[i].insert(neighbor);
		}
	}
	gboard.pos2play[0].reset(); // 0 capable.
	gboard.pos2play[0].flip();
	gboard.pos2play[1].reset();
	gboard.pos2play[1].flip();

    memset(gboard.board, 0, sizeof(gboard.board));
	//ofstream fb("ablack.txt");
	//ofstream fw("awhite.txt");
	//fb.close();
	//fw.close();


}

int
board_empty()
{
  int i;
  for (i = 0; i < board_size * board_size; i++)
    if (gboard.board[i] != EMPTY)
      return 0;

  return 1;
}

int
Get_board(int i, int j, Board &board)
{
  return board.board[pr[i][j]];
}


static int
pass_move(int i, int j)
{
  return i == -1 && j == -1;
}

static int
on_board(int i, int j)
{
  return i >= 0 && i < board_size && j >= 0 && j < board_size;
}

int
Legal_move(int i, int j, int color, Board &board)
{
  int other = OTHER_COLOR(color);
  
  /* Pass is always legal. */
  if (pass_move(i, j))
    return 1;

  /* Already occupied. */
  if (board.board[pr[i][j]] != EMPTY)
    return 0;

  /* Illegal ko recapture. It is not illegal to fill the ko so we must
   * check the color of at least one neighbor.
   */
  if (i == board.ko_i && j == board.ko_j
      && ((on_board(i - 1, j) && board.board[pr[i-1][j]] == other)
	  || (on_board(i + 1, j) && board.board[pr[i+1][j]] == other)))
    return 0;

  return 1;
}

static int
has_additional_liberty(int orpos, int libpos, Board &board)
{
	int pos_core = find_set(orpos, board);

	if (board.string_lib[pos_core].size() > 1)
		return 1;
  /*int pos = orpos;
  do {
    int k;
    for (k = 0; k < 4; k++) {
      int neighbor = neighbors[pos][k];
      if (neighbor >= 0 && board.board[neighbor] == EMPTY
	  && (libpos != neighbor))
      {
		return 1;
	  }
    }
    pos = board.next_stone[pos];
  } while (pos != orpos);*/


  return 0;
}


static int
provides_liberty(int apos, int pos, int color, Board &board)
{
  /* A vertex off the board does not provide a liberty. */
  if (apos < 0)
    return 0;

  /* An empty vertex IS a liberty. */
  if (board.board[apos] == EMPTY)
    return 1;

  /* A friendly string provides a liberty to (i, j) if it currently
   * has more liberties than the one at (i, j).
   */
  if (board.board[apos] == color)
    return has_additional_liberty(apos, pos, board);

  /* An unfriendly string provides a liberty if and only if it is
   * captured, i.e. if it currently only has the liberty at (i, j).
   */
  return !has_additional_liberty(apos, pos, board);
}

static int
Suicide(int i, int j, int color, Board &board)
{
  int k;
  int pos = pr[i][j];
  for (k = 0; k < 4; k++)
  {
    int neighbor = neighbors[pos][k];
    if (provides_liberty(neighbor, pos, color, board))
	{
		return 0;
	}
  }

  return 1;
}

static int
remove_string(int pos, Board &board)
{
  int removed = 0;
  int color = board.board[pos];
  int ot_color = OTHER_COLOR(color);
  int opos = pos;
  int string_core = find_set(pos ,board);
  do {
    board.board[pos] = EMPTY;
	board.uf_set[pos] = pos;
	board.string_lib[pos].clear();
	board.pos2play[0][pos] = board.pos2play[1][pos] = 1; //release
	for (int k = 0; k < 4; ++k)
	{
		int neighbor = neighbors[pos][k];
		if (neighbor < 0) continue;
		if (board.board[neighbor] != ot_color)
		{
			board.string_lib[pos].insert(neighbor);
		}
		else  // ot_color
		{
			int pos_core = find_set(neighbor, board);
			int p = -1;
			if (board.string_lib[pos_core].size() == 1) // original can be cvaptured
			{
				p = *(board.string_lib[pos_core].begin()); 
			}

			//board.string_lib[pos_core].erase(string_core);
			board.string_lib[pos_core].insert(pos);
			
			if (p != -1)
			{
				board.pos2play[OTHER_COLOR(color)-1][p] = 1; // no need to attack myself

				if (board.pos2play[color-1][p] != 0) // originally we can play at it
				{

					if (Suicide(ir[p], jr[p], color, board)) // suicide if the string has additional liberty
					{
						board.pos2play[color-1][p] = 0;
						int l;
						for (l = 0; l < 4; ++l)
						{
							int ew = neighbors[neighbor][l];
							if (ew >= 0 && board.board[ew] == color) // 可以提掉子
							{
								//board.pos2play[color-1][neighbor] = 1; // necessary????
								break;
							}
						}
						if (l == 4) board.pos2play[OTHER_COLOR(color)-1][p] = 0;  //eye
						//board.pos2play[OTHER_COLOR(color)-1][p] = 0; // no need to attack myself
					}
				}
			}

		}
	}
    removed++;
    pos = board.next_stone[pos];
  } while (pos != opos);

  if (removed == 1)
  {
	  if (Suicide(ir[opos], jr[opos], color, board))
	  {
		  board.pos2play[color-1][opos] = 0;
	  }
  }
  return removed;
}

static int
same_string(int pos1, int pos2, Board &board)
{
  if (find_set(pos1, board) == find_set(pos2, board))
	  return 1;
  
  return 0;
}

int Play_move(int i, int j, int color, Board &board)
{
  /* Reset the ko point. */
  board.ko_i = -1;
  board.ko_j = -1;

  /* Nothing more happens if the move was a pass. */
  if (pass_move(i, j))
  {  
	  board.last = -1;
	  return 0;
  }


  int pos = pr[i][j];
  int captured_stones = 0;
  int k;
  board.last = pos;

  /* If the move is a Suicide we only need to remove the adjacent
   * friendly stones.
   */
  if (Suicide(i, j, color, board)) {
	  board.board[pos] = OTHER_COLOR(color);
	  //FILE * tmp = fopen("remove.txt","a+");
	for (k = 0; k < 4; k++) {
      int neighbor = neighbors[pos][k];
      if (neighbor >= 0
	  && board.board[neighbor] == color)
	    {
		//	fprintf(tmp, "%d %d %d\n",i,j,color);
			remove_string(neighbor, board);
			board.string_lib[neighbor].insert(pos);
	  }
    }
	//fclose(tmp);
	  board.board[pos] = EMPTY;
    return 2;
  }

  board.board[pos] = color;
  //FILE * tmp = fopen("remove.txt","a+");
  /* Not Suicide. Remove captured opponent strings. */
  for (k = 0; k < 4; k++) {
    int neighbor = neighbors[pos][k];
    if (neighbor >= 0
	&& board.board[neighbor] == OTHER_COLOR(color)
	&& !has_additional_liberty(neighbor, pos, board))
	{
	//    fprintf(tmp, "%d %d %d\n",i,j,color);
		captured_stones += remove_string(neighbor , board);
	}
  }
  board.board[pos] = EMPTY;
  //fclose(tmp);
  /* Put down the new stone. Initially build a single stone string by
   * setting next_stone[pos] pointing to itself.
   */
  
  /* If we have friendly neighbor strings we need to link the strings
   * together.
   */

  board.board[pos] = color;
  board.next_stone[pos] = pos;
  board.pos2play[0][pos] = board.pos2play[1][pos] = 0;

  for (k = 0; k < 4; k++) {
	int pos2 = neighbors[pos][k];
    /* Make sure that the stones are not already linked together. This
     * may happen if the same string neighbors the new stone in more
     * than one direction.
     */
	if (pos2 == -1) continue;
	
	if (board.board[pos2] != color) // easy
	{
		int pos_core = find_set(pos2,board);
		board.string_lib[pos_core].erase(pos); // core
	}
    else
	if (board.board[pos2] == color && !same_string(pos, pos2, board))  // the same color.
	{
      /* The strings are linked together simply by swapping the the
       * next_stone pointers.
       */
      int tmp = board.next_stone[pos2];
      board.next_stone[pos2] = board.next_stone[pos];
      board.next_stone[pos] = tmp;

	  int core1 = find_set(pos, board);
	  int core2 = find_set(pos2, board);
	  board.string_lib[core2].erase(pos); // core

	  if (board.string_lib[core1].size() >= board.string_lib[core2].size() )
	 {  
		 link(core1, core2, board); // has the same neighbor.
		 board.string_lib[core1].insert(board.string_lib[core2].begin(),board.string_lib[core2].end());
		 board.string_lib[core2].clear();
	  }
	  else
	  {
		  link(core2, core1, board);
		  board.string_lib[core2].insert(board.string_lib[core1].begin(),board.string_lib[core1].end());
		  board.string_lib[core1].clear();
	  }
	  // pos2 take pos as the father

    }
  }

  int pos_core = find_set(pos, board);
  if (board.string_lib[pos_core].size() == 1)
  {
	  int p = *(board.string_lib[pos_core].begin());
	  board.pos2play[OTHER_COLOR(color)-1][p] = 1;
	  if (Suicide(ir[p], jr[p], color, board))
		  board.pos2play[color-1][p] = 0;
	  else
		  board.pos2play[color-1][p] = 1;
  }

  for (k = 0; k < 4; ++k)
  {
	  int neighbor = neighbors[pos][k];
	  if (neighbor < 0) continue;

	  if (board.board[neighbor] == OTHER_COLOR(color))
	  {
		  int ot_core = find_set(neighbor, board);
		  if (board.string_lib[ot_core].size() == 1)
		  {
			  int p = *(board.string_lib[ot_core].begin());
			  board.pos2play[color-1][p] = 1;
			  
			  if (Suicide(ir[p], jr[p], OTHER_COLOR(color), board))
				board.pos2play[OTHER_COLOR(color)-1][p] = 0; 
			  else
				board.pos2play[OTHER_COLOR(color)-1][p] = 1;
		  }
	  }
	  else if (board.board[neighbor] == EMPTY)
	  {
		  if (Suicide(ir[neighbor], jr[neighbor], color, board))
		  {
			  board.pos2play[color-1][neighbor] = 0;
			  board.pos2play[OTHER_COLOR(color)-1][neighbor] = 1;// can not be here!
		  }
		  else if (Suicide(ir[neighbor], jr[neighbor], OTHER_COLOR(color), board))
		  {
			  board.pos2play[OTHER_COLOR(color)-1][neighbor] = 0; // can not be here!
			  int l;
			  for (l = 0; l < 4; ++l)
			  {
				  int ew = neighbors[neighbor][l];
				  if (ew >= 0 && board.board[ew] == OTHER_COLOR(color))
				  {
						board.pos2play[color-1][neighbor] = 1; // necessary????
						break;
				  }
			  }
			  if (l == 4)
				  board.pos2play[color-1][neighbor] = 0; // eye
		  }
	  }
  }

  /* If we have captured exactly one stone and the new string is a
   * single stone it may have been a ko capture.
   */
  if (captured_stones == 1 && board.next_stone[pos] == pos) {
    /* Check whether the new string has exactly one liberty. If so it
     * would be an illegal ko capture to play there immediately. We
     * know that there must be a liberty immediately adjacent to the
     * new stone since we captured one stone.
     */
	  int neighbor;
	  for (k = 0; k < 4; k++) {
		  neighbor = neighbors[pos][k];
		  if (neighbor >= 0 && board.board[neighbor] == EMPTY)
			break;
		}
    
		if (!has_additional_liberty(pos, neighbor, board)) {
			board.ko_i = ir[neighbor];
			board.ko_j = jr[neighbor];
    }
  }
  return 1;
}

/* Generate a move. */
void Generate_move(int *i, int *j, int color, Board &board)
{
    int ot_color = OTHER_COLOR(color);

	if (board.last!=-1)
	{
		int neighbor;
		//int neighbor;
		int ot_color = OTHER_COLOR(color);
		/*for (int k = 0; k < 4; ++k)
		{
			neighbor = neighbors[board.last][k];
			if (neighbor >= 0)
			{
				int ai = ir[neighbor];
				int aj = jr[neighbor];
				if (Legal_move(ai,aj,color,board) && board.board[board.last] == ot_color)
				{
					if(!has_additional_liberty(board.last, neighbor, board))
					{
						*i = ai;
						*j = aj;
						return;
					}
				}
			}

		}*/

		random_shuffle(eight.begin(),eight.end());

		for (int sq=0 ;sq<8;++sq )
        {
			int k = eight[sq];
			neighbor = neighbors[board.last][k];
			
			if (neighbor >= 0)
			{
				int ai = ir[neighbor];
				int aj = jr[neighbor];
				if (Legal_move(ai,aj,color,board) && !Suicide(ai, aj, color, board))
				{
					if (  cut1_pattern(ai, aj,color, board) || cut2_pattern(ai, aj,color, board) ||
						hane_pattern(ai,aj,color, board) || board_pattern(ai,aj,color, board))
					{
						*i = ai;
						*j = aj;
						return;
					}
				}
			}
		}
	}

	//// 这个生成是全局找可以下的 上面那个是全盘随机点 看能下就下 但是不合适 所以如果能维护哪些点是可以下的就很好了~！

 // int moves[174];
 // int num_moves = 0;
 // int move;
 // int ai, aj;
 // int k;

 // memset(moves, 0, sizeof(moves));
 // for (ai = 0; ai < board_size; ai++)
 //   for (aj = 0; aj < board_size; aj++) {
 //     /* Consider moving at (ai, aj) if it is legal and not Suicide. */
 //     if (Legal_move(ai, aj, color, board)
	//	  && !Suicide(ai, aj, color, board)) 
	//  {
	//		int pos = pr[ai][aj];
	//		/* Further require the move not to be Suicide for the opponent... */
	//		if (!Suicide(ai, aj, ot_color, board))
	//			moves[num_moves++] = pos;
	//		else {
	//		/* ...however, if the move captures at least one stone,
	//			* consider it anyway.
	//		*/
	//			int pos = pr[ai][aj];
	//			for (k = 0; k < 4; k++) 
	//			{
	//				int neighbor = neighbors[pos][k];
	//				if ( neighbor != -1 && board.board[neighbor] == ot_color) 
	//				{
	//					moves[num_moves++] = pos;
	//					break;
	//				}
	//			}

	//			}
	//	}
 //   }

	int store = -1;
	int ko_one = -1;
	int cidx = color -1;
	if (board.ko_i != -1)
	{
		ko_one = pr[board.ko_i][board.ko_j];
		store = board.pos2play[cidx][ko_one];
		board.pos2play[cidx][ko_one] = 0;
	}
	int num2play = board.pos2play[cidx].count();
	
	if (num2play == 0)
	{
		if (ko_one != -1) board.pos2play[cidx][ko_one] = store;
		*i = -1;
		*j = -1;

		return;
	}


	int idx = rand()%(board.pos2play[cidx].count());
	int t = 0;
	for (int k = 0 ; k < board_size * board_size; ++k)
	{
		if (board.pos2play[cidx][k] == 1)
		{
			if (t == idx)
			{
				*i = ir[k];
				*j = jr[k];
				//board.pos2play[cidx][ko_one] = store;
				if (ko_one != -1) board.pos2play[cidx][ko_one] = store;
				//gen<<*i<<' '<<*j<<endl;
				//gen<<"end"<<endl;
				return;
			}
			++t;
		}
	}

	//gen<<"fuck"<<endl;



   /* Choose one of the considered moves randomly with uniform
   * distribution. (Strictly speaking the moves with smaller 1D
   * coordinates tend to have a very slightly higher probability to be
   * chosen, but for all practical purposes we get a uniform
   * distribution.)
   */
  //if (num_moves > 0) {
  //  move = moves[rand() % num_moves];
  //  *i = ir[move];
  //  *j = jr[move];
  //}
  //else {
  //  /* But pass if no move was considered. */
  // 
  //}
}

void simulation(long long seed, int color, int order)
{
	srand(seed);
	//gtp_printf("%d>",seed);
	int bestChildren[MAX_BOARD_SIZE];

	while (true)
	{
		gmutex.lock();
		++sim;
		gmutex.unlock();

		finish_time = clock();
		if (finish_time - start_time > 3000) { gtp_printf("%d die\n",order);/* beta_num = i/3;*/ break;}
		int to_mlt = color == BLACK ? -1 : 1; //to_multiply 如果要下的是黑子的话 取最大值 
		set<int> amaf[2];

		Node *bg = root; // begin to search
		float bestUCB1 = -100;
		int bestChild;
        Board tboard(gboard);

		//先实现UCT好了
		for (int idx = 1; ; ++idx)
	 	{
			bestUCB1 = -100;
			to_mlt = -to_mlt; // 这边把root状态装置成对的
			if (bg->numOfChildren!= 0)
	 		{
				int cdidx = 0;
				float child_ucb1;
				float ln_part = log(float(bg->n)); // Oh I need to update the root!!!
				float beta = sqrt(beta_num/(beta_num+3*bg->n));			
				int cidx = 0;
				for (int c = 0; c < bg->numOfChildren; ++c)
				{
					Node *tmp_child = bg->children[c];
					//float beta = float(tmp_child->rn) / (tmp_child->n + tmp_child->rn + tmp_child->n*tmp_child->rn*bias);
					if (tmp_child->n!=0)
					{
						if (to_mlt<0)
							child_ucb1 = (1-beta)*(1-tmp_child->q)+beta*(1-tmp_child->rq) + sqrt(ln_part/tmp_child->n*MIN(0.25,compute_variance(1-tmp_child->q,ln_part/tmp_child->n)));
						else
							child_ucb1 = (1-beta)*tmp_child->q+beta*tmp_child->rq + sqrt(ln_part/tmp_child->n*MIN(0.25,compute_variance(tmp_child->q,ln_part/tmp_child->n)));

						if (child_ucb1 > bestUCB1){ bestUCB1 = child_ucb1; cidx = 1; bestChildren[0] = c;}
						else if (child_ucb1 == bestUCB1) {bestChildren[cidx++] = c;}
					}
					else
					{
						child_ucb1 = INFINITE;
						if (child_ucb1 == bestUCB1){   bestChildren[cidx++] = c;}
						else if (child_ucb1 > bestUCB1) {bestUCB1 = child_ucb1; cidx = 1; bestChildren[0] = c;}
					}
				}
				bg = bg->children[bestChildren[rand()%cidx]];
				Play_move(ir[bg->idx],jr[bg->idx],bg->color, tboard);
				//tlast2 = tlast;
				//tboard.last = bg->idx;
	 		}
	 		else
	 			break; // Oh we have arrived at the leaf node!
		}
		int or_color = bg->color;
		int ot_color = OTHER_COLOR(or_color);
		int z;
		Node * fault_leaf = bg;
		if (fault_leaf->n >= EXPAND_VISITS && fault_leaf->numOfChildren == 0) // It's enough to expand it
		{
			gmutex.lock();
			fault_leaf->expand(tboard);
			gmutex.unlock();

			if(fault_leaf->numOfChildren>0) //expand successfully
			{
				Node *tmp_child = fault_leaf->children[rand() % fault_leaf->numOfChildren]; // randomly
				Play_move(ir[tmp_child->idx], jr[tmp_child->idx], ot_color, tboard);
				tboard.last = tmp_child->idx;
				z = playout(or_color, tboard, amaf); // the color should bt the same as node[idx-1], but color should be the one to win!!

				update_tree(z, tmp_child, amaf); // When we update we can know which childre is the maximum
			}
			else
			{
				z = playout(ot_color, tboard, amaf);
				update_tree(z, fault_leaf, amaf);
			}
		}
		else // not need to expand
		{
			z = playout(ot_color, tboard, amaf);
			update_tree(z, fault_leaf, amaf);
		}
	}

}


void mcts(int *xi, int *yj, int color)
{
	//int MAX_GAME_LEN = board_size*board_size*3;
	// root must be the status after that the opponent has played
	if (walks < 10) stage = 2;
	else stage = 0;

	//else if (walks < 20) stage = 1;
	//else stage = 0;
	root = new Node(gboard.last,NULL,OTHER_COLOR(color)); 
	root->n = 1;
	root->expand(gboard, stage);
	sim = 0;

	//多线程可能就要挪到里面去了 node

	vector<thread> threads;

	 for (int i = 0 ; i < THREAD_NUM; ++i)
    {
        threads.push_back(thread(simulation, rand(), color, i));
    } // when time is up , they would exit.

	for (auto &thread : threads) //join
    {
        thread.join();
    }
	
	//beta_num = 0.66*sim;
	ofstream fout("bn.txt");

	fout<<sim<<endl;

	/*if (sim > 8000)
		beta_num = 3200;
	else
		if (sim > 12000)
			beta_num = 4300;
		else
			if (sim > 15000)
				beta_num = 5000;
			else
				if (sim > 18000)
					beta_num = 0.35*sim;*/

	int result = best_child();


	if (result>=0)
	{
		*xi = I(result);
		*yj = J(result);
	}
	else
	{	
		*xi = -1;
		*yj = -1;
	}
	delete root;
	root = NULL;
}

//start to play color
bool playout(int color, Board &board ,set<int> amaf[])
{
	//int Final_status[MAX_BOARD_SIZE];
	int pmove = 1, other_pmove = 1;
	int i, j;
	int other_color = OTHER_COLOR(color);
	int test = 0;
	
	while ((pmove || other_pmove) && test< 300 )
	{
		++test;
		Generate_move(&i, &j, color, board);
		pmove = Play_move(i, j, color, board);
		if (pmove) {amaf[color-1].insert(pr[i][j]); board.last = pr[i][j];}
		else board.last = -1;

		Generate_move(&i, &j, other_color, board);
		other_pmove = Play_move(i, j, other_color, board);
		if (other_pmove) {amaf[color-1].insert(pr[i][j]); board.last = pr[i][j];}
		else board.last = -1;
	}
	Compute_final_status(board);
	return evaluate_playout(board);
}

bool evaluate_playout(Board &board)
{
	float score = komi;
	int i, j;

	for (i = 0; i < board_size; i++)
    	for (j = 0; j < board_size; j++) {
	    	int status = Get_final_status(i, j, board);
	    	if (status == BLACK_TERRITORY)
				score--;
	      	else if (status == WHITE_TERRITORY)
				score++;
	      	else if ((status == ALIVE) ^ (board.board[pr[i][j]] == WHITE))
				score--;
			else
				score++;
	    }

	return score<0;

}

float compute_variance(float winrate,float ln_part)
{
	float cp1 = winrate*winrate;
	float cp2 = winrate;
	return cp2 - cp1 + sqrt(2*ln_part);
}

void update_tree(int z, Node *back_node, set<int> amaf[])
{
	gmutex.lock();
	set<int>::iterator amaf_idx;
	while (back_node->parent != NULL)
	{
		back_node->n += 1; //stand update
		if (back_node->q != INFINITE)
			back_node->q = back_node->q + (z-back_node->q)/back_node->n;
		else
			back_node->q = z;

		//################################################################
		//amaf sibling

		Node *tmp_parent = back_node->parent;

		int color = back_node->color;
		amaf_idx = amaf[color-1].begin();
		int n;
		for (int c = 0; c < tmp_parent->numOfChildren; ++c)
		{
			Node* tmp_child = tmp_parent->children[c];
			int child_idx = tmp_child->idx;
			while ( *amaf_idx < child_idx && amaf_idx != amaf[color-1].end())
				++amaf_idx;
			if (amaf_idx == amaf[color-1].end()) 
				break;
			if (*amaf_idx == child_idx)
			{
				tmp_child->rn += 1;
				tmp_child->rq = tmp_child->rq + (z-tmp_child->rq)/tmp_child->rn;
				// amaf get update
			}
			//n += tmp-
		}
		amaf[color-1].insert(back_node->idx);
		back_node = tmp_parent;
	}
	root->n += 1;
	gmutex.unlock();
}

int best_child()
{
	int playtimes = 0;
	int idx = 0;
	if (!root->numOfChildren) return -1;
	//gtp_printf("num root %d \n",root->numOfChildren);
	int *node = new int[board_size*board_size];
	for (int i = 0 ; i < root->numOfChildren; ++i)
	{
		Node * tmp = root->children[i];
		//gtp_printf("<%d times>\n",tmp->n);
		if (playtimes > tmp->n)
			continue;
		if (playtimes < tmp->n)
		{
			idx = 0;
			playtimes = tmp->n;
			node[idx] = i;
		}		
		else
			node[++idx] = i;
	}

	int result =  root->children[node[rand() % (idx+1)]]->idx;
	delete node;
	node = NULL;
	return result;
}

/* Set a final status value for an entire string. */
void
set_final_status_string(int pos, int status)
{
	Set_final_status_string(pos, status, gboard);
}

void
Set_final_status_string(int pos, int status, Board& board)
{
  int pos2 = pos;
  do {
    board.final_status[pos2] = status;
    pos2 = board.next_stone[pos2];
  } while (pos2 != pos);
}

/* Compute final status. This function is only valid to call in a
 * position where generate_move() would return pass for at least one
 * color.
 *
 * Due to the nature of the move generation algorithm, the final
 * status of stones can be determined by a very simple algorithm:
 *
 * 1. Stones with two or more liberties are alive with territory.
 * 2. Stones in atari are dead.
 *
 * Moreover alive stones are unconditionally alive even if the
 * opponent is allowed an arbitrary number of consecutive moves.
 * Similarly dead stones cannot be brought alive even by an arbitrary
 * number of consecutive moves.
 *
 * Seki is not an option. The move generation algorithm would never
 * leave a seki on the board.
 *
 * Comment: This algorithm doesn't work properly if the game ends with
 *          an unfilled ko. If three passes are required for game end,
 *          that will not happen.
 */

void compute_final_status(void)
{
	Compute_final_status(gboard);
}

void
Compute_final_status(Board &board)
{
  int i, j;
  int pos;
  int k;

  for (pos = 0; pos < board_size * board_size; pos++)
    board.final_status[pos] = UNKNOWN;
  
  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++)
	{
		int ppos = pr[i][j];
		if (board.board[ppos] == EMPTY)
			for (k = 0; k < 4; k++) {
				int neighbor = neighbors[ppos][k];
				if (neighbor < 0)
					continue;
	  /* When the game is finished, we know for sure that (ai, aj)
           * contains a stone. The move generation algorithm would
           * never leave two adjacent empty vertices. Check the number
           * of liberties to decide its status, unless it's known
           * already.
	   *
	   * If we should be called in a non-final position, just make
		* sure we don't call set_final_status_string() on an empty
	   * vertex.
	   */
				if (board.final_status[neighbor] == UNKNOWN) {
					if (board.board[neighbor] != EMPTY) {
						if (has_additional_liberty(neighbor, ppos, board))
							Set_final_status_string(neighbor, ALIVE, board);
						else
							Set_final_status_string(neighbor, DEAD, board);
					}
				}
	  /* Set the final status of the (i, j) vertex to either black
           * or white territory.
	   */
				if (board.final_status[pr[i][j]] == UNKNOWN) {
					if ((board.final_status[pos] == ALIVE) ^ (board.board[neighbor] == WHITE))
						board.final_status[pr[i][j]] = BLACK_TERRITORY;
					else
						board.final_status[pr[i][j]] = WHITE_TERRITORY;
				}
		}
	}
}

int
get_final_status(int i, int j)
{
  return gboard.final_status[pr[i][j]];
}

int
Get_final_status(int i, int j, Board &board)
{
	return board.final_status[pr[i][j]];
}

void
set_final_status(int i, int j, int status)
{
  gboard.final_status[pr[i][j]] = status;
}

/* Valid number of stones for fixed placement handicaps. These are
 * compatible with the GTP fixed handicap placement rules.
 */
int
valid_fixed_handicap(int handicap)
{
  if (handicap < 2 || handicap > 9)
    return 0;
  if (board_size % 2 == 0 && handicap > 4)
    return 0;
  if (board_size == 7 && handicap > 4)
    return 0;
  if (board_size < 7 && handicap > 0)
    return 0;
  
  return 1;
}

/* Put fixed placement handicap stones on the board. The placement is
 * compatible with the GTP fixed handicap placement rules.
 */
void
place_fixed_handicap(int handicap)
{
  int low = board_size >= 13 ? 3 : 2;
  int mid = board_size / 2;
  int high = board_size - 1 - low;
  
  if (handicap >= 2) {
    play_move(high, low, BLACK);   /* bottom left corner */
    play_move(low, high, BLACK);   /* top right corner */
  }
  
  if (handicap >= 3)
    play_move(low, low, BLACK);    /* top left corner */
  
  if (handicap >= 4)
    play_move(high, high, BLACK);  /* bottom right corner */
  
  if (handicap >= 5 && handicap % 2 == 1)
    play_move(mid, mid, BLACK);    /* tengen */
  
  if (handicap >= 6) {
    play_move(mid, low, BLACK);    /* left edge */
    play_move(mid, high, BLACK);   /* right edge */
  }
  
  if (handicap >= 8) {
    play_move(low, mid, BLACK);    /* top edge */
    play_move(high, mid, BLACK);   /* bottom edge */
  }
}

/* Put free placement handicap stones on the board. We do this simply
 * by generating successive black moves.
 */
void
place_free_handicap(int handicap)
{
  int k;
  int i, j;
  
  for (k = 0; k < handicap; k++) {
    generate_move(&i, &j, BLACK);
    play_move(i, j, BLACK);
  }
}


/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
