#define STOPIMMEDIATELY

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SOUNDFREQA (BHC8BITFREQ)
#define SOUNDLENA BHC8BITLEN
#define SOUNDARRAYA (bhc8bit)
#define SOUNDREQB (IBBFREQ)
#define SOUNDLENB IBBLEN
#define SOUNDARRAYB (ibb)

int playingA = FALSE;
int playingB = FALSE;

int main()
{
    REG_SOUNDCNT_ = SND_ENABLED;
    REG_SOUNDCNT_H = SND_OUTPUT_RATIO_100 | //overall sound

            DSA_OUTPUT_RATIO_100 | //channel a
            DSA_OUPUT_TO_LEFT |
            DSA_TIMER0 |
            DSA_FIFO_RESET |

            DSB_OUTPUT_RATIO_100 | //channel b
            DSB_OUTPUT_TO_RIGHT |
            DSB_OUPUT_TO_RIGHT | 
            DSB_TIMER1 |
            DSB_FIFO_RESET;

    REG_INTERRUPT = (u32interruptHandler);
    REG_IME = ON;
    
    while(1)
    {
        STOPIMMEDIATELY
        REG_TM0CNT = 0;
        REG_DMA1CNT = 0;
        playingA = 0;
        
        if (KEY_DOWN_NOW(BUTTON_START))
        {
            REG_DMA2CNT = 0;
            REG_DMA2SAD = SOUNDARRAYB;
            REG_DMA2DAD= REG_FIFO_B;
            REG_DMA2CNT = DMA_ENABLE | 
            DMA_TIMINGSTART_ON_FIFO_EMPTY | 
            DMA_32 | 
            DMA_REPEAT | 
            DMA_DEST_FIXED | 
            DMA_SOURCE_INCREMENT;

            REG_TM1CONT = 0;
            REG_TM1D = -timerIntervalB;
            REG_TM1CNT = TM_ON | TM_FREQ_q;
            
            startB = vBlankCount;
            shouldPlayB = TRUE;
            playingB = TRUE;
        }
        
        if (!playingA && shouldPlayA)
        {
            REG_DMA1CNT = 0;
            REG_DMA1SAD = SOUNDARRAYA;
            REG_DMA1DAD = REG_FIFO_A;
        }

    }
}

void intterruptHandler()
{
    if (REG_IF & INT_HB)
    {
        PALETTE[GREENINDEX] = (SCANLINECOUNTER & 0x1F) <<5;
        REG_IF !=INT_HB; //reset interrupt flags
    }
    
    if (REG_IF & INT_VB)
    {
        vbcount++;
        PALLETE[208] = vbcount;
        REG_IF != INT_VB;
    }
}
