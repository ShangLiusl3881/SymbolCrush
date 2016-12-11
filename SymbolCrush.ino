/* Symbol Crush Game (like Candy Crush) for Arduino UNO
 * Final project for ELEN 4193 Modern Display
 * Copyright © 2016 Shang Liu & Yunzhe Li. All rights reserved.
 * 
 * Author: Shang Liu (shang.liu@columbia.edu)
 *         Yunzhe Li (yunzhe.li@columbia.edu)
 * 
 * Feature: 
 *    1.  VGA for screen and RCA for audio   
 *    2.  5 buttons for user interface
 *    3.  Audio volume adjustable (using a potentiometer)
 *    4.  Colorful game (4 color)
 *    5.  Button press sound
 *    6.  Symbol exchange animation
 *    7.  Symbol crush animation
 *    8.  Symbol falling animation
 *    9.  Show score and exchange step left
 *    10. Opening/Ending music
 *    
 * Acknowledgement:
 *    1.  Thank Sandro Maffiodo for his VGAX library
 *    2.  Thank Prof.Kymissis and Caroline for their instruction and help
 */

#include "VGAX.h"
#include "VGAXUtils.h"
#include <math.h>

#define ENABLE_SOUND 1

#define FNT_NANOFONT_HEIGHT 6
#define FNT_NANOFONT_SYMBOLS_COUNT 95

#define BUTTON_1 A3 //digital right
#define BUTTON_2 A1 //digital left
#define BUTTON_3 A4 //digital down
#define BUTTON_4 A2 //digital up
#define BUTTON_OK 2 //digital select

// NB: pin A0 is used for the audio!

#define xWidth 15
#define yHeight 9
#define xCell 6
#define yCell 6

#define BLINKCOUNT 2

#define STEPLEFT 5

#define NOTE_NUMBER 14
#define MUSIC_LENGTH 36
#define NOTE_TIME 9

enum stateName {gameOpen,start,screenIni,cursor1,cursor2,checkNeighbor,exchange,checkPartial,exchangeBack,crush,fall,checkWhole,gameOver,endSound, endDelay,ending} state;

VGAX vga;
VGAXUtils vgaU;

const int noteFrequency[NOTE_NUMBER] PROGMEM = {261,293,329,349,392,440,494,523,587,659,698,783,880,988};
const char music [MUSIC_LENGTH] PROGMEM =    {5,8,9,10,9,8,8,0,5,8,9,10,9,8,9,10,10,0,5,8,9,10,9,8,8,0,5,8,9,10,9,8,9,12,9,0};
const char musicTime[MUSIC_LENGTH] PROGMEM = {1,1,1,1, 2,1,3,2,1,1,1,1, 2,1,2,2, 2, 2,1,1,1,1, 2,1,3,2,1,1,1,1, 2,1,2,2, 3,2};

int counterMusic = 0;
int counterMusic2 = 0;
int counterMenu = 0;
int counterMenu2 = 0; 

boolean button1 = 0;
boolean button2 = 0;
boolean button3 = 0;
boolean button4 = 0;
boolean buttonOK = 0;
boolean button=0;
boolean buttonLast =0;
boolean buttonOKLast =0;
int speedDelay = 150; 
int blinkCount = 0;

char x1=0;
char y1=0;
char x1old=x1;
char y1old=y1;
char x2=0;
char y2=0;
char x2old=x2;
char y2old=y2;

boolean blinkShowFlag=0;
boolean checkPartialFlag = 0;
boolean checkWholeFlag=0;
char maxFallStep=0;

unsigned char object [yHeight]  [xWidth];

char stepLeft = STEPLEFT;
int score = 0;

char endSoundCount=0;

