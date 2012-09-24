#include "tetris.h"

unsigned char Playfield[10][22];        // Defines the playfield including all settled tetrominoes
unsigned char TetrominoeBag[7];         // This is the random bag, it contains the next seven pieces of play
unsigned char BagPointer = 0;           // This points to where in our TetrominoeBag we are
unsigned char NextPiece = 0;            // This is the next piece in the bag 1 - 7
t_tetrominoe CurrentPiece;              // This is the current piece in play

/*
 * 0 = I piece, cyan
 * 1 = O piece, yellow
 * 2 = T piece, purple
 * 3 = S piece, green
 * 4 = Z piece, red
 * 5 = J piece, blue
 * 6 = L piece, orange
 */
const t_tetrominoe Tetrominoes[7] = {
    {
        &cyan_block_color_palette,
        &cyan_block_image_data,
        0x01,
        {
            {3, 20},
            {4, 20},
            {5, 20},
            {6, 20}
        },
        {
            {3, 20},
            {4, 20},
            {5, 20},
            {6, 20}
        },
        {4, 21},
        {
            {4, 8},
            {20, 8},
            {36, 8},
            {52, 8}
        }
    },
    {
        &yellow_block_color_palette,
        &yellow_block_image_data,
        0x02,
        {
            {4, 21},
            {4, 20},
            {5, 21},
            {5, 20}
        },
        {
            {4, 21},
            {4, 20},
            {5, 21},
            {5, 20}
        },
        {0, 0},  // Doesn't rotate
        {
            {20, 0},
            {36, 0},
            {20, 16},
            {36, 16}
        }
    },
    {
        &purple_block_color_palette,
        &purple_block_image_data,
        0x03,
        {
            {3, 20},
            {4, 20},
            {5, 20},
            {4, 21}
        },
        {
            {3, 20},
            {4, 20},
            {5, 20},
            {4, 21}
        },
        {4, 21},
        {
            {28, 0},
            {12, 16},
            {28, 16},
            {44, 16}
        }
    },
    {
        &green_block_color_palette,
        &green_block_image_data,
        0x04,
        {
            {3, 21},
            {4, 21},
            {4, 20},
            {5, 20}
        },
        {
            {3, 21},
            {4, 21},
            {4, 20},
            {5, 20}
        },
        {4, 21},
        {
            {28, 16},
            {44, 16},
            {12, 0},
            {28, 0}
        }
    },
    {
        &red_block_color_palette,
        &red_block_image_data,
        0x05,
        {
            {3, 20},
            {4, 20},
            {4, 21},
            {5, 21}
        },
        {
            {3, 20},
            {4, 20},
            {4, 21},
            {5, 21}
        },
        {4, 21},
        {
            {28, 0},
            {44, 0},
            {12, 16},
            {28, 16}
        }
    },
    {
        &blue_block_color_palette,
        &blue_block_image_data,
        0x06,
        {
            {3, 20},
            {4, 20},
            {5, 20},
            {3, 21}
        },
        {
            {3, 20},
            {4, 20},
            {5, 20},
            {3, 21}
        },
        {4, 21},
        {
            {12, 0},
            {12, 16},
            {28, 16},
            {44, 16}
        }
    },
    {
        &orange_block_color_palette,
        &orange_block_image_data,
        0x07,
        {
            {3, 20},
            {4, 20},
            {5, 20},
            {5, 21}
        },
        {
            {3, 20},
            {4, 20},
            {5, 20},
            {5, 21}
        },
        {4, 21},
        {
            {44, 16},
            {12, 16},
            {28, 16},
            {44, 0}
        }
    }
};

/*
 * Clears the playfield
 */
void ClearPlayfield(void)
{
    unsigned char x,y;
    for( x = 0; x < 10; x++ )
    {
        for( y = 0; y < 22; y++ ) Playfield[x][y] = 0x00;
    }
}


/*
 * Transfers the current active object into the settled pieces playfield
 * Returns: 1 if play continues, 0 if play is over
 */
