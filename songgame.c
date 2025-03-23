#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 
#include <time.h>  
#include "visuals.c" 

#define AUDIO_BASE  0xFF203040  // base address of the audio codec
#define SWITCHES_ADDR 0xFF200040  // base address of switches
#define TIMER_BASE  0xFF202000  // base address of the timer

#define SAMPLE_RATE 8000  // 8 kHz sample rate
#define CLOCK_FREQ  100000000  // 100 MHz clock
#define TIMER_PERIOD (CLOCK_FREQ / SAMPLE_RATE)  // Timer period in clock cycles

#define MAX_VALUE 0xFFFFFF
#define MIN_VALUE 0

// hardware registers (memory-mapped)
volatile int* audio_ptr = (int*) AUDIO_BASE;
volatile int* switch_ptr = (int*) SWITCHES_ADDR;
volatile int* timer_ptr = (int*) TIMER_BASE;

//define notes (Hz)
#define C3  130
#define D3  147
#define E3  165
#define F3  175
#define G3  196
#define A3  220
#define B3  247
#define C4  261
#define D4  294
#define E4  330
#define F4  349
#define G4  392
#define A4  440
#define B4  494
#define C5  523


//define durations (in milliseconds)
#define QUARTER 500
#define HALF 1000
#define WHOLE 2000

// struct for the audio codec registers
struct audio_t {
    volatile unsigned int control;
    volatile unsigned char rarc;
    volatile unsigned char ralc;
    volatile unsigned char wsrc;
    volatile unsigned char wslc;
    volatile unsigned int ldata;
    volatile unsigned int rdata;
};

//create a struct of songs so that we can randomly access them 
typedef struct {
    const int (*notes)[2];
    int length;
} Song;

//pointer to audio register structure
struct audio_t *const audiop = ((struct audio_t *) AUDIO_BASE);

//song name: twinkle twinkle
const int twinkle_twinkle[][2] = {
    {C4, QUARTER}, {C4, QUARTER}, {G4, QUARTER}, {G4, QUARTER},
    {A4, QUARTER}, {A4, QUARTER}, {G4, HALF},
    {F4, QUARTER}, {F4, QUARTER}, {E4, QUARTER}, {E4, QUARTER},
    {D4, QUARTER}, {D4, QUARTER}, {C4, HALF}
};
//song name: mary had a little lamb
const int mary_had_a_little_lamb[][2] = {
    {E4, QUARTER}, {D4, QUARTER}, {C4, QUARTER}, {D4, QUARTER},
    {E4, QUARTER}, {E4, QUARTER}, {E4, HALF},
    {D4, QUARTER}, {D4, QUARTER}, {D4, HALF},
    {E4, QUARTER}, {G4, QUARTER}, {G4, HALF}
};

//song name: old macDonald had a farm
const int old_macdonald[][2] = {
    {G4, QUARTER}, {G4, QUARTER}, {G4, QUARTER}, {D4, HALF},
    {E4, QUARTER}, {E4, QUARTER}, {D4, HALF},
    {B3, QUARTER}, {B3, QUARTER}, {A3, QUARTER}, {A3, QUARTER}, 
    {G3, HALF}, {D4, QUARTER}, {G3, HALF},

};

//song name: O Canada 
const int o_canada[][2] = {
    {C4, QUARTER}, {F4, QUARTER}, {F4, QUARTER}, {G4, QUARTER},
    {F4, QUARTER}, {E4, QUARTER}, {D4, HALF},
    {C4, QUARTER}, {F4, QUARTER}, {F4, QUARTER}, {G4, QUARTER},
    {F4, QUARTER}, {E4, QUARTER}, {D4, HALF},
};

// song name: ring around the rosy
const int ring_around_the_rosy[][2] = {
    {C4, QUARTER}, {D4, QUARTER}, {E4, QUARTER}, {C4, QUARTER},
    {C4, QUARTER}, {D4, QUARTER}, {E4, QUARTER}, {C4, QUARTER},
    {E4, QUARTER}, {F4, QUARTER}, {G4, HALF},
    {E4, QUARTER}, {C4, QUARTER}, {G4, HALF},

};


