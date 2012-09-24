#include <xc.h>
#include "game.h"
#include "highscores.h"

char HiScoreNames[10][12];
unsigned long HiScores[10];


/*
 * Writes the high score data contained in RAM to the data EEPROM for safekeeping
 */
void SaveHighScores(void)
{
    unsigned char i,e;
    INTCONbits.GIE = 0;         // Disable interrupts
    PIR2bits.EEIF = 0;          // Ensure the EEPROM flag bit is cleared
    EECON1bits.EEPGD = 0;       // Access data EEPROM memory
    EECON1bits.CFGS = 0;        // Access Flash program or data EEPROM memory
    EECON1bits.WREN = 1;        // Allows write cycles to Flash program/data EEPROM
    for( i = 0; i < 10; i++ )
    {                           // Loop over all 10 items in the high scores list
        for( e = 0; e < 16; e++ )
        {                       // For each of the 16 bytes that comprise a high score entry..
            EEADR = i * 16 + e; // Set the address of this byte to write in data EEPROM
            if( e < 4 )         // If we're working on the unsigned long score, 4 bytes
            {
                EEDATA = (unsigned char)(HiScores[i] >> 24 - (e * 8));  // Set the appropriate byte of the unsigned long to the EEPROM data register
            } else
            {
                EEDATA = HiScoreNames[i][e-4];  // Otherwise, we're writing part of the name entry, and load that byte into the EEPROM data register
            }
            EECON2 = 0x55;                  // Special write sequence,
            EECON2 = 0xAA;                  // as per the Microchip datasheet
            EECON1bits.WR = 1;              // Initiate the write
            while(!PIR2bits.EEIF) NOP();    // Wait until the write is complete
            PIR2bits.EEIF = 0;              // Clear the EEPROM flag bit for the next cycle
        }
    }
    EECON1bits.WREN = 0;        // Inhibit write cycles to Flash program/data EEPROM
    INTCONbits.GIE = 1;         // Enable interrupts again
}


void LoadHighScores(void)
{
    unsigned char i,e;
    unsigned char unset = 1;
    for( i = 0; i < 10; i++ )
    {
        HiScores[i] = 0;
        memset(HiScoreNames[i], 0x00, 12);
        for( e = 0; e < 16; e++ )
        {
            EEADR = i * 16 + e;
            EECON1bits.EEPGD = 0;
            EECON1bits.RD = 1;
            if( e < 4 )
            {
                HiScores[i] |= (unsigned long)(EEDATA << 24 - (e * 8));
            } else
            {
                HiScoreNames[i][e-4] = EEDATA;
            }
        }
        // check for an unflashed eeprom, and initialize
        if( HiScores[i] == 0xFFFFFFFF )
        {
            HiScores[i] = 0x00000000;
            memset(HiScoreNames[i], 0x00, 12);
        } else
        {
            unset = 0;
        }
    }
    if(unset)
    {
        // save the initialized high score list
        SaveHighScores();
    }
}


void DisplayHighScores(void)
{
    unsigned char i;
    glcd_FilledRectangle(0, 0, 240, 320, COLOR_BLACK);
    glcd_Print("HIGH SCORES", 2, 0, 1, 1, COLOR_WHITE, COLOR_BLACK);
    TextFG = COLOR_GOLD;
    TextBG = COLOR_BLACK;
    TextY = 1;
    for( i = 0; i < 10; i++ )
    {
        if(HiScores[i] == 0) break;
        TextX = 3;
        TextY = (i + 1) * 17 + 1;
        printf("%-12s %7lu", HiScoreNames[i], HiScores[i]);
    }
    TextY+=34;
    glcd_Print("Press [ROTATE]", 2, 0, TextY, 1, COLOR_WHITE, COLOR_BLACK);
    while(!BTN_ROTATE_CCW == PRESSED) NOP();
    while(!BTN_ROTATE_CCW == NOT_PRESSED) NOP();
}


/*
 * Searches through the high scores to see if the new score ranked
 * Returns: 1, High score achieved, 0, not a high score
 */
unsigned char IsHighScore(unsigned long NewScore)
{
    signed char i;
    for( i = 9; i > -1; i-- ) if( NewScore > HiScores[i] ) return 1;
    return 0;
}

/*
 * Supplied a new high score and a name, this inserts the new entry at the
 * appropriate spot in the high score table
 */
