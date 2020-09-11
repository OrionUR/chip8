#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_MEM 4096
#define MAX_SP 16
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
    u16 opcode;
    u8 memory[MAX_MEM];
    u8 v[MAX_SP];
    u16 i;
    u16 pc;
    u8 gfx[SCREEN_WIDTH * SCREEN_HEIGHT];
    u8 delay_timer;
    u8 sound_timer;
    u16 stack[MAX_SP];
    u16 sp;
    u8 key[MAX_SP];

    bool wait;
    u8 wait_reg;
} chip8;

chip8* initialize();
void loadGame(chip8* myChip8, char* buffer, int bufferSize);
void emulateCycle(chip8* myChip8);
void drawFlag(chip8* myChip8);
void setKeys(chip8* myChip8);
void drawGraphics(chip8* myChip8);

#endif