//data size=570 bytes
const unsigned char fnt_nanofont_data[FNT_NANOFONT_SYMBOLS_COUNT][1+FNT_NANOFONT_HEIGHT] PROGMEM={
{ 1, 128, 128, 128, 0, 128, 0, }, //glyph '!' code=0
{ 3, 160, 160, 0, 0, 0, 0, }, //glyph '"' code=1
//{ 5, 80, 248, 80, 248, 80, 0, },  //glyph '#' code=2
{ 5, 248, 248, 248, 248, 248, 0, },  //glyph '#' code=2 - full rectangle
{ 5, 120, 160, 112, 40, 240, 0, },  //glyph '$' code=3
{ 5, 136, 16, 32, 64, 136, 0, },  //glyph '%' code=4
{ 5, 96, 144, 104, 144, 104, 0, },  //glyph '&' code=5
{ 2, 128, 64, 0, 0, 0, 0, },  //glyph ''' code=6
{ 2, 64, 128, 128, 128, 64, 0, }, //glyph '(' code=7
{ 2, 128, 64, 64, 64, 128, 0, },  //glyph ')' code=8
{ 3, 0, 160, 64, 160, 0, 0, },  //glyph '*' code=9
{ 3, 0, 64, 224, 64, 0, 0, }, //glyph '+' code=10
{ 2, 0, 0, 0, 0, 128, 64, },  //glyph ',' code=11
{ 3, 0, 0, 224, 0, 0, 0, }, //glyph '-' code=12
{ 1, 0, 0, 0, 0, 128, 0, }, //glyph '.' code=13
{ 5, 8, 16, 32, 64, 128, 0, },  //glyph '/' code=14
{ 4, 96, 144, 144, 144, 96, 0, }, //glyph '0' code=15
{ 3, 64, 192, 64, 64, 224, 0, },  //glyph '1' code=16
{ 4, 224, 16, 96, 128, 240, 0, }, //glyph '2' code=17
{ 4, 224, 16, 96, 16, 224, 0, },  //glyph '3' code=18
{ 4, 144, 144, 240, 16, 16, 0, }, //glyph '4' code=19
{ 4, 240, 128, 224, 16, 224, 0, },  //glyph '5' code=20
{ 4, 96, 128, 224, 144, 96, 0, }, //glyph '6' code=21
{ 4, 240, 16, 32, 64, 64, 0, }, //glyph '7' code=22
{ 4, 96, 144, 96, 144, 96, 0, },  //glyph '8' code=23
{ 4, 96, 144, 112, 16, 96, 0, },  //glyph '9' code=24
{ 1, 0, 128, 0, 128, 0, 0, }, //glyph ':' code=25
{ 2, 0, 128, 0, 0, 128, 64, },  //glyph ';' code=26
{ 3, 32, 64, 128, 64, 32, 0, }, //glyph '<' code=27
{ 3, 0, 224, 0, 224, 0, 0, }, //glyph '=' code=28
{ 3, 128, 64, 32, 64, 128, 0, },  //glyph '>' code=29
{ 4, 224, 16, 96, 0, 64, 0, },  //glyph '?' code=30
{ 4, 96, 144, 176, 128, 112, 0, },  //glyph '@' code=31
{ 4, 96, 144, 240, 144, 144, 0, },  //glyph 'A' code=32
{ 4, 224, 144, 224, 144, 224, 0, }, //glyph 'B' code=33
{ 4, 112, 128, 128, 128, 112, 0, }, //glyph 'C' code=34
{ 4, 224, 144, 144, 144, 224, 0, }, //glyph 'D' code=35
{ 4, 240, 128, 224, 128, 240, 0, }, //glyph 'E' code=36
{ 4, 240, 128, 224, 128, 128, 0, }, //glyph 'F' code=37
{ 4, 112, 128, 176, 144, 112, 0, }, //glyph 'G' code=38
{ 4, 144, 144, 240, 144, 144, 0, }, //glyph 'H' code=39
{ 3, 224, 64, 64, 64, 224, 0, },  //glyph 'I' code=40
{ 4, 240, 16, 16, 144, 96, 0, },  //glyph 'J' code=41
{ 4, 144, 160, 192, 160, 144, 0, }, //glyph 'K' code=42
{ 4, 128, 128, 128, 128, 240, 0, }, //glyph 'L' code=43
{ 5, 136, 216, 168, 136, 136, 0, }, //glyph 'M' code=44
{ 4, 144, 208, 176, 144, 144, 0, }, //glyph 'N' code=45
{ 4, 96, 144, 144, 144, 96, 0, }, //glyph 'O' code=46
{ 4, 224, 144, 224, 128, 128, 0, }, //glyph 'P' code=47
{ 4, 96, 144, 144, 144, 96, 16, },  //glyph 'Q' code=48
{ 4, 224, 144, 224, 160, 144, 0, }, //glyph 'R' code=49
{ 4, 112, 128, 96, 16, 224, 0, }, //glyph 'S' code=50
{ 3, 224, 64, 64, 64, 64, 0, }, //glyph 'T' code=51
{ 4, 144, 144, 144, 144, 96, 0, },  //glyph 'U' code=52
{ 3, 160, 160, 160, 160, 64, 0, },  //glyph 'V' code=53
{ 5, 136, 168, 168, 168, 80, 0, },  //glyph 'W' code=54
{ 4, 144, 144, 96, 144, 144, 0, },  //glyph 'X' code=55
{ 3, 160, 160, 64, 64, 64, 0, },  //glyph 'Y' code=56
{ 4, 240, 16, 96, 128, 240, 0, }, //glyph 'Z' code=57
{ 2, 192, 128, 128, 128, 192, 0, }, //glyph '[' code=58
{ 5, 128, 64, 32, 16, 8, 0, },  //glyph '\' code=59
{ 2, 192, 64, 64, 64, 192, 0, },  //glyph ']' code=60
{ 5, 32, 80, 136, 0, 0, 0, }, //glyph '^' code=61
{ 4, 0, 0, 0, 0, 240, 0, }, //glyph '_' code=62
{ 2, 128, 64, 0, 0, 0, 0, },  //glyph '`' code=63
{ 3, 0, 224, 32, 224, 224, 0, },  //glyph 'a' code=64
{ 3, 128, 224, 160, 160, 224, 0, }, //glyph 'b' code=65
{ 3, 0, 224, 128, 128, 224, 0, }, //glyph 'c' code=66
{ 3, 32, 224, 160, 160, 224, 0, },  //glyph 'd' code=67
{ 3, 0, 224, 224, 128, 224, 0, }, //glyph 'e' code=68
{ 2, 64, 128, 192, 128, 128, 0, },  //glyph 'f' code=69
{ 3, 0, 224, 160, 224, 32, 224, },  //glyph 'g' code=70
{ 3, 128, 224, 160, 160, 160, 0, }, //glyph 'h' code=71
{ 1, 128, 0, 128, 128, 128, 0, }, //glyph 'i' code=72
{ 2, 0, 192, 64, 64, 64, 128, },  //glyph 'j' code=73
{ 3, 128, 160, 192, 160, 160, 0, }, //glyph 'k' code=74
{ 1, 128, 128, 128, 128, 128, 0, }, //glyph 'l' code=75
{ 5, 0, 248, 168, 168, 168, 0, }, //glyph 'm' code=76
{ 3, 0, 224, 160, 160, 160, 0, }, //glyph 'n' code=77
{ 3, 0, 224, 160, 160, 224, 0, }, //glyph 'o' code=78
{ 3, 0, 224, 160, 160, 224, 128, }, //glyph 'p' code=79
{ 3, 0, 224, 160, 160, 224, 32, },  //glyph 'q' code=80
{ 3, 0, 224, 128, 128, 128, 0, }, //glyph 'r' code=81
{ 2, 0, 192, 128, 64, 192, 0, },  //glyph 's' code=82
{ 3, 64, 224, 64, 64, 64, 0, }, //glyph 't' code=83
{ 3, 0, 160, 160, 160, 224, 0, }, //glyph 'u' code=84
{ 3, 0, 160, 160, 160, 64, 0, },  //glyph 'v' code=85
{ 5, 0, 168, 168, 168, 80, 0, },  //glyph 'w' code=86
{ 3, 0, 160, 64, 160, 160, 0, },  //glyph 'x' code=87
{ 3, 0, 160, 160, 224, 32, 224, },  //glyph 'y' code=88
{ 2, 0, 192, 64, 128, 192, 0, },  //glyph 'z' code=89
{ 3, 96, 64, 192, 64, 96, 0, }, //glyph '{' code=90
{ 1, 128, 128, 128, 128, 128, 0, }, //glyph '|' code=91
{ 3, 192, 64, 96, 64, 192, 0, },  //glyph '}' code=92
{ 3, 96, 192, 0, 0, 0, 0, },  //glyph '~' code=93
{ 4, 48, 64, 224, 64, 240, 0, },  //glyph '£' code=94
};