void InsertHighScore(unsigned long NewScore, char *NewName)
{
    signed char i, NewIndex;
    unsigned char ScoreRanked = 0;
    for( NewIndex = 9; NewIndex > -1; NewIndex-- )          // Loop through all entries in reverse
    {
        if( HiScores[NewIndex] > NewScore ) break;          // Once we've found a score we haven't beaten, we know where to insert this score
        ScoreRanked = 1;                                    // Mark that we've actually achieved a high score
    }
    if(!ScoreRanked) return;                                // If we haven't acheived a high score, leave
    for( i = 8; i >= (NewIndex+1); i-- )                    // Loop through the entries again in reverse, only this time, we only loop until we hit
    {                                                       // the location where the new score will be,
        HiScores[i+1] = HiScores[i];                        // Shift all the other entries down by one
        memcpy(HiScoreNames[i+1], HiScoreNames[i], 12);
    }
    HiScores[NewIndex+1] = NewScore;                        // Load the new score and name into the appropriate spot in the table
    memcpy(HiScoreNames[NewIndex+1], NewName, 12);
}



void NameEntry_Underline2(unsigned char LetterIndex)
{
    unsigned char Margin = 3;
    glcd_FilledRectangle(Margin + LetterIndex * 12, 120, 10, 3, COLOR_BLACK);

}
void NameEntry_Underline1(unsigned char LetterIndex)
{
    unsigned char Margin = 3;
    glcd_FilledRectangle(Margin + LetterIndex * 12, 120, 10, 3, COLOR_GOLD);
}

void NameEntry_Letter(unsigned char LetterIndex, char Letter)
{
    unsigned char Margin = 3;
    glcd_PrintChar(Letter, 2, Margin + LetterIndex * 12, 105, COLOR_GOLD, COLOR_BLACK);
}

void DoNameEntry(unsigned long TopScore)
{
    unsigned char NameEntryComplete = 0;
    unsigned char EntryLetter = 0;
    char Letter = 0;
    char ThisEntry[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char CursorBlink = 0;
    glcd_FilledRectangle(0, 0, 240, 320, COLOR_BLACK);
    glcd_Print("Congratulations", 2, 0, 3, 1, COLOR_WHITE, COLOR_BLACK);
    TextX = 1;
    TextY = 20;
    TextFG = COLOR_WHITE;
    TextBG = COLOR_BLACK;
    printf("Your score of %-6lu", TopScore);
    glcd_Print("has made it to the", 2, 0, 37, 1, COLOR_WHITE, COLOR_BLACK);
    glcd_Print("High Score List", 2, 0, 54, 1, COLOR_WHITE, COLOR_BLACK);
    glcd_Print("Your Name", 2, 3, 88, 0, COLOR_WHITE, COLOR_BLACK);

    while(!NameEntryComplete)
    {
        // Make the cursor blink
        if(GameCycle % 20 == 0) {
            if(!CursorBlink)
            {
                NameEntry_Underline1(EntryLetter);
                CursorBlink = 1;
            } else
            {
                NameEntry_Underline2(EntryLetter);
                CursorBlink = 0;
            }
        }

        if(BTN_DOWN == PRESSED && !btnDOWNhandled)
        {

        }
        if(BTN_RIGHT == PRESSED && !btnRIGHThandled)
        {
            if(EntryLetter < 11)
            {
                NameEntry_Underline2(EntryLetter);
                ThisEntry[EntryLetter] = Letter;
                EntryLetter++;
                Letter = ThisEntry[EntryLetter];
            }
            btnRIGHThandled = 1;
        }
        if(BTN_RIGHT == NOT_PRESSED && btnRIGHThandled) btnRIGHThandled = 0;
        if(BTN_LEFT == PRESSED && !btnLEFThandled)
        {
            if(EntryLetter != 0)
            {
                NameEntry_Underline2(EntryLetter);
                ThisEntry[EntryLetter] = Letter;
                EntryLetter--;
                Letter = ThisEntry[EntryLetter];
            }
            btnLEFThandled = 1;
        }
        if(BTN_LEFT == NOT_PRESSED && btnLEFThandled) btnLEFThandled = 0;
        if(BTN_DOWN == PRESSED && !btnDOWNhandled)
        {
            Letter++;
            if(Letter < 48) Letter = 48;
            if(Letter < 65 && Letter > 57) Letter = 65;
            if(Letter > 90) Letter = 0;
            NameEntry_Letter(EntryLetter, Letter);
            ThisEntry[EntryLetter] = Letter;
            btnDOWNhandled = 1;
        }
        if(BTN_DOWN == NOT_PRESSED && btnDOWNhandled) btnDOWNhandled = 0;

        if(BTN_ROTATE_CCW == PRESSED && !btnCCWhandled)
        {
            btnCCWhandled = 1;
            // entry complete
            InsertHighScore(TopScore, &ThisEntry);
            SaveHighScores();
            NameEntryComplete = 1;
        }
    }
}
