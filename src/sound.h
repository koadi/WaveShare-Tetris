/* 
 * File:   sound.h
 * Author: Phil
 *
 * Created on September 24, 2012, 10:27 AM
 */

#ifndef SOUND_H
#define	SOUND_H

#include <xc.h>

#define MusicClock_Stop()   {T1CONbits.TMR1ON = 0; T3CONbits.TMR3ON = 0; SPEAKER = 1;}
#define MusicClock_Start()  {T1CONbits.TMR1ON = 1; T3CONbits.TMR3ON = 1;}


const unsigned char static  Korobeiniki[] = {"16e6,16p,16b,16c6,16d6,32e6,32d6,16c6,16b,16a,16p,16a,16c6,16e6,16p,16d6,16c6,16b,8p,16c6,16d6,16p,16e6,16p,16c6,16p,16a,16p,16a,p,16d6,16p,16f6,16a6,16p,16g6,16f6,16e6,8p,16c6,16e6,32f6,32e6,16d6,16c6,16b,8p,16c6,16d6,16g#,16e6,16g#,16c6,16p,16a,16p,a,16e6,16p,16b,16c6,16d6,32e6,32d6,16c6,16b,16a,16p,16a,16c6,16e6,16p,16d6,16c6,16b,8p,16c6,16d6,16p,16e6,16p,16c6,16p,16a,16p,16a,p,16d6,16p,16f6,16a6,16p,16g6,16f6,16e6,8p,16c6,16e6,32f6,32e6,16d6,16c6,16b,8p,16c6,16d6,16g#,16e6,16g#,16c6,16p,16a,16p,a,e,c,d,b4,c,a4,g#4,b4,e,c,d,b4,8c,8e,a,2g#,16e6,16p,16b,16c6,16d6,32e6,32d6,16c6,16b,16a,16p,16a,16c6,16e6,16p,16d6,16c6,16b,8p,16c6,16d6,16p,16e6,16p,16c6,16p,16a,16p,16a,p,16d6,16p,16f6,16a6,16p,16g6,16f6,16e6,8p,16c6,16e6,32f6,32e6,16d6,16c6,16b,8p,16c6,16d6,16g#,16e6,16g#,16c6,16p,16a,16p,a,16p"};

extern unsigned char MusicOn;
extern unsigned char NotePlaying;
extern unsigned int TMR1BeatCounter;
extern unsigned int NotePreload;
extern unsigned int NotePointer;
extern unsigned int NextNote;
extern unsigned int NextDuration;
extern unsigned char NextOctave;
extern unsigned char PlayNextNote;
extern unsigned int BeatSpeed;
extern unsigned int Note;

extern void MusicClock_Init(void);
extern void LoadNextNote(void);
extern void PlayFrequency(unsigned int NewNote, unsigned char Octave, unsigned unsigned int Duration);

#endif	/* SOUND_H */

