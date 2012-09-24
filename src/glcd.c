#include <xc.h>
#include <stdlib.h>
#include "glcd.h"


unsigned int TextX = 0;
unsigned int TextY = 0;
unsigned int TextFG = 0xFFFF;
unsigned int TextBG = 0x0000;

void GLCD_Reset(void)
{
    GLCD_Reset_Low();
    GLCD_Reset_Delay();
    GLCD_Reset_High();
    GLCD_Reset_Delay();
}

void GLCD_Post_Data(unsigned int data)
{
    SSPBUF = (unsigned char)(data >> 8);
    while(!SSPIF) NOP();
    SSPIF = 0;
    SSPBUF = (unsigned char)(data);
    while(!SSPIF) NOP();
    SSPIF = 0;
}

void GLCD_Post_Command(unsigned int index, unsigned int command)
{
    GLCD_Index();
    GLCD_Post_Data(index);
    GLCD_Data();
    GLCD_Post_Data(command);
}

void glcd_Init(void)
{
    unsigned int x,y;

    TRISC = 0x10;
    SSPSTAT = 0x80;
    SSPCON1 = 0x30;
    INTCON = 0x00;
    PIR1 = 0x00;

    GLCD_Enable();
    GLCD_Reset();

    GLCD_Post_Command(0x0003, 0x0001);

    // oscillator start
    GLCD_Post_Command(0x003A, 0x0001);

    // Y Setting
    GLCD_Post_Command(0x0024,0x007B);
    GLCD_Post_Command(0x0025,0x003B);
    GLCD_Post_Command(0x0026,0x0034);
    GLCD_Post_Command(0x0027,0x0004);
    GLCD_Post_Command(0x0052,0x0025);
    GLCD_Post_Command(0x0053,0x0033);
    GLCD_Post_Command(0x0061,0x001C);
    GLCD_Post_Command(0x0062,0x002C);
    GLCD_Post_Command(0x0063,0x0022);
    GLCD_Post_Command(0x0064,0x0027);
    GLCD_Post_Command(0x0065,0x0014);
    GLCD_Post_Command(0x0066,0x0010);

    // Basical clock for 1 line (BASECOUNT(7:0)) number
    GLCD_Post_Command(0x002E, 0x002D);

    //Power supply setting
    GLCD_Post_Command(0x0019,0x0000);
    GLCD_Post_Command(0x001A,0x1000);
    GLCD_Post_Command(0x001B,0x0023);
    GLCD_Post_Command(0x001C,0x0C01);
    GLCD_Post_Command(0x001D,0x0000);
    GLCD_Post_Command(0x001E,0x0009);
    GLCD_Post_Command(0x001F,0x0035);
    GLCD_Post_Command(0x0020,0x0015);
    GLCD_Post_Command(0x0018,0x1E7B);

    //windows setting
    GLCD_Post_Command(0x0008,0x0000);
    GLCD_Post_Command(0x0009,0x00EF);
    GLCD_Post_Command(0x000a,0x0000);
    GLCD_Post_Command(0x000b,0x013F);

    //LCD display area setting
    GLCD_Post_Command(0x0029,0x0000);
    GLCD_Post_Command(0x002A,0x0000);
    GLCD_Post_Command(0x002B,0x00EF);
    GLCD_Post_Command(0x002C,0x013F);

    //Gate scan setting
    GLCD_Post_Command(0x0032,0x0002);

    //n line inversion line number
    GLCD_Post_Command(0x0033,0x0000);

    //Line inversion/frame inversion/interlace setting
    GLCD_Post_Command(0x0037,0x0000);

    //GOE1,GOE2 signal start
    GLCD_Post_Command(0x003B,0x0001);

    //Color mode
    GLCD_Post_Command(0x0004,0x0000);

    //windows mode setting
    GLCD_Post_Command(0x0005,0x0010);

    //Display setting register 2
    GLCD_Post_Command(0x0001,0x0000);

    //display setting
    GLCD_Post_Command(0x0000,0x0000);//display on

    GLCD_Index();
    GLCD_Post_Data(0x0E);
    GLCD_Data();
    for( x = 0; x < 240; x++)
    {
        for( y = 0; y < 320; y++)
        {
            GLCD_Post_Data(COLOR_BLACK);
        }
    }

    GLCD_Disable();
}

