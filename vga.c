
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240


extern const unsigned char maryhadalittlelambscreen[]; 
extern const unsigned char twinkletwinklescreen[];
extern const unsigned char startScreen[];
extern const unsigned char ringaroundtherosyscreen[];
extern const unsigned char ocanadascreen[];
extern const unsigned char oldmacdonaldscreen[];

void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void wait_for_vsync();
void draw_start_screen();

int pixel_buffer_start;


int main(void) {
    srand(time(0));
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();
	draw_start_screen();
}

void plot_pixel(int x, int y, short int line_color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return; // Prevent writing to invalid memory
    }
    volatile short int *one_pixel_address = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *one_pixel_address = line_color;
}

void clear_screen() {
    for (int i = 0; i <= SCREEN_WIDTH; i++) {
        for (int j = 0; j <= SCREEN_HEIGHT; j++) {
            plot_pixel(i, j, 0x0000);
        }
    }
}

void wait_for_vsync() {
    volatile int * pixel_ctrl_ptr = (int *) 0xff203030;
    int status;
    *pixel_ctrl_ptr = 1;
    status = *(pixel_ctrl_ptr + 3);
    while ((status & 0x01) != 0) {
        status = *(pixel_ctrl_ptr + 3);
    }
}

void draw_start_screen() {
    int index = 0;
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 320; x++) {
            plot_pixel(x, y, startScreen[index]);
            index++;
        }
    }
}

