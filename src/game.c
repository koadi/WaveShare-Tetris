/* 
 * File:   game.c
 * Author: Phil
 *
 * Created on September 19, 2012, 12:23 PM
 */
#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "sound.h"
#include "graphics.h"
#include "glcd.h"
#include "tetris.h"
#include "highscores.h"
#include "game.h"

unsigned char GamePaused = 0;           // Is the game currently paused?
unsigned int RandomSeed = 0;            // Holds the random seed for the RNG
unsigned char GameOver = 1;             // Is the game over? Initially it is.
unsigned char NewLines;                 // Holder for the amount of lines achieved in a given play
unsigned long Score = 0;                // Players score
unsigned int Lines = 0;                 // Total lines achieved
unsigned char Level = 0;                // Players current level
unsigned int LevelAdvanceLines = 0;     // How many lines left before a level up
unsigned char GravitySetting = 0;       // The normal drop rate of the tetrominoes in GameCycles
unsigned char Gravity = 0;              // The amount of GameCycles left before the current tetrominoe drops
unsigned char GameCycle = 0;            // A counter that runs from 1 - 60 used to synchronize events
unsigned char SoftDrop = 0;             // Is the player performing a soft drop?
unsigned char DelayedAutoShift = 0;     // Is delayed auto shift enabled (left/right sliding)
unsigned char handle_DAS = 0;           // Does delayed auto shift need to be handled in the RunGame loop?
unsigned char handle_GravityDrop;       // Does a gravity drop need to be handled in the RunGame loop?
unsigned char btnCCWhandled = 0;        // Debounces the rotate button
unsigned char btnLEFThandled = 0;       // Debounces the left button
unsigned char btnRIGHThandled = 0;      // Debounces the right button
unsigned char btnDOWNhandled = 0;       // Debounces the down button
unsigned char btnMIDDLEhandled = 0;     // Debounces the middle (joystick press in) button


/*
 * Interrupt handler
 */
void interrupt high_isr(void)
{
    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF)      // Timer0 has overflowed (Game Clock)
    {
        
        GameCycle++;                                // 1/60th of a second has elapsed, increment the GameCycle counter
        Gravity--;                                  // Decrement the gavity counter
        if( Gravity == 0 ) {                        // Gravity has reached 0, the piece needs to shift down
            if( SoftDrop )
            {                                       // If the player was performing a soft drop,
                Gravity = GRAVITY_SOFTDROP_SPEED;   // the new gravity setting will be the soft drop speed
            } else
            {
                Gravity = GravitySetting;           // otherwise, the new gravity setting shall be the default drop rate for the current level of play
            }
            handle_GravityDrop = 1;                 // Let the RunGame loop know the piece needs to shift down
        }
        if( GameCycle == 60 ) GameCycle = 0;        // Reset the GameCycle back to zero

        if(GameOver) RandomSeed++;                  // If we're not currently playing a game, increment the random seed value, this is
                                                    // what seeds the RNG for the initial bag of tetrominoes, basically the seed is the number
                                                    // of GameCycles passed before the player presses ROTATE to start the game.

        // DAS - Delayed Auto Shift system
        // works exactly like it does in the NES version of Tetris.
        // 16 GameCycles before the inital repeat, 6 GameCycles for each additional repeat
        if( BTN_LEFT == PRESSED || BTN_RIGHT == PRESSED )
        {
            DelayedAutoShift --;            // Decrememnt the DAS counter
            if( DelayedAutoShift == 0 ) {   // If we've reached 0, we need to shift the piece left or right depending
                handle_DAS = 1;             // Let the RunGame loop know the piece needs to be shifted left/right
                DelayedAutoShift = 6;       // Reset the DAS counter to 6 GameCycles
            }
        }


        GameClock_Reset();                  // Reset (preload) Timer0 to expire in 1/60th of a second for the next GameCycle
        
        INTCONbits.TMR0IF = 0;              // Clear the Timer0 overflow interrupt flag
    }
    if(PIE1bits.TMR1IE && PIR1bits.TMR1IF)      // Timer1 has overflowed (Music Beat Period)
    {
        TMR1 = BeatSpeed;                       // Reset Timer1 to our beat speed (see sound.c)
        if(TMR1BeatCounter && TMR1BeatCounter >= 16) {
            TMR1BeatCounter-=16;                // Decrememnt the beat counter
        } else
        {
            NotePlaying = 0;                    // Beat counter has expired, we're no longer playing a note
            PlayNextNote = 1;                   // Tell the RunAttractCycle to play the next note
        }
        if(!MusicOn)                            // If the music shouldn't be on,
        {
            T1CONbits.TMR1ON = 0;               // Disable the music related timers so furthur interrupts don't
            T3CONbits.TMR3ON = 0;               // disturb our game
            PIR2bits.TMR3IF = 0;                // reset the interrupt flags too
        }
        PIR1bits.TMR1IF = 0;                    // Clear the interrupt flag for Timer1
    }
    if(PIE2bits.TMR3IE && PIR2bits.TMR3IF)      // Timer3 has overflowed, this is our Music frequency counter
    {
        // note clock expired
        if(NotePlaying)                         // If we're playing a note,
        {
            SPEAKER = !SPEAKER;                 // toggle the speaker to generate the sound
        } else
        {
            SPEAKER = 1;                        // otherwise, make sure the PNP driver transistor is off
        }
        TMR3 = Note;                            // Reset Timer3 to the note frequency for the current playing note
        PIR2bits.TMR3IF = 0;                    // Clear the interrupt flag for Timer3
    }
}