void glcd_SetPixel(unsigned int X, unsigned int Y, unsigned int Color)
{
    GLCD_Enable();
    //GLCD_Post_Command(0x08, X*8);   // X start
    //GLCD_Post_Command(0x0A, Y*16);  // Y start
    //GLCD_Post_Command(0x09, X*8);   // X end
    //GLCD_Post_Command(0x0B, Y*16);  // Y end
    GLCD_Post_Command(0x0005, 0x0000);
    GLCD_Post_Command(0x06, X);   // RAM X
    GLCD_Post_Command(0x07, Y);  // RAM Y

    GLCD_Index();
    GLCD_Post_Data(0x0E);   // RAM Write
    GLCD_Data();
    GLCD_Post_Data(Color);
    GLCD_Disable();
}

void glcd_Line(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int Color) {
    signed int sx;
    signed int sy;
    if(x0 > x1) {
        // swap for slope and horizontal left/right
        sx = x0;
        sy = y0;
        x0 = x1;
        y0 = y1;
        x1 = sx;
        y1 = sy;
    }
    if(x0 == x1) {
        // vertical line
        if(y0 > y1) {
            // swap for vertical top/bottom
            sx = x0;
            sy = y0;
            x0 = x1;
            y0 = y1;
            x1 = sx;
            y1 = sy;
        }
        for(sy=y0;sy<=y1;sy++) {
            glcd_SetPixel(x0,sy,Color);
        }
        return;
    }
    if(y0 == y1) {
        // horizontal line
        for(sx=x0;sx<=x1;sx++) {
            glcd_SetPixel(sx,y0,Color);
        }
        return;
    }
    signed int dx = abs(x1-x0);
    signed int dy = abs(y1-y0);

    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    signed int err = dx - dy;
    signed int e2;
    while( (x0 != x1) && (y0 != y1) ) {
        glcd_SetPixel(x0,y0,Color);
        e2 = 2 * err;
        if( e2 > -dy ) {
            err -= dy;
            x0 += sx;
        }
        if( e2 < dx ) {
            err += dx;
            y0 += sy;
        }
    }
    glcd_SetPixel(x0,y0,Color);
}

void glcd_circle_plot4points(unsigned int cx, unsigned int cy, unsigned int x, unsigned int y, unsigned int Color) {
    glcd_SetPixel(cx + x, cy + y, Color);
    if(x != 0) glcd_SetPixel(cx - x, cy + y, Color);
    if(y != 0) glcd_SetPixel(cx + x, cy - y, Color);
    glcd_SetPixel(cx - x, cy - y, Color);
}

void glcd_circle_plot8points(unsigned int cx, unsigned int cy, unsigned int x, unsigned int y, unsigned int Color) {
    glcd_circle_plot4points(cx, cy, x, y, Color);
    glcd_circle_plot4points(cx, cy, y, x, Color);
}

void glcd_Circle(unsigned int cx, unsigned int cy, unsigned int radius, unsigned int Color) {
    signed int err = -radius;
    unsigned int x = radius;
    unsigned int y = 0;
    while( x > y ) {
        glcd_circle_plot8points(cx,cy,x,y,Color);
        err += y;
        y++;
        err += y;
        if( err >= 0 ) {
            err -= x;
            x--;
            err -= x;
        }
    }
    glcd_circle_plot4points(cx,cy,x,y,Color);
}

void glcd_ellipse_plot4points(unsigned int cx, unsigned int cy, signed int x, signed int y, unsigned int Color) {
    glcd_SetPixel(cx+x, cy+y, Color);
    glcd_SetPixel(cx-x, cy+y, Color);
    glcd_SetPixel(cx-x, cy-y, Color);
    glcd_SetPixel(cx+x, cy-y, Color);
}

