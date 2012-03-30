#include <stdlib.h>
#include <string.h>
#include "myLib.h"
#include "text.h"
#include "powerupsfx.h"
#include "specialbgm2.h"

#include "kit.h"

#define NUMOBJS 5
SOUND soundA;
SOUND soundB;
int vbCountA;
int vbCountB;

MOVOBJ objs[NUMOBJS];
int size = 5;

void initialize();
void update();
void draw();

#define STARTSCREEN 0
#define GAMESCREEN 1
#define LOSESCREEN 2
#define WINSCREEN 3
#define PAUSESCREEN 4

int state;

void setupSounds();
void playSoundA( const unsigned char* sound, int length, int frequency, int isLooping );
void playSoundB( const unsigned char* sound, int length, int frequency, int isLooping );

void setupInterrupts();
void interruptHandler();

void start();
void game();
void pause();
void win();
void lose();

int randomSeed = 0;
int vbCounts[2];

unsigned int buttons;
unsigned int oldButtons;

unsigned short scanLineCounter;
char fpsbuffer[30];

#define BLACKINDEX 0
#define REDINDEX 1
#define BLUEINDEX 2
#define GREENINDEX 3
#define WHITEINDEX 4

int main()
{
	REG_DISPCTL = MODE4 | BG2_ENABLE;

	buttons = BUTTONS;
	
	state = STARTSCREEN;

	setupInterrupts();
	setupSounds();
	
	// ADD COLORS TO YOUR PALETTE HERE ! ! !
	PALETTE[BLACKINDEX] = BLACK;
	PALETTE[REDINDEX] = RED;
	PALETTE[BLUEINDEX] = BLUE;
	PALETTE[GREENINDEX] = GREEN;
	PALETTE[WHITEINDEX] = WHITE;
        
        playSoundA(specialbgm2, SPECIALBGM2LEN, SPECIALBGM2FREQ, 1);

	while(1)
	{
		oldButtons = buttons;
		buttons = BUTTONS;
		
		fillScreen4(BLACKINDEX);


                switch(state)
		{
			case STARTSCREEN:
				start();
				break;
			case GAMESCREEN:
				game();
				if(BUTTON_PRESSED(BUTTON_A))
				{
                                    playSoundB(powerupsfx, POWERUPSFXLEN, POWERUPSFXFREQ, 0);
				}
				break;
			case PAUSESCREEN:
				pause();
				break;
			case WINSCREEN:
				win();
				break;
			case LOSESCREEN:
				lose();
				break;
		}

		waitForVblank();
		flipPage();
	}

	return 0;
}

void setupSounds()
{
	// COMPLETE THIS FUNCTION !!!
	// This function will enable sounds
	
	// uncomment and complete the following lines
	REG_SOUNDCNT_X = SND_ENABLED;

	REG_SOUNDCNT_H = SND_OUTPUT_RATIO_100 | //overall sound

            DSA_OUTPUT_RATIO_50 | //channel a
            DSA_OUTPUT_TO_BOTH |
            DSA_TIMER0 |
            DSA_FIFO_RESET |

            DSB_OUTPUT_RATIO_100 | //channel b
            DSB_OUTPUT_TO_BOTH |
            DSB_TIMER1 |
            DSB_FIFO_RESET;

	REG_SOUNDCNT_L = 0;
}

void playSoundA( const unsigned char* sound, int length, int frequency, int isLooping ) {
    dma[1].cnt = 0;
    vbCountA = 0;
    // fist compute the timer interval for the sound.  The number of cycles divided by the frequency of the sound
    int interval = 16777216/frequency;
    // then setup the DMA channel for this sound
    DMANow(1, sound, REG_FIFO_A, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);
    // then setup the timer for this sound
    REG_TM0CNT = 0;
    REG_TM0D = -interval;
    // then start the timer
    REG_TM0CNT = TIMER_ON;

    // you may have to create additional global variables for these functions to work (to stop the sounds later)

    
    soundA.data = sound;
    soundA.length = length;
    soundA.frequency = frequency;
    soundA.isPlaying = 1;
    soundA.loops = isLooping;
    
    soundA.duration = ((60*length)/frequency) - ((length/frequency)*3)-1;

}


void playSoundB( const unsigned char* sound, int length, int frequency, int isLooping ) {
    dma[2].cnt = 0;
    vbCountB = 0;
    // fist compute the timer interval for the sound.  The number of cycles divided by the frequency of the sound
    int interval = 16777216/frequency;
    // then setup the DMA channel for this sound
    DMANow(2, sound, REG_FIFO_B, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);
    // then setup the timer for this sound
    REG_TM1CNT = 0;
    REG_TM1D = -interval;
    // then start the timer
    REG_TM1CNT = TIMER_ON;

    // you may have to create additional global variables for these functions to work (to stop the sounds later)

    
    soundB.data = sound;
    soundB.length = length;
    soundB.frequency = frequency;
    soundB.isPlaying = 1;
    soundB.loops = isLooping;
    
    soundB.duration = ((60*length)/frequency) - ((length/frequency)*3)-1;
	
}

