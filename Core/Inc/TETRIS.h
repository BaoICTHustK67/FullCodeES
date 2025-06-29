#pragma once

#include "main.h"
#include "stdbool.h"

//============================================================
// "extern" keyword is used to avoid multiple definitions
// it makes variables visible to other source files that want
// to use them but variables are only defined in one place

extern int tetris[30][30]; // Khai báo mảng

extern int  TETRIS_Score, HighScore,
            tetrominoLocX, tetrominoLocY, // Tetromino Location X & Y
            DelayTime, count,
            ASCIIValue,
            dem,
            Level,
            FallingSpeed;

extern unsigned short int	CurrentState,
                            NextPiece,
                            colour;

// Quy ước các trạng thái của tetrominoes
// Vì "switch... case" không hỗ trợ dữ liệu chuỗi
// nên đành phải quy ước thành số nguyên

    // 'I1' = 1		'L4' = 10
    // 'I2' = 2		'S1' = 11
    // 'J1' = 3		'S2' = 12
    // 'J2' = 4		'T1' = 13
    // 'J3' = 5		'T2' = 14
    // 'J4' = 6		'T3' = 15
    // 'L1' = 7		'T4' = 16
    // 'L2' = 8		'Z1' = 17
    // 'L3' = 9		'Z2' = 18

extern char key; // Stores keys that user pressed

extern bool	NewTetromino,
            CanMoveRight,
            CanMoveLeft,
            CanMoveDown,
            CanRotate,
            Pause,
            fullrow,
            GamePlay, // Checks whether the programme is in gameplay state or not
            disable; //để tạm dừng màn hình bên trái

// Functions define
void ArrayReset();
void PrintArray(int x, int y);
void ArrayDebug();
void DeleteRow(int n);
void MoveAllRowAbove(int n);
int CheckFullRow();
void PrintVariables();
void Refresh();
void InputProcess();
void TETRIS_main();
void khung();
void TETRIS_PrintScore();