void glcd_Ellipse(unsigned int CenterX, unsigned int CenterY, unsigned int XRadius, unsigned int YRadius, unsigned int Color) {
    signed int x;
    signed int y;
    signed long xChange;
    signed long yChange;
    signed long err;
    unsigned int twoAsqr;
    unsigned int twoBsqr;
    signed long stoppingX;
    signed long stoppingY;

    twoAsqr = 2 * XRadius * XRadius;
    twoBsqr = 2 * YRadius * YRadius;
    x = XRadius;
    y = 0;
    xChange = YRadius * YRadius * (2 * XRadius * -1);
    yChange = XRadius * XRadius;
    err = 0;
    stoppingX = twoBsqr * XRadius;
    stoppingY = 0;
    while( stoppingX >= stoppingY ) {
        glcd_ellipse_plot4points(CenterX,CenterY,x,y,Color);
        y++;
        stoppingY += twoAsqr;
        err += yChange;
        yChange += twoAsqr;
        if( (2 * err + xChange) > 0 ) {
            x--;
            stoppingX -= twoBsqr;
            err += xChange;
            xChange += twoBsqr;
        }
    }

    x = 0;
    y = YRadius;
    xChange = YRadius * YRadius;
    yChange = XRadius * XRadius * (2 * YRadius * -1);
    err = 0;
    stoppingX = 0;
    stoppingY = twoAsqr * YRadius;
    while( stoppingX <= stoppingY ) {
        glcd_ellipse_plot4points(CenterX,CenterY,x,y,Color);
        x++;
        stoppingX += twoBsqr;
        err += xChange;
        xChange += twoBsqr;
        if( (2 * err + yChange) > 0 ) {
            y--;
            stoppingY -= twoAsqr;
            err += yChange;
            yChange += twoAsqr;
        }
    }


}

void glcd_Rectangle(unsigned int CornerX, unsigned int CornerY, unsigned int XLength, unsigned int YLength, unsigned int Color) {
    XLength -= 1;
    YLength -= 1;
    glcd_Line(CornerX, CornerY, CornerX+XLength, CornerY, Color);
    glcd_Line(CornerX+XLength, CornerY,CornerX+XLength, CornerY+YLength, Color);
    glcd_Line(CornerX, CornerY+YLength, CornerX+XLength, CornerY+YLength, Color);
    glcd_Line(CornerX, CornerY, CornerX, CornerY+YLength, Color);
}

void glcd_22_filled_rectangle(unsigned int CornerX, unsigned int CornerY, unsigned int XLength, unsigned int YLength, unsigned int Color)
{
    unsigned long i = (unsigned long)XLength * YLength;
    GLCD_Enable();
    GLCD_Post_Command(0x08, CornerX);   // X start
    GLCD_Post_Command(0x0A, CornerY);  // Y start
    GLCD_Post_Command(0x09, CornerX+XLength-1);   // X end
    GLCD_Post_Command(0x0B, CornerY+YLength-1);  // Y end
    GLCD_Post_Command(0x0005, 0x0010);
    //GLCD_Post_Command(0x06, CornerX);   // RAM X
    //GLCD_Post_Command(0x07, CornerY);  // RAM Y

    GLCD_Index();
    GLCD_Post_Data(0x0E);   // RAM Write
    GLCD_Data();
    for( i = i; i != 0; i-- ) GLCD_Post_Data(Color);
    GLCD_Disable();
}

void glcd_FilledRectangle(unsigned int CornerX, unsigned int CornerY, unsigned int XLength, unsigned int YLength, unsigned int Color)
{
    glcd_22_filled_rectangle(CornerX, CornerY, XLength, YLength, Color);
}

void glcd_Square(unsigned int CornerX, unsigned int CornerY, unsigned int Length, unsigned int Color) {
    glcd_Rectangle(CornerX, CornerY, Length, Length, Color);
}