/*
 * Switches the game into the Game Over state
 * the playfield is filled with colored blocks and the message [GAME OVER] is displayed
 * then wait for the user to acknowledge they failed by pressing the rotate button
 */
void DoGameOver(void)
{
    signed char x,y,z;
    z = 1;                          // Color cycle counter
    for( y = 19; y > -1; y-- )      // For every row in the Playfield from the top to the bottom
    {
        for( x = 0; x < 10; x++ )   // and for every block in that row, from left to right
        {
          if( z == 8 ) z = 1;       // reset the color cycle counter back to 1 since we only have 7 different block colors
          DrawBlock((unsigned char)x,(unsigned char)y,(unsigned char)z);    // Draw a colored block at the current Playfield[x][y]
          z++;                      // Increment the color counter
        }
    }
    glcd_FilledRectangle(0,153, 240, 14, COLOR_BLACK);                      // Draw a black bar across the screen in the middle
    glcd_Print("[GAME OVER]", 2, 0, 153, 1, COLOR_WHITE, COLOR_BLACK);      // Write [GAME OVER] into the middle of that bar
    while(!BTN_ROTATE_CCW == PRESSED) NOP();                                // Wait for the player to acknowledge
    while(!BTN_ROTATE_CCW == NOT_PRESSED) NOP();
    GameOver = 1;                                                           // Set the GameOver state
}


/*
 * Draws a preview of the upcoming tetrominoe on the sidebar
 */
void DrawPreview(const t_tetrominoe *Piece)
{
    unsigned char BlockPointer;
    unsigned int x,y,i;

    glcd_FilledRectangle(163, 105, 72, 40, COLOR_BLACK);                        // Clear the next piece preview area on the sidebar

    GLCD_Enable();                                                              // Set the GLCD CS line

    for( BlockPointer = 0; BlockPointer < 4; BlockPointer++ )                   // For all four blocks of this tetrominoe
    {
        x = (unsigned int)163 + Piece->PreviewBlocks[BlockPointer].X;           // Caclulate the screen X location for the preview
        y = (unsigned int)109 + Piece->PreviewBlocks[BlockPointer].Y;           // Calculate the screen Y location for the preview

        GLCD_Post_Command(0x08, x);                                             // X starting location for Window Access Mode
        GLCD_Post_Command(0x0A, y);                                             // Y starting location for Window Access Mode
        GLCD_Post_Command(0x09, x+15);                                          // X ending location for Window Access Mode
        GLCD_Post_Command(0x0B, y+15);                                          // Y ending location for Window Access Mode
        GLCD_Post_Command(0x0005, 0x0010);                                      // Put GLCD in Window Access Mode
        GLCD_Index();                                                           // Set the D/I line to instruction mode (command mode, whatever)
        GLCD_Post_Data(0x0E);                                                   // GLCD RAM Write instruction
        GLCD_Data();                                                            // Set the D/I line to data mode
        for( i = 0; i < 256; i++ )                                              // For all 256 pixels in this block
        {                                                                       // Write the color Word from the Pallette indexed in the block image (see graphics.h)
            GLCD_Post_Data(Tetrominoes[(Piece->GridMarker-1)].BlockPalette[(Tetrominoes[(Piece->GridMarker-1)].BlockImage[i])]);
        }
    }
    GLCD_Disable();                                                             // Clear the GLCD CS line
}