unsigned char LockObject(void)
{
    unsigned char BlockPointer;
    for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
    {
        if(Playfield[CurrentPiece.CurrentBlocks[BlockPointer].X][CurrentPiece.CurrentBlocks[BlockPointer].Y] != 0) return 0;
        Playfield[CurrentPiece.CurrentBlocks[BlockPointer].X][CurrentPiece.CurrentBlocks[BlockPointer].Y] = CurrentPiece.GridMarker;
    }
    return 1;
}
/*
 * Generates a completely new TetrominoeBag of the seven pieces
 */
void GenerateNewBag(void)
{
    unsigned char Rnd;
    unsigned char SlotsFilled = 0;
    unsigned char BadCanidate = 0;
    unsigned char i;
    TetrominoeBag[0] = 0;
    while( SlotsFilled != 7)
    {
        Rnd = (unsigned char)rand() & 0x07;
        if(Rnd != 0x00)
        {
            // Canidate piece, check if it was already chosen
            for( i = 0; i < SlotsFilled; i++ )
            {
                if(TetrominoeBag[i] == Rnd)
                {
                    BadCanidate = 1;
                    break;
                }
            }
            if(!BadCanidate)
            {
                // Add this piece to our bag
                TetrominoeBag[SlotsFilled] = Rnd;
                SlotsFilled++;
            }
            BadCanidate = 0;
        }
    }
}

/*
 * Gets a new piece from the bag
 */
void ReachIntoBag(void)
{
    if(BagPointer == 6)
    {
        memcpy(&CurrentPiece, &Tetrominoes[NextPiece-1], sizeof(t_tetrominoe));
        BagPointer = 0;
    } else
    {
        memcpy(&CurrentPiece, &Tetrominoes[TetrominoeBag[BagPointer]-1], sizeof(t_tetrominoe));
        BagPointer++;
    }
    NextPiece = TetrominoeBag[BagPointer];
    if(BagPointer == 6) GenerateNewBag();
}


/*
 * Checks for a collision if the object were to move in the specified direction
 * Returns: 0 for no collision, 1 for collision
 */
unsigned char CheckCollision(unsigned char Direction)
{
    unsigned char BlockPointer = 0;
    // Now check for adjacent settled objects in the playfield
    switch( Direction )
    {
        case LEFT:
            for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
            {
                if( CurrentPiece.CurrentBlocks[BlockPointer].X == 0 ) return 1; // Up against left wall
                if( Playfield[(CurrentPiece.CurrentBlocks[BlockPointer].X - 1)][CurrentPiece.CurrentBlocks[BlockPointer].Y] != 0 ) return 1;    // Collides with a settled block
            }
            break;
        case RIGHT:
            for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
            {
                if( CurrentPiece.CurrentBlocks[BlockPointer].X == 9 ) return 1; // Up against right wall
                if( Playfield[(CurrentPiece.CurrentBlocks[BlockPointer].X + 1)][CurrentPiece.CurrentBlocks[BlockPointer].Y] != 0 ) return 1;    // Collides with a settled block
            }
            break;
        case DOWN:
            for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
            {
                if( CurrentPiece.CurrentBlocks[BlockPointer].Y == 0 ) return 1; // Up against bottom wall
                if( Playfield[CurrentPiece.CurrentBlocks[BlockPointer].X][(CurrentPiece.CurrentBlocks[BlockPointer].Y - 1)] != 0 ) return 1;    // Collides with a settled block
            }
            break;
    }
    return 0;
}

/*
 * Makes the current piece move by one grid cell in the specified direction
 */
void MoveObject(unsigned char Direction)
{
    unsigned char BlockPointer = 0;
    // Now move the blocks
    switch( Direction )
    {
        case LEFT:
            for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
            {
                CurrentPiece.CurrentBlocks[BlockPointer].X--;
            }
            CurrentPiece.RotationalCenter.X--;
            break;
        case RIGHT:
            for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
            {
                CurrentPiece.CurrentBlocks[BlockPointer].X++;
            }
            CurrentPiece.RotationalCenter.X++;
            break;
        case DOWN:
            for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
            {
                CurrentPiece.CurrentBlocks[BlockPointer].Y--;
            }
            CurrentPiece.RotationalCenter.Y--;
            break;
    }
}

