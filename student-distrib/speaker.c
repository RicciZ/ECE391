#include "speaker.h"

//Play sound using PC speaker
static void play_sound(uint32_t nFrequence){
    uint32_t Div;
    uint8_t tmp;

    //Set the PIT to the desired frequency
    Div = 1193180 / nFrequence;
    outb(SPEAKER_INIT, SPEAKER_PORT_CMD);
    outb((uint8_t)(Div), SPEAKER_PORT_DATA);
    outb((uint8_t)(Div >> 8), SPEAKER_PORT_DATA);

    // play the sound using the PC speaker
    tmp = inb(SPEAKER_PORT_PLAY);
    if (tmp != (tmp | 3)){
        outb(tmp | 3, SPEAKER_PORT_PLAY);
    }
}

//make it shutup
static void mute(){
    uint8_t tmp = inb(SPEAKER_PORT_PLAY) & SPEAKER_MASK_MUTE;

    outb(tmp, SPEAKER_PORT_PLAY);
}


//Make a beep
void beep(uint32_t freq){
    play_sound(freq);
    timer_wait(1);
    mute();
    pit_phase(100);
}