/*
 * Configures Timer0 for use as our Game Clock
 */
void GameClock_Init(void)
{
    // sets up the game clock timer
    T0CONbits.TMR0ON = 0;   // stop timer
    T0CONbits.T08BIT = 0;   // configure for 16-bit
    T0CONbits.T0CS = 0;     // choose FCY as clock source
    T0CONbits.T0PS = 0x07;  // assign a 1:256 prescale
    T0CONbits.PSA = 0;      // enable prescaling
}


/*
 * Enables the interrupts we'll be needing for the game
 */
void Interrupts_Init(void)
{
    RCONbits.IPEN = 0;      // Use compatibility mode (all interrupts are high priority)
    INTCONbits.TMR0IE = 1;  // Enable Timer0 overflow interrupt (our Game Clock)
    PIE1bits.TMR1IE = 1;    // Enable Timer1 overflow interrupt (musical beat timer)
    PIE2bits.TMR3IE = 1;    // Enable Timer3 overflow interrupt (musical frequency timer)
    INTCONbits.PEIE = 1;    // Enable peripheral interrupts
    INTCONbits.GIE = 1;     // Enable global interrupts (all of them)
}


/*
 * Pauses the current game.
 * Clear the Playfield, and the next piece preview area, stops the game clock
 * and writes the message [PAUSED] across the Playfield
 */
void PauseGame(void)
{
    signed char x, y;
    unsigned int gy;
    unsigned char cleared = 0;
    GamePaused = 1;                                                         // Sets the GamePaused state
    GameClock_Stop();                                                       // Stops the Game Clock from counting
    glcd_FilledRectangle(163, 105, 72, 40, COLOR_BLACK);                    // Blacks out the next piece preview
    EraseCurrentObject();                                                   // Erases the current piece in play from the screen
    for( y = 19; y > -1; y-- )                                              // For every row in the Playfield
    {
        for( x = 0; x < 10; x++ )                                           // and every block in that row
        {
            if( Playfield[x][y] != 0 )                                      // if it's not empty, the we've found the highest block on the screen
            {
                gy = (unsigned int)(((20 - (y + 1)) + 1) * 16 - 16);        // calculate the screen coordinates of that highest block
                glcd_FilledRectangle(0, gy, 160, 320 - gy, COLOR_BLACK);    // and paint over the screen from that block downwards
                cleared = 1;                                                // mark as cleared so we can jump out of the loop
                break;
            }
        }
        if(cleared) break;
    }
    glcd_Print("[PAUSED]", 2, 33, 153, 0, COLOR_WHITE, COLOR_BLACK);        // Write the word [PAUSED] across the Playfield
}


/*
 * Resumes a previously paused game
 */
void ResumeGame(void)
{
    unsigned char x,y;
    glcd_FilledRectangle(0, 153, 160, 14, COLOR_BLACK);                     // Clear the [PAUSED] text from the Playfield
    for( x = 0; x < 10; x++ )                                               // For every column
    {
        for( y = 0; y < 20; y++ )                                           // and every row
        {
            if( Playfield[x][y] != 0 ) DrawBlock(x, y, Playfield[x][y]);    // draw the blocks that happen to be there
        }
    }
    DrawPreview(&Tetrominoes[(NextPiece - 1)]);                             // Draw the next piece preview
    DrawCurrentObject();                                                    // Draw the current piece in play
    GamePaused = 0;                                                         // Clear the GamePaused state
    GameClock_Start();                                                      // Restart the Game Clock
}


