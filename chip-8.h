#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_MEM 4096
#define MAX_V 16
#define MAX_S 16
#define MAX_K 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

#define Assert(expr)                          \
    if (!(expr)) {                            \
        printf("assert error: %s \n", #expr); \
        *(volatile int*)0 = 0;                \
    }

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct chip8 {
    u16 opcode;                            // operation code
    u8 memory[MAX_MEM];                    // memory address
    u8 v[MAX_V];                           // register from v0 to vf
    u16 i;                                 // index register
    u16 pc;                                // program counter
    u8 gfx[SCREEN_WIDTH * SCREEN_HEIGHT];  // screen
    u8 delay_timer;                        // delay timer
    u8 sound_timer;                        // sound timer
    u16 stack[MAX_S];                      // stack
    u16 sp;                                // stack pointer
    u8 key[MAX_K];                         // keypad
    bool draw_flag;
} chip8;

chip8* initialize();
bool loadGame(chip8* myChip8, const char* filename);
void emulateCycle(chip8* myChip8);
void drawGraphics(chip8* myChip8);
void setKeys(chip8* myChip8);

#endif