/*
 * Attempts to rotate a piece counter clockwise
 */
void RotateCCW(void)
{

    t_gridpoint Blocks[4];
    t_gridpoint Center;
    unsigned char BlockPointer;
    signed char TotalKicks = 0;
    signed char Kick = 0;
    unsigned char Valid = 0;
    signed char x[4],y[4];
    unsigned int gx,gy;
    // First make a copy of the current piece locations, and rotational center
    memcpy(&Blocks, &CurrentPiece.CurrentBlocks, sizeof(Blocks));
    memcpy(&Center, &CurrentPiece.RotationalCenter, sizeof(t_gridpoint));

    // Rotate the blocks
    /* CX CY = Rotational origin
     * X1 Y1 = Previous block location
     * X2 Y2 = New block location
     * X2 = (Y1 + CX - CY)
     * Y2 = (CX + CY - X1)
     */

    for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
    {
        // rotate
        x[BlockPointer] = (signed char)(Blocks[BlockPointer].Y + Center.X - Center.Y);
        y[BlockPointer] = (signed char)(Center.X + Center.Y - Blocks[BlockPointer].X);
    }
    // Check for wall kicks
    while(!Valid)
    {
        for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
        {
            // Floor kicks are not allowed
            if( y[BlockPointer] < 0 ) return;
            if( x[BlockPointer] + TotalKicks < 0 ) Kick = 1;
            if( x[BlockPointer] + TotalKicks > 9 ) Kick = -1;
        }
        if(!Kick) Valid = 1;
        TotalKicks += Kick;
        if( TotalKicks > 2 || TotalKicks < -2 ) break;
    }
    // if we are !Valid here, we can't complete the move
    if(!Valid) return;
    // implement the kicks
    for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ ) x[BlockPointer] += TotalKicks;
    // now we check for settled block collisions
    for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
    {
        // sanity check
        if( x[BlockPointer] < 0 || x[BlockPointer] > 9 || y[BlockPointer] < 0 || y[BlockPointer] > 19 ) return;
        if(Playfield[x[BlockPointer]][y[BlockPointer]] != 0x00) return;
    }

    // our rotation is valid, update the center
    Center.X += TotalKicks;
    memcpy(&CurrentPiece.RotationalCenter, &Center, sizeof(t_gridpoint));
    for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
    {
        CurrentPiece.CurrentBlocks[BlockPointer].X = (unsigned char)x[BlockPointer];
        CurrentPiece.CurrentBlocks[BlockPointer].Y = (unsigned char)y[BlockPointer];
        // erase the old blocks
        gx = (unsigned int)((Blocks[BlockPointer].X+1) * 16 - 16);
        gy = (unsigned int)(((20 - (Blocks[BlockPointer].Y + 1)) + 1) * 16 - 16);
        glcd_FilledRectangle(gx, gy, 16, 16, COLOR_BLACK);
    }
    DrawBlocks(CurrentPiece.CurrentBlocks, CurrentPiece.GridMarker);
    
}

/*
 * Erases the current object on the screen
 */
void EraseCurrentObject(void)
{
    unsigned char BlockPointer;
    unsigned int x,y;
    for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
    {
        x = (unsigned int)((CurrentPiece.CurrentBlocks[BlockPointer].X+1) * 16 - 16);
        y = (unsigned int)(((20 - (CurrentPiece.CurrentBlocks[BlockPointer].Y + 1)) + 1) * 16 - 16);
        glcd_FilledRectangle(x, y, 16, 16, COLOR_BLACK);
    }
}

/*
 * Draws the current piece on the playing field
 */
void DrawCurrentObject(void)
{
    DrawBlocks(CurrentPiece.CurrentBlocks, CurrentPiece.GridMarker);
}