#if 0
/*
 * Not complete, due to a lack of remaining program flash space
 * it would have drawn a decorative pattern on the sidebar area of the screen where
 * your score and such are
 */
void DecorateSide(void)
{
    glcd_Line(160, 0, 160, 63, 0xFFFF);
    glcd_Line(160, 0, 223, 0, 0xFFFF);
    glcd_Line(191, 1, 191, 15, 0x0000);
    glcd_Line(176, 15, 191, 15, 0x0000);
    glcd_Line(175, 15, 175, 47, 0x0000);
    glcd_Line(160, 47, 175, 47, 0x0000);
    glcd_Line(160, 48, 176, 48, 0xFFFF);
    glcd_Line(176, 47, 176, 16, 0xFFFF);
    glcd_Line(176, 16, 191, 16, 0xFFFF);
    glcd_Line(191, 17, 191, 63, 0x0000);
    glcd_Line(161, 63, 190, 63, 0x0000);
}
#endif


/*
 * Resets the game state
 */
void StartNewGame(void)
{
    GameOver = 0;                                                               // Set some initial values for the new game
    GamePaused = 0;
    Score = 0;
    Lines = 0;
    Level = 0;
    NotePointer = 0;
    LevelAdvanceLines = 20;
    GravitySetting = 48;
    SoftDrop = 0;
    Gravity = GravitySetting;
    GameCycle = 0;
    handle_GravityDrop = 0;
    handle_DAS = 0;
    TextBG = COLOR_SIDEBAR_BG;
    TextFG = COLOR_BLACK;
    srand(RandomSeed);                                                          // Reseed the RNG
    ClearPlayfield();                                                           // Clear the memory in the Playfield array
    glcd_FilledRectangle(0, 0, 160, 320, COLOR_BLACK);                          // Paint the Playfield black
    glcd_FilledRectangle(160, 0, 80, 320, COLOR_SIDEBAR_BG);                    // Paint the sidebar gray
    //DecorateSide();
    glcd_Print("Score:", 2, 163, 3, 0, COLOR_BLACK, COLOR_SIDEBAR_BG);          // Write the score heading on the sidebar
    TextX = 163;
    TextY = 20;
    printf("%6d", Score);                                                       // Write the current score on the sidebar (0)
    glcd_Print("Lines:", 2, 163, 37, 0, COLOR_BLACK, COLOR_SIDEBAR_BG);         // Write the lines heading on the sidebar
    TextX = 163;
    TextY = 54;
    printf("%6d", Lines);                                                       // Write the current number of completed lines on the sidebar (0)
    glcd_Print("Level:", 2, 163, 71, 0, COLOR_BLACK, COLOR_SIDEBAR_BG);         // Write the level heading on the sidebar
    TextX = 163;
    TextY = 88;
    printf("%6d", Level);                                                       // Write the current level on the sidebar (0)
    GenerateNewBag();                                                           // Generate a new random bag of tetrominoes
    ReachIntoBag();                                                             // Pull out the next tetrominoe from the bag
    DrawPreview(&Tetrominoes[NextPiece-1]);                                     // Draw the preview of the next tetrominoe
}


/*
 * Advances the play level
 */