//function to set up the timer
void setup_timer() {
    *(timer_ptr + 2) = TIMER_PERIOD;  // the period loading
    *(timer_ptr + 1) = 0b011;         //enable timer + auto-restart
}

//function to play a note
void play_note(int frequency, int duration_ms) {
    int periodSamples = SAMPLE_RATE / frequency;
    int counter = 0;
    int high = 0;

    //calculate the number of timer cycles required
    int totalSamples = (duration_ms * SAMPLE_RATE) / 1000;

    for (int i = 0; i < totalSamples; i++) {
        if (counter >= periodSamples / 2) {
            high = !high;
            counter = 0;
        }

        //waiting for space
        while (audiop->wslc == 0){}

        if (high) {
            audiop->ldata = MAX_VALUE;
            audiop->rdata = MAX_VALUE;
        } else {
            audiop->ldata = MIN_VALUE;
            audiop->rdata = MIN_VALUE;
        }

        counter++;

        // wait for the timer flag
        //while (!(*(timer_ptr) & 0x1)) {}
        //*(timer_ptr) = 0;  //clear the flag
    }
}

//function to play a song
void play_song(const int song[][2], int length) {
    for (int i = 0; i < length; i++) {
        play_note(song[i][0], song[i][1]);
    }
}

//setting up for random selection
unsigned int seed = 1;

unsigned int custom_rand() {
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF; //random seed
    return seed;
}

void custom_srand(unsigned int new_seed) {
    seed = new_seed;
}

// main function
int main(void) {
    audiop->control = 0x1;
    custom_srand(*(timer_ptr) & 0xFFFF);

    //creating the array of songs
    Song songs[5];
    songs[0].notes = twinkle_twinkle;
    songs[0].length = sizeof(twinkle_twinkle) / sizeof(twinkle_twinkle[0]);
    
    songs[1].notes = mary_had_a_little_lamb;
    songs[1].length = sizeof(mary_had_a_little_lamb) / sizeof(mary_had_a_little_lamb[0]);
    
    songs[2].notes = old_macdonald;
    songs[2].length = sizeof(old_macdonald) / sizeof(old_macdonald[0]);
    
    songs[3].notes = o_canada;
    songs[3].length = sizeof(o_canada) / sizeof(o_canada[0]);
    
    songs[4].notes = ring_around_the_rosy;
    songs[4].length = sizeof(ring_around_the_rosy) / sizeof(ring_around_the_rosy[0]);
    
    int prevSwitchState = 0;
    int currentSwitchState;

    int switchInput;
    while (1) {
        //switchInput = *switch_ptr;

        // if (switchInput & 0x1) {
        //     play_song(twinkle_twinkle, sizeof(twinkle_twinkle) / sizeof(twinkle_twinkle[0]));
        // } else if (switchInput & 0x2) {
        //     play_song(mary_had_a_little_lamb, sizeof(mary_had_a_little_lamb) / sizeof(mary_had_a_little_lamb[0]));
        // } else if (switchInput & 0x4) {
        //     play_song(old_macdonald, sizeof(old_macdonald) / sizeof(old_macdonald[0]));
        // } else if (switchInput & 0x8) {
        //     play_song(o_canada, sizeof(o_canada) / sizeof(o_canada[0]));
        // } else if (switchInput & 0x10) {
        //     play_song(ring_around_the_rosy, sizeof(ring_around_the_rosy) / sizeof(ring_around_the_rosy[0]));
        // }

        currentSwitchState = *switch_ptr;
        
        // check if any switch has been newly pressed
        if (currentSwitchState != 0 && prevSwitchState == 0) {
            // select a random song 
            int randomSongIndex = custom_rand() % 5;
            play_song(songs[randomSongIndex].notes, songs[randomSongIndex].length);
        }
        
        // store the current switch state for the next iteration
        prevSwitchState = currentSwitchState;
    }

    return 0;
}

