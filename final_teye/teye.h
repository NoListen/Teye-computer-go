/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is Brown, a simple go program.                           *
 *                                                               *
 * Copyright 2003 and 2004 by Gunnar Farnebäck.                  *
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

#include "gtp.h"
#include "Board.h"
#include <utility>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <vector>
//#include <set>


using namespace std;
#define VERSION_STRING "3.0 ap 5000"
#define THREAD_NUM 4
#define MIN_BOARD 2
#define MAX_BOARD 19
#define MAX_BOARD_SIZE 361
#define EXPAND_VISITS 1
#define INFINITE 100000
#define NUM_SIMULATION 8000


/* These must agree with the corresponding defines in gtp.c. */
#define EMPTY 0
#define WHITE 1
#define BLACK 2

/* Used in the final_status[] array. */
#define DEAD 0
#define ALIVE 1
#define SEKI 2
#define WHITE_TERRITORY 3
#define BLACK_TERRITORY 4
#define UNKNOWN 5

extern float komi;
extern int board_size;

void init_brown(void);
void clear_board(void);
int board_empty(void);
int on_board(int i, int j);
int get_board(int i, int j);
int get_string(int i, int j, int *stonei, int *stonej);
int legal_move(int i, int j, int color);

int Get_board(int i, int j, Board &board);

//int Get_string(int i, int j, int *stonei, int *stonej, int Next_stone[]);
int Legal_move(int i, int j, int color, Board &board);

void play_move(int i, int j, int color);
void generate_move(int *i, int *j, int color);

void compute_final_status(void);
void Compute_final_status(Board &board);

int get_final_status(int i, int j);
//int Get_final_status(int i, int j, int Final_status[]);
int Get_final_status(int i, int j, Board &board);

void set_final_status(int i, int j, int status);
//void Set_final_status_string(int pos, int status, int Final_status[], int* Next_stone);
void Set_final_status_string(int pos, int status, Board& board);

int valid_fixed_handicap(int handicap);
void place_fixed_handicap(int handicap);
void place_free_handicap(int handicap);
//int Play_move(int i, int j, int color, int &Ko_i, int &Ko_j, int* Board, int* Next_stone);
//int Play_move(int i, int j, int color, int &Ko_i, int &Ko_j, int Board[], int Next_stone[], pair<int,set<int>> CFG[], set<int> &Empty_pos);
int Play_move(int i, int j, int color, Board &board);
//void Generate_move(int *i, int *j, int color, int &Ko_i, int &Ko_j, int Board[], pair<int,set<int>> CFG[], set<int> &Empty_pos);
void Generate_move(int *i, int *j, int color, Board &board);

/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