void LevelUp(void)
{
    Level++;                                                    // Increment the current level
    glcd_FilledRectangle(163, 88, 77, 14, COLOR_SIDEBAR_BG);    // Clear the sidebar area where the current level is displayed
    TextX = 163;                                                // Set coordinates to be used by printf's putch() routine
    TextY = 88;
    printf("%6d", Level);                                       // Write the new level in the sidebar
    LevelAdvanceLines = Level * 10 + 10;                        // Calculate the number of lines needed to advance to the next level
    if(LevelAdvanceLines > 100) LevelAdvanceLines = 100;        // or 100, whichever comes first
    switch(Level)                                               // Set the new piece drop speed for this level
    {                                                           // this is identical to the NES version of Tetris
        case 1:
            GravitySetting = 43;
            break;
        case 2:
            GravitySetting = 38;
            break;
        case 3:
            GravitySetting = 33;
            break;
        case 4:
            GravitySetting = 28;
            break;
        case 5:
            GravitySetting = 23;
            break;
        case 6:
            GravitySetting = 18;
            break;
        case 7:
            GravitySetting = 13;
            break;
        case 8:
            GravitySetting = 8;
            break;
        case 9:
            GravitySetting = 6;
            break;
        case 10:
        case 11:
        case 12:
            GravitySetting = 5;
            break;
        case 13:
        case 14:
        case 15:
            GravitySetting = 4;
            break;
        case 16:
        case 17:
        case 18:
            GravitySetting = 3;
            break;
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
            GravitySetting = 2;
            break;
        default:
            GravitySetting = 1;
            break;
    }
}


/*
 * Main Game Loop, this is the heart of the game
 */