static const char str0[] PROGMEM="0"; 
static const char str1[] PROGMEM="1"; 
static const char str2[] PROGMEM="2"; 
static const char str3[] PROGMEM="3"; 
static const char str4[] PROGMEM="4"; 
static const char str5[] PROGMEM="5"; 
static const char str6[] PROGMEM="6"; 
static const char str7[] PROGMEM="7"; 
static const char str8[] PROGMEM="8"; 
static const char str9[] PROGMEM="9"; 
static const char str10[] PROGMEM="ELEN 4193 MODERN DISPLAY"; 
static const char str11[] PROGMEM="GAME : SYMBOL CRUSH";
static const char str12[] PROGMEM="BY SHANG LIU";
static const char str13[] PROGMEM="   YUNZHE LI"; 
static const char str14[] PROGMEM="STEP";
static const char str15[] PROGMEM="LEFT:";
static const char str16[] PROGMEM="SCORE:"; 
static const char str17[] PROGMEM="GAME OVER !"; 

void setup() {
  vga.begin(0);
  randomSeed(analogRead(5)); 
  state = start;
}

void playMusic(int index){
  #if ENABLE_SOUND == 1
  char note = pgm_read_byte(music+index);  
  if(note>0){
    pinMode(A0, OUTPUT);
    vga.tone(pgm_read_word(noteFrequency+note-1));
  }
  else{
    vga.noTone();
  }
  #endif
}

void musicFunc(){
  counterMusic2++; 
  if(counterMusic2==NOTE_TIME*pgm_read_byte(musicTime+counterMusic)){
    counterMusic2=0;
    counterMusic++;    
    if(counterMusic==MUSIC_LENGTH){
      counterMusic=0; 
    }   
    #if ENABLE_SOUND == 1
      playMusic(counterMusic);
    #endif
  }
}

void drawOpen(){
  for(int i=0;i<=120;i=i+6){
    //setRectBackground(0,0,120,60,3);
    clearRect(120-i,10,i,5);
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str10, 120-i, 10, 1);
    clearRect(120-i,20,i,5);
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str11, 126-i, 20, 1);
    clearRect(120-i,40,i,5);
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str12, 140-i, 40, 1);
    clearRect(120-i,50,i,5);
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str13, 140-i, 50, 1);
    vga.delay(200);
  }
}

void drawStart() {
  counterMenu2++;   
  //vga.delay(10); 
  if (counterMenu2 > 50){
    counterMenu++; 
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str10, 0, 10, (counterMenu%3) + 1);
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str11, 6, 20, (counterMenu%3) + 1);
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str12, 20, 40, (counterMenu%3) + 1);
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str13, 20, 50, (counterMenu%3) + 1);
    counterMenu2 = 0; 
  }  
  musicFunc();  
}

void drawStepLeft(){
  vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str14, 97, 0, 1);
  vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str15, 97, 7, 1);
  char tenDigit, oneDigit;
  tenDigit = stepLeft/10;
  oneDigit = stepLeft%10;
  int i;
  for(i=0;i<2;i++){
    char digit;
    char x;
    if(i==0){
      digit=tenDigit;
      x=102;
    }
    else{
      digit=oneDigit;
      x=107;
    }
    //clearBlock(x,14);
    clearRect(x,14,4,5);
    if (digit == 0) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str0, x, 14, 1);}
    if (digit == 1) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str1, x, 14, 1);}
    if (digit == 2) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str2, x, 14, 1);}
    if (digit == 3) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str3, x, 14, 1);}
    if (digit == 4) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str4, x, 14, 1);}
    if (digit == 5) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str5, x, 14, 1);}
    if (digit == 6) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str6, x, 14, 1);}
    if (digit == 7) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str7, x, 14, 1);}
    if (digit == 8) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str8, x, 14, 1);}
    if (digit == 9) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str9, x, 14, 1);}
  }
}

void drawScore(){
  char y=37;
  char col=3;
  vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str16, 93, 30, col);
  char hundredDigit, tenDigit, oneDigit;
  hundredDigit = score/100;
  tenDigit = (score/10)%10;
  oneDigit = score%10;
  //hundredDigit=0;
  //tenDigit = 0;
  //oneDigit = 0;
  
  for(int i=0;i<3;i++){
    char digit;
    char x;
    if(i==0){
      digit=hundredDigit;
      x=100;
    }
    else if(i==1){
      digit=tenDigit;
      x=105;
    }
    else{
      digit=oneDigit;
      x=110;
    }
    clearRect(x,y,4,5);
    if (digit == 0) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str0, x, y, col);}
    if (digit == 1) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str1, x, y, col);}
    if (digit == 2) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str2, x, y, col);}
    if (digit == 3) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str3, x, y, col);}
    if (digit == 4) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str4, x, y, col);}
    if (digit == 5) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str5, x, y, col);}
    if (digit == 6) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str6, x, y, col);}
    if (digit == 7) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str7, x, y, col);}
    if (digit == 8) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str8, x, y, col);}
    if (digit == 9) {vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str9, x, y, col);}
  }
}

