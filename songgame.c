#include <stdio.h>
#include <stdint.h>

#define AUDIO_BASE  0xFF203040  // Base address of the audio codec
#define SWITCHES_ADDR 0xFF200040  // Base address of switches
#define TIMER_BASE  0xFF202000  // Base address of the timer

#define SAMPLE_RATE 8000  // 8 kHz sample rate
#define CLOCK_FREQ  100000000  // 100 MHz clock
#define TIMER_PERIOD (CLOCK_FREQ / SAMPLE_RATE)  // Timer period in clock cycles

#define MAX_VALUE 0xFFFFFF
#define MIN_VALUE 0

// Hardware registers (memory-mapped)
volatile int* audio_ptr = (int*) AUDIO_BASE;
volatile int* switch_ptr = (int*) SWITCHES_ADDR;
volatile int* timer_ptr = (int*) TIMER_BASE;

//define notes (Hz)
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

//pointer to audio register structure
struct audio_t *const audiop = ((struct audio_t *) AUDIO_BASE);

//song aata
const int twinkle_twinkle[][2] = {
    {C4, QUARTER}, {C4, QUARTER}, {G4, QUARTER}, {G4, QUARTER},
    {A4, QUARTER}, {A4, QUARTER}, {G4, HALF},
    {F4, QUARTER}, {F4, QUARTER}, {E4, QUARTER}, {E4, QUARTER},
    {D4, QUARTER}, {D4, QUARTER}, {C4, HALF}
};

const int mary_had_a_little_lamb[][2] = {
    {E4, QUARTER}, {D4, QUARTER}, {C4, QUARTER}, {D4, QUARTER},
    {E4, QUARTER}, {E4, QUARTER}, {E4, HALF},
    {D4, QUARTER}, {D4, QUARTER}, {D4, HALF},
    {E4, QUARTER}, {G4, QUARTER}, {G4, HALF}
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
    int cycles = (duration_ms * SAMPLE_RATE) / 1000;

    for (int i = 0; i < cycles; i++) {
        if (counter >= periodSamples / 2) {
            high = !high;
            counter = 0;
        }

        if (high) {
            audiop->ldata = MAX_VALUE;
            audiop->rdata = MAX_VALUE;
        } else {
            audiop->ldata = MIN_VALUE;
            audiop->rdata = MIN_VALUE;
        }

        counter++;

        // wait for the timer flag
        while (!(*(timer_ptr) & 0x1)) {}
        *(timer_ptr) = 0;  //clear the flag
    }
}

//function to play a song
void play_song(const int song[][2], int length) {
    for (int i = 0; i < length; i++) {
        play_note(song[i][0], song[i][1]);
    }
}

// main function
int main(void) {
    setup_timer();

    int switchInput;
    while (1) {
        switchInput = *switch_ptr;

        if (switchInput & 0x1) {
            play_song(twinkle_twinkle, sizeof(twinkle_twinkle) / sizeof(twinkle_twinkle[0]));
        } else if (switchInput & 0x2) {
            play_song(mary_had_a_little_lamb, sizeof(mary_had_a_little_lamb) / sizeof(mary_had_a_little_lamb[0]));
        }
    }

    return 0;
}