void setupInterrupts()
{
	REG_IME = 0;
	REG_INTERRUPT = (unsigned int)interruptHandler;
	// accept interrupts
	REG_IE |= INT_VBLANK;
	// turn on interrupt sending
	REG_DISPSTAT |= INT_VBLANK_ENABLE;

	REG_IME = 1;
}

void interruptHandler()
{
    REG_IME = 0; //turns interrupts off, not getting a new one during playtime

    if(REG_IF & INT_VBLANK)
    {
        int i;
        for(i =0; i<NUMOBJS; i++)
        {
            if(objs[i].AI_STATE == FLEE)
            {
                    objs[i].color = rand()%WHITEINDEX + 1;
            }
        }

        vbCountA++;
        if (vbCountA >= soundA.duration)
        {
            REG_TM0CNT = 0; //turns off dma, stops sound
            dma[1].cnt = 0;
            if (soundA.loops)
            {
                playSoundA(soundA.data, soundA.length, soundA.frequency, soundA.loops);
            }
        }

        vbCountB++;
        if (vbCountB >= soundB.duration)
        {
            REG_TM1CNT = 0;
            dma[2].cnt = 0;

            if (soundB.loops)
            {
                playSoundB(soundB.data, soundB.length, soundB.frequency, soundA.loops);
            }

        }



        // MODIFY THIS FUNCTION !!!
        // Add code to stop/repeat sounds here


        REG_IF = INT_VBLANK; //resets interrupt vBlank
    }

    REG_IME = 1; //turns back on
}

void start()
{

	drawString4(50,50, "START SCREEN", WHITEINDEX);
	drawString4(70,50, "Press START to begin", WHITEINDEX);
	randomSeed++;

	if(BUTTON_PRESSED(BUTTON_START))
	{
		state = GAMESCREEN;
		srand(randomSeed);
		initialize();
	}
}

void game()
{
	drawString4(50,50, "GAME SCREEN", BLUEINDEX);
	drawString4(70,50, "Press START to return", BLUEINDEX);

	if(BUTTON_HELD(BUTTON_L))
	{
		size--;
	}
	if(BUTTON_HELD(BUTTON_R))
	{
		size++;
	}

	update();

	draw();

	if(BUTTON_PRESSED(BUTTON_START))
	{
		state = STARTSCREEN;
	}

}

void pause()
{
	drawString4(50,50, "PAUSE SCREEN", WHITEINDEX);

}

void win()
{
	drawString4(50,50, "WIN SCREEN", GREENINDEX);

}

void lose()
{
	drawString4(50,50, "LOSE SCREEN", REDINDEX);

}

void initialize()
{
	int dels[ ] = {-2, -1, 1, 2};

	int i;
	for(i = 0; i < NUMOBJS; i++)
	{
		objs[i].size = size;

		objs[i].row = rand()%(160-objs[i].size);
		objs[i].col = rand()%(240-objs[i].size);

		objs[i].color = rand()%WHITEINDEX + 1;

		objs[i].rdel = dels[rand()%4];
		objs[i].cdel = dels[rand()%4];
	}
}

void update()
{
	int i;
	for(i=0; i<NUMOBJS; i++)
	{

		objs[i].row += objs[i].rdel;
		objs[i].col += objs[i].cdel;

		objs[i].size = size;

		if(objs[i].row<0)
		{
			objs[i].row = 0;
			objs[i].rdel = -objs[i].rdel;
		}
		if(objs[i].row>159-objs[i].size)
		{
			objs[i].row = 159-objs[i].size;
			objs[i].rdel = -objs[i].rdel;
		}
		if(objs[i].col < 0)
		{
			objs[i].col = 0;
			objs[i].cdel = -objs[i].cdel;
		}
		if(objs[i].col > 239-objs[i].size)
		{
			objs[i].col = 239-objs[i].size;
			objs[i].cdel= -objs[i].cdel;
		}
	}
}

void draw()
{
	int i;
	for(i=0; i < NUMOBJS; i++)
	{
		drawRect4(objs[i].row, objs[i].col, objs[i].size, objs[i].size, objs[i].color);
	}

	sprintf(fpsbuffer, "%d", SCANLINECOUNTER);
	drawString4(10,10, fpsbuffer, WHITEINDEX);

}