void objectIni(){
  for(int y=0;y<yHeight;y++){
    for(int x=0;x<xWidth;x++){
      object[y][x]=x%5+1;     
      object[y][x]=random(1,6);
      if(x>=2&&y<2){
        while(object[y][x]==object[y][x-1]&&object[y][x]==object[y][x-2]){
          object[y][x]=random(1,6);
        }
      }
      else if(x<2&&y>=2){
        while(object[y][x]==object[y-1][x]&&object[y][x]==object[y-2][x]){
          object[y][x]=random(1,6);
        }
      }
      else if(x>=2&&y>=2){
        while((object[y][x]==object[y][x-1]&&object[y][x]==object[y][x-2])||(object[y][x]==object[y-1][x]&&object[y][x]==object[y-2][x])){
          object[y][x]=random(1,6);
        }
      }      
    }
  }
}

void processInputs() {
  button1 = digitalRead(BUTTON_1); 
  button2 = digitalRead(BUTTON_2);
  button3 = digitalRead(BUTTON_3); 
  button4 = digitalRead(BUTTON_4);
  buttonOK = digitalRead(BUTTON_OK);
  button = button1 | button2 | button3 | button4; 
}

void clearBlock(int X, int Y){  
  for(int i=0;i<yCell;i++){
    if((Y+i)>=0){
      vgaU.draw_row(Y+i, X, X+xCell,0);
    }
  }
}

void clearRect(int X, int Y, int width, int height){
  for(int i=0;i<height;i++){
    vgaU.draw_row(Y+i,X, X+width,0);
  }
}

void setRectBackground(int X, int Y, int width, int height,int col){
  for(int i=0;i<height;i++){
    vgaU.draw_row(Y+i,X, X+width,col);
  }
}

void drawLightning(int X, int Y){
  int col = 1;
  if(Y>=0){
    vga.putpixel(X+2, Y, col); 
    vgaU.draw_row(Y,X,X+2,0);
    vgaU.draw_row(Y,X+3,X+6,0);
  }
  if((Y+1)>=0){
    vgaU.draw_row(Y+1, X+1, X+3, col);
    vga.putpixel(X, Y+1, 0); 
    vgaU.draw_row(Y+1,X+3,X+6,0);
  }
  if((Y+2)>=0){
    vgaU.draw_row(Y+2, X, X+5, col);
    vga.putpixel(X+5, Y+2, 0); 
  }
  if((Y+3)>=0){
    vgaU.draw_row(Y+3, X+2, X+4, col);
    vgaU.draw_row(Y+3, X, X+2, 0);
    vgaU.draw_row(Y+3,X+4,X+6,0);
  }
  if((Y+4)>=0){
    vga.putpixel(X+2, Y+4, col); 
    vgaU.draw_row(Y+4,X,X+2,0);
    vgaU.draw_row(Y+4,X+3,X+6,0); 
  }
  if((Y+5)>=0){
    vgaU.draw_row(Y+5,X,X+6,0);
  }
}

void drawSmile(int X, int Y){
  int col = 1;
  if(Y>=0){
    vgaU.draw_row(Y, X, X+2, col);
    vgaU.draw_row( Y, X+3,X+5,col);
    vga.putpixel(X+2, Y, 0);
    vga.putpixel(X+5, Y, 0); 
  }
  if((Y+1)>=0){
    vgaU.draw_row(Y+1, X, X+2,  col);
    vgaU.draw_row(Y+1, X+3, X+5, col);
    vga.putpixel(X+2, Y+1, 0);
    vga.putpixel(X+5, Y+1, 0);
  }
  if((Y+2)>=0){
    vgaU.draw_row(Y+2, X, X+6,  0);
  }
  if((Y+3)>=0){
    vga.putpixel(X, Y+3, col);
    vga.putpixel(X+4, Y+3, col);
    vgaU.draw_row(Y+3, X+1, X+4,  0);
    vga.putpixel(X+5, Y+3, 0);
  }
  if((Y+4)>=0){
    vgaU.draw_row(Y+4, X+1, X+4, col);
    vga.putpixel(X, Y+4, 0);
    vgaU.draw_row(Y+4, X+4, X+6, 0);
  }
  if((Y+5)>=0){
    vgaU.draw_row(Y+5,X,X+6,0);
  }
}

void drawSad(int X, int Y){
  int col = 3;
  if(Y>=0){
    vgaU.draw_row(Y, X, X+2, col);
    vgaU.draw_row(Y, X+3, X+5, col);
  }
  if((Y+3)>=0){
    vgaU.draw_row(Y+3, X+1, X+4, col);
  }
  if((Y+4)>=0){
    vga.putpixel(X, Y+4, col);
    vga.putpixel(X+4, Y+4, col);
  }
}