void DrawBlock(unsigned char x, unsigned char y, unsigned char ObjectType)
{
    unsigned char i;
    unsigned int gx,gy;
    GLCD_Enable();

    if( y > 19 ) return;   // Don't draw blocks off the screen
    gx = (unsigned int)((x+1) * 16 - 16);
    gy = (unsigned int)(((20 - (y + 1)) + 1) * 16 - 16);

    GLCD_Post_Command(0x08, gx);   // X start
    GLCD_Post_Command(0x0A, gy);  // Y start
    GLCD_Post_Command(0x09, gx+15);   // X end
    GLCD_Post_Command(0x0B, gy+15);  // Y end
    GLCD_Post_Command(0x0005, 0x0010);
    GLCD_Index();
    GLCD_Post_Data(0x0E);   // RAM Write
    GLCD_Data();
    for( i = 0; i != 255; i++ )
    {
        GLCD_Post_Data(Tetrominoes[(ObjectType-1)].BlockPalette[(Tetrominoes[(ObjectType-1)].BlockImage[i])]);
    }
    GLCD_Post_Data(Tetrominoes[(ObjectType-1)].BlockPalette[(Tetrominoes[(ObjectType-1)].BlockImage[255])]);
    GLCD_Disable();
}

/*
 * Draws a tetrominoe on the screen
 */
void DrawBlocks(t_gridpoint BlockLocations[], unsigned char ObjectType)
{
    unsigned char BlockPointer, i;
    unsigned int x,y;
    GLCD_Enable();

    for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )
    {
        if( BlockLocations[BlockPointer].Y > 19 ) continue;   // Don't draw blocks off the screen
        x = (unsigned int)((BlockLocations[BlockPointer].X+1) * 16 - 16);
        y = (unsigned int)(((20 - (BlockLocations[BlockPointer].Y + 1)) + 1) * 16 - 16);

        GLCD_Post_Command(0x08, x);   // X start
        GLCD_Post_Command(0x0A, y);  // Y start
        GLCD_Post_Command(0x09, x+15);   // X end
        GLCD_Post_Command(0x0B, y+15);  // Y end
        GLCD_Post_Command(0x0005, 0x0010);
        GLCD_Index();
        GLCD_Post_Data(0x0E);   // RAM Write
        GLCD_Data();
        for( i = 0; i != 255; i++ )
        {
            GLCD_Post_Data(Tetrominoes[(ObjectType-1)].BlockPalette[(Tetrominoes[(ObjectType-1)].BlockImage[i])]);
        }
        GLCD_Post_Data(Tetrominoes[(ObjectType-1)].BlockPalette[(Tetrominoes[(ObjectType-1)].BlockImage[255])]);
    }
    GLCD_Disable();
}

/*
 * Looks for, and clears complete lines.
 * Returns: The number of lines cleared
 */
unsigned char ProcessLines(void)
{
    unsigned char Lines = 0;
    unsigned char LineIndex[4];
    unsigned char BlockCount;
    unsigned char x,y,z;
    for( y = 0; y < 19; y++ )
    {
        BlockCount = 0;
        for( x = 0; x < 10; x++ )
        {
            if( Playfield[x][y] == 0x00 ) break;
            BlockCount++;
        }
        if( BlockCount == 10 )
        {
            LineIndex[Lines] = y;
            Lines++;
        }
    }
    if( Lines == 0 ) return 0;
    for( x = 0; x < Lines; x++ )
    {
        for( y = 0; y < 10; y++ )
        {
            Playfield[y][LineIndex[x]] = 0x00;
        }
    }
    // shift playfield
    for( x = 0; x < Lines; x++ )
    {
        for( y = LineIndex[x]; y < 19; y++ )
        {
            for(z = 0; z < 10; z++ )
            {
                Playfield[z][y] = Playfield[z][y+1];
            }
        }
        // after shifting the playfield, line 2 needs shifting 1, line 3 needs shifting 2, etc
        if(x < 3) LineIndex[x+1]-= (x+1);
    }
    // redraw playfield
    // check for populated blocks, only clear them
    glcd_FilledRectangle(0, 0, 160, 320, COLOR_BLACK);
    for( x = 0; x < 10; x++ )
    {
        for( y = 0; y < 20; y++ )
        {
            if( Playfield[x][y] != 0 ) DrawBlock(x, y, Playfield[x][y]);
        }
    }
    return Lines;
}