void glcd_Print(const char *Text, unsigned char Font, unsigned int CornerX, unsigned int CornerY, unsigned char CenterX, unsigned int FGColor, unsigned int BGColor) {
    unsigned char fontWidth;
    unsigned char fontHeight;
    unsigned char fontSpacing;
    unsigned char fontBytes;
    unsigned char textLength = 0;
    unsigned char i;
    unsigned char byte2;
    unsigned char byte4;
    unsigned int int1;
    unsigned int x;
    unsigned int y;
    const unsigned char *ptrFont;

    switch( Font ) {
        case 0:

        case 1:
            ptrFont = &glcd_font_5x7;
            fontWidth = 5;
            fontHeight = 7;
            fontSpacing = 1;
            fontBytes = 5;
        case 2:
            ptrFont = &glcd_font_10x14;
            fontWidth = 10;
            fontHeight = 14;
            fontSpacing = 2;
            fontBytes = 18;
    }
    if(CenterX == 1) {
        for(i=0;Text[i] != 0;i++) {
            textLength++;
        }
        CornerX = (240 - (textLength * (fontWidth + fontSpacing)))/2;
    }
    for(i=0;Text[i] != 0;i++) {
        byte2 = 8;
        int1 = 0;
        for(y=0;y<fontHeight;y++) {
            for(x=0;x<fontWidth;x++) {
                if( byte2 == 8 ) {
                    byte2 = 0;
                    byte4 = ptrFont[(((Text[i] - 32) * fontBytes) + int1)];
                    int1++;
                }
                if( ((1 << byte2) & byte4) == 0 ) {
                    glcd_SetPixel(x+CornerX,y+CornerY,BGColor);
                } else {
                    glcd_SetPixel(x+CornerX,y+CornerY,FGColor);
                }
                byte2++;
            }
        }
        CornerX += fontWidth + fontSpacing;
    }
}


void glcd_PrintChar(char Text, unsigned char Font, unsigned int CornerX, unsigned int CornerY, unsigned int FGColor, unsigned int BGColor) {
    unsigned char fontWidth;
    unsigned char fontHeight;
    unsigned char fontSpacing;
    unsigned char fontBytes;
    unsigned char i;
    unsigned char byte2;
    unsigned char byte4;
    unsigned int int1;
    unsigned int x;
    unsigned int y;
    const unsigned char *ptrFont;

    switch( Font ) {
        case 0:

        case 1:
            ptrFont = &glcd_font_5x7;
            fontWidth = 5;
            fontHeight = 7;
            fontSpacing = 1;
            fontBytes = 5;
        case 2:
            ptrFont = &glcd_font_10x14;
            fontWidth = 10;
            fontHeight = 14;
            fontSpacing = 2;
            fontBytes = 18;
    }
    byte2 = 8;
    int1 = 0;
    for(y=0;y<fontHeight;y++) {
        for(x=0;x<fontWidth;x++) {
            if( byte2 == 8 ) {
                byte2 = 0;
                byte4 = ptrFont[(((Text - 32) * fontBytes) + int1)];
                int1++;
            }
            if( ((1 << byte2) & byte4) == 0 ) {
                glcd_SetPixel(x+CornerX,y+CornerY,BGColor);
            } else {
                glcd_SetPixel(x+CornerX,y+CornerY,FGColor);
            }
            byte2++;
        }
    }
}

void glcd_DrawBitmap2X(const unsigned char *BitmapData, const unsigned int *BitmapPalette, unsigned int OriginX, unsigned int OriginY, unsigned int BitmapWidth, unsigned int BitmapHeight)
{
    unsigned int x, y;
    unsigned int xA, yA;
    unsigned int i = 0;

    for( y = 0; y < BitmapHeight; y++ )
    {
        for( x = 0; x < BitmapWidth; x++ )
        {
            xA = x * 2 + OriginX;
            yA = y * 2 + OriginY;
            glcd_SetPixel(xA, yA, BitmapPalette[BitmapData[i]]);
            glcd_SetPixel(xA+1, yA, BitmapPalette[BitmapData[i]]);
            glcd_SetPixel(xA, yA+1, BitmapPalette[BitmapData[i]]);
            glcd_SetPixel(xA+1, yA+1, BitmapPalette[BitmapData[i]]);
            i++;
        }
    }
}

void putch(char c)
{
    glcd_PrintChar(c, 2, TextX, TextY, TextFG, TextBG);
    TextX +=12;
}