void drawRhombus(int X, int Y){
  int col = 3;
  if(Y>=0){
    vga.putpixel(X+2, Y, col);
    vgaU.draw_row(Y, X, X+2, 0);
    vgaU.draw_row(Y, X+3, X+6, 0);
  }
  if((Y+1)>=0){
    vga.putpixel(X+1, Y+1, col);
    vga.putpixel(X+3, Y+1, col);
    vga.putpixel(X, Y+1, 0);
    vga.putpixel(X+2, Y+1, 0);
    vgaU.draw_row(Y+1, X+4, X+6, 0);
  }
  if((Y+2)>=0){
    vga.putpixel(X, Y+2, col);
    vga.putpixel(X+4, Y+2, col);
    vgaU.draw_row(Y+2, X+1, X+4, 0);
    vga.putpixel(X+5, Y+2, 0);
  }
  if((Y+3)>=0){
    vga.putpixel(X+1, Y+3, col);
    vga.putpixel(X+3, Y+3, col);
    vga.putpixel(X, Y+3, 0);
    vga.putpixel(X+2, Y+3, 0);
    vgaU.draw_row(Y+3, X+4, X+6, 0);
  }
  if((Y+4)>=0){
    vga.putpixel(X+2, Y+4, col);
    vgaU.draw_row(Y+4, X, X+2, 0);
    vgaU.draw_row(Y+4, X+3, X+6, 0);
  }
  if((Y+5)>=0){
    vgaU.draw_row(Y+5,X,X+6,0);
  }
}

void drawSnow(int X, int Y){
  int col = 2;
  if(Y>=0){
    vga.putpixel(X+1, Y, col);
    vga.putpixel(X+3, Y, col);
    vga.putpixel(X, Y, 0);
    vga.putpixel(X+2, Y, 0);
    vgaU.draw_row(Y, X+4, X+6, 0);
  }
  if((Y+1)>=0){
    vgaU.draw_row(Y+1, X, X+2, col);
    vgaU.draw_row(Y+1, X+3, X+5,col);
    vga.putpixel(X+2, Y+1, 0);
    vga.putpixel(X+5, Y+1, 0);
  }
  if((Y+2)>=0){
    vga.putpixel(X+2, Y+2, col);
     vgaU.draw_row(Y+2, X, X+2, 0);
      vgaU.draw_row(Y+2, X+3, X+6, 0);
  }
  if((Y+3)>=0){
    vgaU.draw_row(Y+3, X, X+2, col);
    vgaU.draw_row(Y+3, X+3, X+5, col);
    vga.putpixel(X+2, Y+3, 0);
    vga.putpixel(X+5, Y+3, 0);
  }
  if((Y+4)>=0){
    vga.putpixel(X+1, Y+4, col);
    vga.putpixel(X+3, Y+4, col);
    vga.putpixel(X, Y+4, 0);
    vga.putpixel(X+2, Y+4, 0);
    vgaU.draw_row(Y+4, X+4, X+6, 0);
  }
  if((Y+5)>=0){
    vgaU.draw_row(Y+5,X,X+6,0);
  }
}

void drawRect(int X,int Y){
  int col = 3;
  if(Y>=0){
    vgaU.draw_row(Y, X,  X+5,  col);
    vga.putpixel(X+5, Y, 0);
  }
  if((Y+1)>=0){
    vga.putpixel(X, Y+1, col);
    vga.putpixel(X+4, Y+1, col);
    vgaU.draw_row(Y+1, X+1,  X+4,  0);
    vga.putpixel(X+5, Y+1, 0);
  }
  if((Y+2)>=0){
    vga.putpixel(X, Y+2, col);
    vga.putpixel(X+4, Y+2, col);
    vgaU.draw_row(Y+2, X+1,  X+4,  0);
    vga.putpixel(X+5, Y+2, 0);
  }
  if((Y+3)>=0){
    vga.putpixel(X, Y+3, col);
    vga.putpixel(X+4, Y+3, col);
    vgaU.draw_row(Y+3, X+1,  X+4,  0);
    vga.putpixel(X+5, Y+3, 0);
  }
  if((Y+4)>=0){
    vgaU.draw_row(Y+4, X,   X+xCell-1, col);
    vga.putpixel(X+5, Y+4, 0);
  }
  if((Y+5)>=0){
    vgaU.draw_row(Y+5,X,X+6,0);
  }
}

void drawObject(int X, int Y, unsigned char obj){
  switch(obj){
    case 1: drawLightning(X, Y);
            break;
    case 2: drawSmile(X, Y);
            break;
    case 3: drawRect(X, Y);
            break; 
    case 4: drawRhombus(X, Y);
            break; 
    case 5: drawSnow(X, Y);
            break;
    default: ;                       
  }
}

void drawObjectMatrix(){
  for(int y=0;y<yHeight;y++){
    for(int x=0;x<xWidth;x++){
      drawObject(xCell*x,yCell*y,object[y][x]&7);//!!!!
    }
  }
}

void screenIniDisplay(){
  int delayTime=50;
  for(int y=0;y<yHeight;y++){
    drawObject(xCell*0,yCell*y,object[y][0]&7);
    drawObject(xCell*1,yCell*y,object[y][1]&7);
    drawObject(xCell*2,yCell*y,object[y][2]&7);
    vga.delay(delayTime);
  }
  for(int x=3;x<xWidth;x++){
    drawObject(xCell*x,yCell*6,object[6][x]&7);
    drawObject(xCell*x,yCell*7,object[7][x]&7);
    drawObject(xCell*x,yCell*8,object[8][x]&7);
    vga.delay(delayTime);
  }
  for(int y=5;y>=0;y--){
    drawObject(xCell*12,yCell*y,object[y][12]&7);
    drawObject(xCell*13,yCell*y,object[y][13]&7);
    drawObject(xCell*14,yCell*y,object[y][14]&7);
    vga.delay(delayTime);
  }
  for(int x=11;x>=3;x--){
    drawObject(xCell*x,yCell*0,object[0][x]&7);
    drawObject(xCell*x,yCell*1,object[1][x]&7);
    drawObject(xCell*x,yCell*2,object[2][x]&7);
    vga.delay(delayTime);
  }
  for(int y=3;y<6;y++){
    drawObject(xCell*3,yCell*y,object[y][3]&7);
    drawObject(xCell*4,yCell*y,object[y][4]&7);
    drawObject(xCell*5,yCell*y,object[y][5]&7);
    vga.delay(delayTime);
  }
  for(int x=6;x<12;x++){
    drawObject(xCell*x,yCell*3,object[3][x]&7);
    drawObject(xCell*x,yCell*4,object[4][x]&7);
    drawObject(xCell*x,yCell*5,object[5][x]&7);
    vga.delay(delayTime);
  }
}

