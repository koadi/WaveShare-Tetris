/* 
 * File:   highscores.h
 * Author: Phil
 *
 * Created on September 24, 2012, 10:30 AM
 */

#ifndef HIGHSCORES_H
#define	HIGHSCORES_H

#include <stdio.h>
#include <string.h>
#include "game.h"
#include "glcd.h"

extern char HiScoreNames[10][12];
extern unsigned long HiScores[10];

void SaveHighScores(void);
void LoadHighScores(void);
void DisplayHighScores(void);
unsigned char IsHighScore(unsigned long NewScore);
void InsertHighScore(unsigned long NewScore, char *NewName);
void NameEntry_Underline2(unsigned char LetterIndex);
void NameEntry_Underline1(unsigned char LetterIndex);
void NameEntry_Letter(unsigned char LetterIndex, char Letter);
void DoNameEntry(unsigned long TopScore);



#endif	/* HIGHSCORES_H */