void RunGame(void)
{
    while(!GameOver)                                                            // While the game is not over
    {
        if( handle_GravityDrop == 1 )                                           // Should we shift a piece down?
        {
            // drop the object one line
            if(!CheckCollision(DOWN))                                           // If the piece doesn't collide with anything beneath it
            {
                EraseCurrentObject();                                           // Erase the piece from the screen
                MoveObject(DOWN);                                               // Move it down in the Playfield
                DrawCurrentObject();                                            // Redraw it on the screen
            } else
            {                                                                   // Otherwise, the piece has collided with the bottom of the screen, or another piece
                if(!LockObject())                                               // If we can't lock the piece into the Playfield,
                {
                    // Game Over
                    DoGameOver();                                               // ...the game is over.
                    if(IsHighScore(Score)) DoNameEntry(Score);                  // If we earned a high score, we switch to name entry mode
                    DisplayHighScores();                                        // We display the high scores when the game is over
                    break;
                }                                                               // If the piece could be locked into the Playfield
                ReachIntoBag();                                                 // Our game continues, and we reach into our tetrominoe bag for the next piece
                NewLines = ProcessLines();                                      // Now we process any lines that have been cleared
                if( NewLines != 0 )                                             // If there were some lines we cleared
                {
                    Lines += NewLines;                                          // add them to our Lines count
                    if( LevelAdvanceLines <= NewLines ) {                       // Check to see if we've leveled up
                        LevelUp();                                              // if we have, handle that situation
                    } else
                    {                                                           // Otherwise, we decement these new lines from the counter
                        LevelAdvanceLines -= NewLines;                          // that keeps track of how many lines to the next level.
                    }
                    if( NewLines == 1 ) Score += 10 * (Level + 1);              // Our scoring system is pretty simple.
                    if( NewLines == 2 ) Score += 30 * (Level + 1);              // 10 points per 1st line, 20 for the second, 30 for the third, and 40 for the fourth
                    if( NewLines == 3 ) Score += 60 * (Level + 1);              // multiplied by the level we're on.
                    if( NewLines == 4 ) Score += 100* (Level + 1);
                    glcd_FilledRectangle(163, 20, 77, 14, COLOR_SIDEBAR_BG);    // Clear the sidebar where our Score is indicated
                    TextX = 163;
                    TextY = 20;
                    printf("%6u", Score);                                       // Write our new score to the sidebar
                    glcd_FilledRectangle(163, 54, 77, 14, COLOR_SIDEBAR_BG);    // Clear the sidebar where the cleared Lines are shown
                    TextX = 163;
                    TextY = 54;
                    printf("%6u", Lines);                                       // Write the new number of cleared lines to the sidebar
                }
                DrawPreview(&Tetrominoes[NextPiece-1]);                         // Draw the preview of the next piece on the sidebar
            }
            handle_GravityDrop = 0;                                             // Mark the Gravity Drop as complete
        }

        if( BTN_ROTATE_CCW == PRESSED && !btnCCWhandled)                        // Check to see if the rotate button was pressed
        {
            btnCCWhandled = 1;                                                  // Debounce the button
            RotateCCW();                                                        // Rotate the piece
        }
        if( BTN_ROTATE_CCW == NOT_PRESSED && btnCCWhandled ) btnCCWhandled = 0;

        if( BTN_LEFT == PRESSED && !btnLEFThandled )                            // Check to see if the left button was pressed
        {
            btnLEFThandled = 1;                                                 // Debounce the button
            DelayedAutoShift = 16;                                              // Key the initial DAS delay
            if( !CheckCollision(LEFT) )                                         // Check to see if we can move the piece left
            {
                EraseCurrentObject();                                           // if we can, clear the piece on the screen
                MoveObject(LEFT);                                               // move the piece left in the Playfield
                DrawCurrentObject();                                            // Redraw the piece
            }
        }
        if( BTN_LEFT == NOT_PRESSED && btnLEFThandled ) btnLEFThandled = 0;

        if( BTN_RIGHT == PRESSED && !btnRIGHThandled )                          // Check to see if the right button was pressed
        {
            btnRIGHThandled = 1;                                                // Debounce the button
            DelayedAutoShift = 16;                                              // Key the inital DAS delay
            if( !CheckCollision(RIGHT) )                                        // Check to see if we can move the piece right
            {
                EraseCurrentObject();                                           // if we can, clear the piece on the screen
                MoveObject(RIGHT);                                              // move the piece right in the Playfield
                DrawCurrentObject();                                            // Redraw the piece
            }
        }
        if( BTN_RIGHT == NOT_PRESSED && btnRIGHThandled ) btnRIGHThandled = 0;

        if( BTN_DOWN == PRESSED )                                               // Check to see if the down button was pressed
        {
            SoftDrop = 1;                                                       // Enable soft drop mode
            Gravity = GRAVITY_SOFTDROP_SPEED;                                   // Override the current Gravity counter
        } else if( SoftDrop )                                                   // If the button is no longer pressed, but soft drop mode is on
        {
            SoftDrop = 0;                                                       // Turn soft drop mode off
            Gravity = GravitySetting;                                           // Restore the Gravity counter to the default for this level
        }

        // DAS - Delayed Auto Shift
        if( handle_DAS == 1 )                                                   // Check to see if DAS is scheduled
        {
            handle_DAS = 0;                                                     // clear it if so
            if( BTN_LEFT == PRESSED )                                           // were we to move Left?
            {
                if( !CheckCollision(LEFT) )                                     // can we move left?
                {
                    EraseCurrentObject();                                       // erase the piece
                    MoveObject(LEFT);                                           // update the piece location
                    DrawCurrentObject();                                        // redraw the piece
                }
            }
            if( BTN_RIGHT == PRESSED )                                          // were we to move Right?
            {
                if( !CheckCollision(RIGHT) )                                    // can we move right?
                {
                    EraseCurrentObject();                                       // erase the piece
                    MoveObject(RIGHT);                                          // update the piece location
                    DrawCurrentObject();                                        // redraw the piece
                }
            }
        }
        // Pause
        if( BTN_MIDDLE == PRESSED && !GamePaused && !btnMIDDLEhandled )         // If the middle button was pressed, and the game is not paused
        {
            PauseGame();                                                        // Pause the game
            btnMIDDLEhandled = 1;                                               // Debounce the button
        }
        if( BTN_MIDDLE == PRESSED && GamePaused && !btnMIDDLEhandled )          // If the middle button was pressed, and the game is paused
        {
            ResumeGame();                                                       // Unpause the game
            btnMIDDLEhandled = 1;                                               // Debounce the button
        }
        if( BTN_MIDDLE == NOT_PRESSED && btnMIDDLEhandled ) btnMIDDLEhandled = 0;
    }
}


/*
 * Attract cycle 1, it flashes the text "Press [ROTATE] to play" at the bottom of the title screen
 */