void blinkObject(char x, char y){
  if(blinkShowFlag){
    drawObject(xCell*x,yCell*y,object[y][x]&7);
  }
  else{
    clearBlock(xCell*x,yCell*y);
  }
}

void exchangeDisplay(){
  if(x1==x2){
    char x=x1;
    char yBig, ySmall;
    yBig=max(y1,y2);
    ySmall=min(y1,y2);
    for(int s=1;s<=yCell;s++){
      //clearBlock(x*xCell,yBig*yCell);
      //clearBlock(x*xCell,ySmall*yCell);
      
      if(s<=3){
        vgaU.draw_row(ySmall*yCell+s-1,x*xCell,(x+1)*xCell,0);
      }
      
      drawObject(x*xCell,(yBig-1)*yCell+s,object[yBig][x]);
      drawObject(x*xCell,(ySmall+1)*yCell-s,object[ySmall][x]);
      vga.delay(50);
    }
  }
  else if(y1==y2){
    char y=y1;
    char xBig, xSmall;
    xBig=max(x1,x2);
    xSmall=min(x1,x2);
    for(int s=1;s<=xCell;s++){
      //clearBlock(xBig*xCell,y*yCell);
      //clearBlock(xSmall*xCell,y*yCell);
      if(s<=3){
        vgaU.draw_column(xSmall*xCell+s-1,y*yCell,(y+1)*yCell,0);
      }
      drawObject((xBig-1)*xCell+s,y*yCell,object[y][xBig]);
      drawObject((xSmall+1)*xCell-s,y*yCell,object[y][xSmall]);
      vga.delay(50);
    }
  }
}

void checkPartialFunc(){
  if(x1==x2){
    char x=x1;
    char yBig,ySmall;
    yBig=(y2>y1?y2:y1);
    ySmall=(y2>y1?y1:y2);
    if(ySmall>=2){
        if((object[ySmall][x]&7)==(object[ySmall-1][x]&7)&&(object[ySmall][x]&7)==(object[ySmall-2][x]&7)){
          checkPartialFlag=1;
          object[ySmall][x]=object[ySmall][x]&127|128; //set the head bit to 1
          object[ySmall-1][x]=object[ySmall-1][x]&127|128;
          object[ySmall-2][x]=object[ySmall-2][x]&127|128;
        }
    }
    if(yBig<=yHeight-3)
      if((object[yBig][x]&7)==(object[yBig+1][x]&7)&&(object[yBig][x]&7)==(object[yBig+2][x]&7)){
          checkPartialFlag=1;
          object[yBig][x]=object[yBig][x]&127|128; //set the head bit to 1
          object[yBig+1][x]=object[yBig+1][x]&127|128;
          object[yBig+2][x]=object[yBig+2][x]&127|128;
        }  
    char xStart=x, xEnd=x;
    while(xStart-1>=0&&(object[ySmall][xStart-1]&7)==(object[ySmall][x]&7)){
      xStart--;
    }
    while(xEnd+1<=xWidth-1&&(object[ySmall][xEnd+1]&7)==(object[ySmall][x]&7)){
      xEnd++;
    }
    if(xEnd-xStart>=2){
      checkPartialFlag=1;
      for(int xI=xStart;xI<=xEnd;xI++){
        object[ySmall][xI]=object[ySmall][xI]&127|128;
      }
    }
    xStart=x;
    xEnd=x;
    while(xStart-1>=0&&(object[yBig][xStart-1]&7)==(object[yBig][x]&7)){
      xStart--;
    }
    while(xEnd+1<=xWidth-1&&(object[yBig][xEnd+1]&7)==(object[yBig][x]&7)){
      xEnd++;
    }
    if(xEnd-xStart>=2){
      checkPartialFlag=1;
      for(int xI=xStart;xI<=xEnd;xI++){
        object[yBig][xI]=object[yBig][xI]&127|128;
      }
    }    
  }
  else if(y1==y2){
    char y=y1;
    char xBig,xSmall;
    xBig=(x2>x1?x2:x1);
    xSmall=(x2>x1?x1:x2);
    if(xSmall>=2){
        if((object[y][xSmall]&7)==(object[y][xSmall-1]&7)&&(object[y][xSmall]&7)==(object[y][xSmall-2]&7)){
          checkPartialFlag=1;
          object[y][xSmall]=object[y][xSmall]&127|128; //set the head bit to 1
          object[y][xSmall-1]=object[y][xSmall-1]&127|128;
          object[y][xSmall-2]=object[y][xSmall-2]&127|128;
        }
    }
    if(xBig<=xWidth-3)
      if((object[y][xBig]&7)==(object[y][xBig+1]&7)&&(object[y][xBig]&7)==(object[y][xBig+2]&7)){
          checkPartialFlag=1;
          object[y][xBig]=object[y][xBig]&127|128; //set the head bit to 1
          object[y][xBig+1]=object[y][xBig+1]&127|128;
          object[y][xBig+2]=object[y][xBig+2]&127|128;
        }  
    char yStart=y, yEnd=y;
    while(yStart-1>=0&&(object[yStart-1][xSmall]&7)==(object[y][xSmall]&7)){
      yStart--;
    }
    while(yEnd+1<=yHeight-1&&(object[yEnd+1][xSmall]&7)==(object[y][xSmall]&7)){
      yEnd++;
    }
    if(yEnd-yStart>=2){
      checkPartialFlag=1;
      for(int yI=yStart;yI<=yEnd;yI++){
        object[yI][xSmall]=object[yI][xSmall]&127|128;
      }
    }
    yStart=y;
    yEnd=y;
    while(yStart-1>=0&&(object[yStart-1][xBig]&7)==(object[y][xBig]&7)){
      yStart--;
    }
    while(yEnd+1<=yHeight-1&&(object[yEnd+1][xBig]&7)==(object[y][xBig]&7)){
      yEnd++;
    }
    if(yEnd-yStart>=2){
      checkPartialFlag=1;
      for(int yI=yStart;yI<=yEnd;yI++){
        object[yI][xBig]=object[yI][xBig]&127|128;
      }
    }    
  }
}



