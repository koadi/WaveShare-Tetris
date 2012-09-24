/* 
 * File:   tetris.h
 * Author: Phil
 *
 * Created on September 19, 2012, 10:30 AM
 */

#ifndef TETRIS_H
#define	TETRIS_H

#include <stdlib.h>
#include <string.h>
#include "glcd.h"
#include "graphics.h"

#define LEFT 0x00
#define RIGHT 0x01
#define DOWN 0x02

typedef struct
{
    unsigned char X;
    unsigned char Y;
} t_gridpoint;

typedef struct
{
    const unsigned int  *BlockPalette;
    const unsigned char *BlockImage;
    unsigned char       GridMarker;
    t_gridpoint         SpawnBlocks[4];
    t_gridpoint         CurrentBlocks[4];
    t_gridpoint         RotationalCenter;
    t_gridpoint         PreviewBlocks[4];
} t_tetrominoe;



extern unsigned char Playfield[10][22];
extern unsigned char TetrominoeBag[7];
extern unsigned char BagPointer;
extern unsigned char NextPiece;
extern t_tetrominoe CurrentPiece;
extern const t_tetrominoe Tetrominoes[7];


void ClearPlayfield(void);
unsigned char LockObject(void);
void GenerateNewBag(void);
void ReachIntoBag(void);
unsigned char CheckCollision(unsigned char Direction);
void MoveObject(unsigned char Direction);
void RotateCCW(void);
void EraseCurrentObject(void);
void DrawCurrentObject(void);
void DrawBlock(unsigned char x, unsigned char y, unsigned char ObjectType);
void DrawBlocks(t_gridpoint BlockLocations[], unsigned char ObjectType);
unsigned char ProcessLines(void);


#endif	/* TETRIS_H */