void Attract1(void)
{
    glcd_Print("Press [ROTATE]", 2, 0, 285, 1, COLOR_YELLOW, COLOR_SOVIET_RED);
    glcd_Print("to play", 2, 0, 302, 1, COLOR_YELLOW, COLOR_SOVIET_RED);
}
/*
 * Attract cycle 2, it flashes the text "Press [ROTATE] to play" at the bottom of the title screen
 */
void Attract2(void)
{
    glcd_FilledRectangle(0, 285, 240, 34, COLOR_SOVIET_RED);
}


/*
 * Draws the old school Russian TETPN logo on the screen.
 * This is a nod to the original game I played as a child, it was published by
 * Spectrum HoloByte.
 *
 * Thanks Alexey Pajitnov, for many, many hours of fun!
 */
void TitleScreen(void)
{
    glcd_FilledRectangle(0, 0, 240, 320, COLOR_SOVIET_RED);                     // Paint the screen red
    glcd_DrawBitmap2X(&logo_image_data, &logo_color_palette, 0, 64, 120, 35);   // Draw the old TETPN logo on the screen double its encoded size
    glcd_Print("Phil Ciebiera", 2, 0, 200, 1, COLOR_YELLOW, COLOR_SOVIET_RED);  // Write my name
    glcd_Print("(2012)", 2, 0, 217, 1, COLOR_YELLOW, COLOR_SOVIET_RED);         // and the year I made this
}


/*
 * Runs the attract cycle of the game, plays music and then flashes a message at the bottom of the screen.
 */
void RunAttractCycle(void)
{
    unsigned char count = 0;
    unsigned char direction = 0;
    TitleScreen();                                                              // Draw the intial Title screen
    Attract1();                                                                 // Write the "Press [ROTATE] to play" message
    MusicOn = 1;                                                                // Enable the music
    LoadNextNote();                                                             // Load the next note into the buffer
    MusicClock_Init();                                                          // Start the Music Clock
    PlayFrequency(NextNote,NextOctave,NextDuration);                            // Play the first note
    while(BTN_ROTATE_CCW != PRESSED)                                            // Wait for the user to start the game
    {
        if(PlayNextNote && MusicOn)                                             // If we're ready to play the next note, and the music isn't finished
        {
            PlayFrequency(NextNote,NextOctave,NextDuration);                    // Play the next note
            PlayNextNote = 0;                                                   // Mark that we've played the next note
        }
        if(!MusicOn)                                                            // If the music is finished, flash the "Press [ROTATE] to play" message
        {
            if(direction == 0) {
                if( GameCycle == 0 ) count++;
                if(count == 3) {
                    count = 0;
                    direction = 1;
                    Attract1();
                }
            } else
            {
                if( GameCycle == 0 ) count++;
                if(count == 3) {
                    count = 0;
                    direction = 0;
                    Attract2();
                }
            }
        }
    }
}


/*
 * main
 */
void main(void) {
    ADCON1 = 0x0F;      // Puts the 18F4520 in all digital mode
    TRISA = 0x01;       // Makes PORTA0 an input, while the rest are outputs
    TRISD = 0xFE;       // Sets PORTD as switch inputs, except for D0 which is an output used by the GLCD
    
    TRISC0 = 0;         // Outputs for the GLCD
    TRISC1 = 0;
    TRISC2 = 0;

    PORTB = 0x00;       // Clears PORTB
    LATB = 0x00;
    TRISB = 0x00;       // Sets PORTB as outputs


    // Tweak to get the LCD to properly intialize, just a bit of a delay on power up
    GLCD_Disable();
    for(Score = 250000; Score != 0; Score-- ) NOP();
    glcd_Init();
    
    Interrupts_Init();      // Start up the interrupts
    GameClock_Init();       // Initialize the Timer that runs the Game Clock
    GameClock_Start();      // Starts the Game Clock
    GameClock_Reset();      // Resets the Game Clock

    LoadHighScores();       // Loads the high scores from EEPROM into RAM

    while(1)
    {
        RunAttractCycle();  // Start the Title Screen / Attract Mode
        MusicOn = 0;        // Disables music
        StartNewGame();     // Starts a new game
        RunGame();          // Runs the game
    }
}