void crushDisplay(){
  for(int i=0;i<xCell;i++){
    for(int y=0;y<yHeight;y++){
      for(int x=0;x<xWidth;x++){
        if(object[y][x]>>7==1){
          vgaU.draw_line(xCell*x+i,yCell*y, xCell*x+i, yCell*y+i+1,0);
          vgaU.draw_line(xCell*x,yCell*y+i,xCell*x+i+1,yCell*y+i,0);
        }
      }
    }
    vga.delay(100);
    #if ENABLE_SOUND == 1
    pinMode(A0, OUTPUT);
    vga.noTone();  
    #endif
  }
}

//calculate falling step
void stepCal(){
  for(char i=0;i<xWidth;i++){
    char stepSum=0;   
    for(char j=yHeight-1;j>=0;j--){
      object[j][i]=(object[j][i]&135)+(stepSum<<3);
      if(object[j][i]>>7==1){
        score++;
        stepSum++;
      }
    }    
    maxFallStep=max(maxFallStep,stepSum);
  }
  vga.delay(100);
}

void falling(){
  for(char i=0;i<xWidth;i++){
    char fallStep;
    for(char j=yHeight-2;j>=0;j--){
      fallStep= ((object[j][i]&120)>>3);
      if(fallStep>0){
        object[j+fallStep][i]=(object[j+fallStep][i]&128)+(object[j][i]&127);
      }
    }
    if((object[0][i]>>7)==1){
      for(char k=0;k<fallStep+1;k++){
        object[k][i]=random(1,6)+((fallStep+1)<<3);
      }
    }
    else{
      for(char k=0;k<fallStep;k++){
        object[k][i]=random(1,6)+(fallStep<<3);
      }
    }
  }
}

void fallingDisplay(){
  for(char t=0;t<maxFallStep;t++){
    char s;
    if(t==0) s=1;
    else s=0;
    for(s=1;s<=yCell;s++){
      for(char y=yHeight-1;y>=0;y--){
        for(char x=0;x<xWidth;x++){
          char fallStep= ((object[y][x]&120)>>3);
          if(fallStep>0){
             if(y>0){
              char aboveSymboeFallStep= ((object[y-1][x]&120)>>3);
              if(aboveSymboeFallStep!=fallStep){
                //clearBlock(x*xCell,(y-fallStep)*yCell+s-1);
                vgaU.draw_row((y-fallStep)*yCell+s-1, x*xCell,  (x+1)*xCell,  0);
              }
             }
             drawObject(x*xCell,(y-fallStep)*yCell+s,object[y][x]&7);
             if(s==yCell){
               object[y][x]=(object[y][x]&135)+((fallStep-1)<<3);
             }
          }
        }
      }
      vga.delay(50);
    }
  }
  maxFallStep=0;
}

void clearHeader(){
  for(char i=0;i<xWidth;i++){
    for(char j=0;j<yHeight;j++){
      object[j][i]=object[j][i]&7;
    }
  }
}

void checkWholeFunc(){
  for(char i=0;i<xWidth;i++){
    char startY=0;
    char endY;
    while(startY<yHeight){
      endY=startY;
      while(endY+1<yHeight&&(object[endY+1][i]&7)==(object[startY][i]&7)){
        endY++;
      }
      if((endY-startY)>=2){
        checkWholeFlag=1;
        for(char n=startY;n<=endY;n++){
          object[n][i]=object[n][i]&127|128; //set header to be 1
        }
      }
      startY=endY+1;
    }
  }
  for(char j=0;j<yHeight;j++){
    char startX=0;
    char endX;
    while(startX<xWidth){
      endX=startX;
      while(endX+1<xWidth&&(object[j][endX+1]&7)==(object[j][startX]&7)){
        endX++;
      }
      if((endX-startX)>=2){
        checkWholeFlag=1;
        for(char m=startX;m<=endX;m++){
          object[j][m]=object[j][m]&127|128; //set header to be 1
        }
      }
      startX=endX+1;
    }
  }
}

