/* 
 * File:   game.h
 * Author: Phil
 *
 * Created on September 19, 2012, 12:42 PM
 */

#ifndef GAME_H
#define	GAME_H

#include "system.h"


#define GRAVITY_SOFTDROP_SPEED 1


// Game clock cycles at ~60Hz
#define GameClock_Stop()    {T0CONbits.TMR0ON = 0;}
#define GameClock_Start()   {T0CONbits.TMR0ON = 1;}
#define GameClock_Reset()   {TMR0H = 0xFE; TMR0L = 0xFC;}

extern unsigned char GamePaused;
extern unsigned int RandomSeed;
extern unsigned char GameOver;
extern unsigned char NewLines;
extern unsigned long Score;
extern unsigned int Lines;
extern unsigned char Level;
extern unsigned int LevelAdvanceLines;
extern unsigned char GravitySetting;
extern unsigned char Gravity;
extern unsigned char GameCycle;
extern unsigned char GravityDrop;
extern unsigned char DelayedAutoShift;
extern unsigned char handle_DAS;
extern unsigned char handle_GravityDrop;
extern unsigned char btnCCWhandled;
extern unsigned char btnLEFThandled;
extern unsigned char btnRIGHThandled;
extern unsigned char btnDOWNhandled;
extern unsigned char btnMIDDLEhandled;




#endif	/* GAME_H */