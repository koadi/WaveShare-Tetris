#include "sound.h"

unsigned char MusicOn = 0;
unsigned char NotePlaying = 0;
unsigned int TMR1BeatCounter = 0;
unsigned int NotePreload = 0;
unsigned int NotePointer = 0;
unsigned int NextNote = 0;
unsigned int NextDuration = 0;
unsigned char NextOctave = 0;
unsigned char PlayNextNote = 0;
unsigned int BeatSpeed = 50;
unsigned int Note;

void MusicClock_Init(void)
{
    // beat timer
    T1CONbits.RD16 = 1;     // 16-bit read/write to the timer
    T1CONbits.T1CKPS = 3;   // 1:8 prescaler
    T1CONbits.TMR1CS = 0;   // Internal clock FOSC/4
    T1CONbits.TMR1ON = 1;   // Starts the timer
    TMR1 = 0;

    // Note Frequency Timer
    T3CONbits.RD16 = 1;     // 16-bit read/write to the timer
    T3CONbits.T3CKPS = 0;   // No prescale 1:1
    T3CONbits.TMR3CS = 0;   // Internal clock FOSC/4
    T3CONbits.TMR3ON = 1;   // Starts the timer
    TMR3 = 0;
}

void LoadNextNote(void)
{
    if((Korobeiniki[NotePointer] == '3') && (Korobeiniki[NotePointer+1] == '2'))
    {
        NextDuration = 32;
        NotePointer += 2;
    }
    else if((Korobeiniki[NotePointer] == '1') && (Korobeiniki[NotePointer+1] == '6'))
    {
        NextDuration = 16;
        NotePointer += 2;
    }
    else if(Korobeiniki[NotePointer] == '8')
    {
        NextDuration = 8;
        NotePointer++;
    }
    else if(Korobeiniki[NotePointer] == '4')
    {
        NextDuration = 4;
        NotePointer++;
    }
    else if(Korobeiniki[NotePointer] == '2')
    {
        NextDuration = 2;
        NotePointer++;
    }
    else if(Korobeiniki[NotePointer] == '1')
    {
        NextDuration = 1;
        NotePointer++;
    } else
    {
        NextDuration = 4;
    }

    if(Korobeiniki[NotePointer + 1] == '#')
    {

        /* Process Sharps */

        switch(Korobeiniki[NotePointer])
        {
            case 'a' : NextNote = 4292;
                       break;
            case 'c' : NextNote = 3610;
                       break;
            case 'd' : NextNote = 3215;
                       break;
            case 'f' : NextNote = 2703;
                       break;
            case 'g' : NextNote = 2410;
                       break;
        }
        NotePointer +=2;

    } else
    {

        switch(Korobeiniki[NotePointer])
        {
            case 'a' : NextNote = 4545;
                       break;
            case 'b' : NextNote = 4048;
                       break;
            case 'c' : NextNote = 3824;
                       break;
            case 'd' : NextNote = 3407;
                       break;
            case 'e' : NextNote = 3035;
                       break;
            case 'f' : NextNote = 2865;
                       break;
            case 'g' : NextNote = 2551;
                       break;
            case 'p' : NextNote = 0;
                       break;
        }
        NotePointer++;
    }

    if(Korobeiniki[NotePointer] == '.')
    {
        /* Duration 1.5x */
        NextDuration += 128;
        NotePointer++;
    }

    if(Korobeiniki[NotePointer] == '4')
    {
        NextOctave = 4;
        NotePointer++;
    } else if(Korobeiniki[NotePointer] == '5')
    {
        NextOctave = 5;
        NotePointer++;
    } else if(Korobeiniki[NotePointer] == '6')
    {
        NextOctave = 6;
        NotePointer++;
    } else if(Korobeiniki[NotePointer] == '7')
    {
        NextOctave = 7;
        NotePointer++;
    }

    if(Korobeiniki[NotePointer] == '.')
    {
        /* Duration 1.5x */
        NextDuration += 128;
        NotePointer++;
    }

    if(Korobeiniki[NotePointer] != ',')
    {
        MusicOn = 0;
    }
    NotePointer++;
}

void PlayFrequency(unsigned int NewNote, unsigned char Octave, unsigned unsigned int Duration)
{
    switch(Octave)
    {
        case 4:
            Note = NewNote;
            break;
        case 5:
            Note = NewNote >> 1;
            break;
        case 6:
            Note = NewNote >> 2;
            break;
        case 7:
            Note = NewNote >> 4;
            break;
    }
    NotePlaying = 0;

    // process new note frequency
    if(Note)
    {
        Note = ~Note;
        TMR3 = Note;
    }
    // process note duration
    TMR1BeatCounter = 255/(Duration & 0x7F);


    // if duration is 1.5x add .5 to duration
    if(Duration & 0x80) TMR1BeatCounter = (TMR1BeatCounter + (TMR1BeatCounter >> 1));
    if(Note) {
        NotePlaying = 1;
    }
    LoadNextNote();
}