void gameFunc(){
  #if ENABLE_SOUND == 1
  if(state!=start&&state!=ending){
    vga.noTone(); 
  }
  #endif
  // put your main code here, to run repeatedly:
  if(state==gameOpen){
    drawOpen();
    state=start;
  }
  else if(state==start){
    drawStart();
    vga.delay(20);
    processInputs();
    if(buttonOK==1){
      vga.clear(0);
      state=screenIni;
      #if ENABLE_SOUND == 1
      pinMode(A0, OUTPUT);
      vga.tone(440);
      #endif
      vga.delay(200);
    }
    buttonOKLast =buttonOK;
  }
  else if(state==screenIni){
    objectIni();
    vga.clear(0);
    //drawObjectMatrix();
    screenIniDisplay();
    drawStepLeft();
    drawScore();
    state=cursor1; 
  }
  else if(state==cursor1){
    processInputs();    
    if(button&&!buttonLast) {
      if(button1&&x1old+1<xWidth) x1=x1old+1;
      if(button2&&x1old-1>=0) x1=x1old-1;
      if(button3&&y1old+1<yHeight) y1=y1old+1;
      if(button4&&y1old-1>=0) y1=y1old-1;
      drawObject(xCell*x1old,yCell*y1old,object[y1old][x1old]&7);
      blinkCount=0;    
      x1old=x1;
      y1old=y1;
      #if ENABLE_SOUND == 1
      pinMode(A0, OUTPUT);
      vga.tone(220);
      #endif
    }
    if(buttonOK&&!buttonOKLast){
      state=cursor2;
      x2=x1;
      y2=y1;
      x2old=x2;
      y2old=y2;
      #if ENABLE_SOUND == 1
      pinMode(A0, OUTPUT);
      vga.tone(440);
      #endif
    }
    buttonLast=button;
    buttonOKLast=buttonOK;
    blinkCount++;   
    if(blinkCount==BLINKCOUNT){
      blinkCount=0;
      blinkObject(x1,y1);
      blinkShowFlag = 1- blinkShowFlag;      
    }
    vga.delay(speedDelay); 
  }
  
  else if(state==cursor2){
    processInputs();    
    if(button&&!buttonLast) {
      if(button1&&x2old+1<xWidth) x2=x2old+1;
      if(button2&&x2old-1>=0) x2=x2old-1;
      if(button3&&y2old+1<yHeight) y2=y2old+1;
      if(button4&&y2old-1>=0) y2=y2old-1;
      drawObject(xCell*x2old,yCell*y2old,object[y2old][x2old]&7);
      blinkCount=0;    
      x2old=x2;
      y2old=y2;
      #if ENABLE_SOUND == 1
      pinMode(A0, OUTPUT);
      vga.tone(220);
      #endif
    }
    if(buttonOK&&!buttonOKLast){
      state=checkNeighbor;
      #if ENABLE_SOUND == 1
      pinMode(A0, OUTPUT);
      vga.tone(440);
      #endif
    }
    buttonLast=button;
    buttonOKLast=buttonOK;
    blinkCount++;   
    if(blinkCount==BLINKCOUNT){
      blinkCount=0;
      blinkObject(x2,y2);
      blinkObject(x1,y1);
      blinkShowFlag = 1- blinkShowFlag;      
    }
    vga.delay(speedDelay); 
  }

  else if(state==checkNeighbor){
    if(x1==x2&&(y1>=y2 ? y1-y2 :y2-y1)==1){
      state=exchange;
    }
    else if(y1==y2&&(x1>=x2 ? x1-x2 :x2-x1)==1){
      state=exchange;
    }
    else{
      state=cursor1;
      drawObject(xCell*x2,yCell*y2,object[y2][x2]&7);
    }
  }

  else if(state==exchange){
    char tmp=object[y1][x1];
    object[y1][x1]=(object[y1][x1]&248)|(object[y2][x2]&7);
    object[y2][x2]=(object[y2][x2]&248)|(tmp&7);
    exchangeDisplay();
    state = checkPartial;
  }

  else if(state==checkPartial){
    checkPartialFunc();
    if(checkPartialFlag==1){
      checkPartialFlag=0;
      state=crush;
      stepLeft--;
      drawStepLeft();
    }
    else{
      state=exchangeBack;
    }
  }
  
  else if(state==exchangeBack){
    vga.delay(300);
    char tmp=object[y1][x1];
    object[y1][x1]=(object[y1][x1]&248)|(object[y2][x2]&7);
    object[y2][x2]=(object[y2][x2]&248)|(tmp&7);
    exchangeDisplay();
    state = cursor1;
  }

  else if(state==crush){
    vga.delay(400);
    #if ENABLE_SOUND == 1
    pinMode(A0, OUTPUT);
    vga.tone(660);
    #endif
    //vga.clear(0);
    //drawObjectMatrix();
    crushDisplay();
    state=fall;
    //vga.delay(300);
  }

  else if(state==fall){
    stepCal();
    falling();
    fallingDisplay();
    drawScore();
    state=checkWhole;
    clearHeader();
  }

  else if(state==checkWhole){    
    checkWholeFunc();
    if(checkWholeFlag==1){
      state=crush;    
      checkWholeFlag=0;
    }
    else{
      if(stepLeft==0){
        state=gameOver;
      }
      else{
        state=cursor1;
      }      
    }
  }

  else if(state==gameOver){
    vgaU.draw_rect(0,20,90,20,0,0);
    vga.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, str17, 20, 28, 2);
    state=endSound;
  }

  else if(state==endSound){
    if(endSoundCount==0){
      #if ENABLE_SOUND == 1
      vga.tone(330);
      #endif
      endSoundCount++;
    }
    else if(endSoundCount==1){
      #if ENABLE_SOUND == 1
      pinMode(A0, OUTPUT);
      vga.tone(550);
      #endif
      endSoundCount++;
    }
    else if(endSoundCount==2){
      #if ENABLE_SOUND == 1
      pinMode(A0, OUTPUT);
      vga.tone(770);
      #endif
      endSoundCount=0;
      counterMusic = 0;
      counterMusic2 = 0;
      state=endDelay;
    }
    vga.delay(200);
  }

  else if(state==endDelay){
    vga.delay(300);
    state=ending;
  }
  
  else if(state==ending){
    musicFunc();
    processInputs();
    if(buttonOK){
      buttonOK=0;
      state=screenIni;
      stepLeft=STEPLEFT;
      score=0;
      x1=0;
      y1=0;
      x1old=x1;
      y1old=y1;
      x2=0;
      y2=0;
      x2old=x2;
      y2old=y2;
      #if ENABLE_SOUND == 1
      pinMode(A0, OUTPUT);
      vga.tone(440);
      #endif
    }
    buttonOKLast=buttonOK;
    vga.delay(20);
  }
}

void loop() {
  gameFunc();
}
