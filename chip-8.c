#include "chip-8.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static u8 chip8_fontset[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

chip8* initialize() {
    chip8* myChip8 = malloc(sizeof(chip8));

    // Program counter starts at 0x200
    myChip8->pc = 0x200;
    // Reset current opcode
    myChip8->opcode = 0;
    // Reset index register
    myChip8->i = 0;
    // Reset stack pointer
    myChip8->sp = 0;

    // Clear display
    // Clear stack
    // Clear register V0 - VF
    // Clear memory

    // Load fontset
    for (int i = 0; i < 80; i++) {
        myChip8->memory[i] = chip8_fontset[i];
    }

    // Reset timers

    return myChip8;
}

void loadGame(chip8* myChip8, char* buffer, int bufferSize) {
    for (int i = 0; i < bufferSize; i++) {
        myChip8->memory[i + 512] = buffer[i];
    }
}

void emulateCycle(chip8* myChip8) {
    // Fetch opcode
    myChip8->opcode =
        myChip8->memory[myChip8->pc] << 8 | myChip8->memory[myChip8->pc + 1];
    // Decode opcode
    // Execute opcode

    // Update timers
}

void drawFlag(chip8* myChip8);
void setKeys(chip8* myChip8);
void drawGraphics(chip8* myChip8);

/*************************/
/* static function below */
/*************************/

static void Op_0(chip8* myChip8) {
    switch (myChip8->opcode & 0xff) {
        case 0xe0: {
            clearScreen(myChip8);
        } break;

        case 0xee: {
            Assert(myChip8->sp > 0);
            myChip8->pc = myChip8->stack[--myChip8->sp];
        } break;

        // 0NNN
        default: {
            myChip8->pc = myChip8->opcode & 0xfff;
        }
    }
}

static void Op_1(chip8* myChip8) {
    myChip8->pc = myChip8->opcode & 0xfff;
}

static void Op_2(chip8* myChip8) {
    Assert(myChip8->sp < MAX_SP);
    myChip8->stack[myChip8->sp++] = myChip8->pc;
    myChip8->pc = myChip8->opcode & 0xfff;
}

static void Op_3(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;

    if (myChip8->v[vx] == myChip8->opcode & 0xff) {
        myChip8->pc += 2;
    }
}
static void Op_4(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;

    if (myChip8->v[vx] != myChip8->opcode & 0xff) {
        myChip8->pc += 2;
    }
}

static void Op_5(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;
    u8 vy = ((myChip8->opcode) >> 4) & 0xf;

    if (myChip8->v[vx] == myChip8->v[vy]) {
        myChip8->pc += 2;
    }
}

static void Op_6(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;

    myChip8->v[vx] = myChip8->opcode & 0xff;
}

static void Op_7(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;

    myChip8->v[vx] += myChip8->opcode & 0xff;
}

static void Op_8(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;
    u8 vy = ((myChip8->opcode) >> 4) & 0xf;

    switch (myChip8->opcode & 0xf) {
        // 8XY0
        case 0: {
            myChip8->v[vx] = myChip8->v[vy];
        } break;

        // 8XY1
        case 1: {
            myChip8->v[vx] = myChip8->v[vx] | myChip8->v[vy];
        } break;

        // 8XY2
        case 2: {
            myChip8->v[vx] = myChip8->v[vx] & myChip8->v[vy];
        } break;

        // 8XY3
        case 3: {
            myChip8->v[vx] = myChip8->v[vx] ^ myChip8->v[vy];
        } break;

        // 8XY4
        case 4: {
            myChip8->v[0xf] =
                ((u16)myChip8->v[vx] + (u16)myChip8->v[vy]) > 255 ? 1 : 0;
            myChip8->v[vx] += myChip8->v[vy];
        } break;

        // 8XY5
        case 5: {
            myChip8->v[0xf] = myChip8->v[vx] < myChip8->v[vy] ? 0 : 1;
            myChip8->v[vx] -= myChip8->v[vy];
        } break;

        // 8XY6
        case 6: {
            myChip8->v[0xf] = myChip8->v[vx] & 0x1;
            myChip8->v[vx] >>= 1;
        } break;

        // 8XY7
        case 7:

            myChip8->v[0xf] = myChip8->v[vx] > myChip8->v[vy] ? 0 : 1;
            myChip8->v[vx] = myChip8->v[vy] - myChip8->v[vx];
            break;

        // 8XYE
        case 0xe: {
            myChip8->v[0xf] = (myChip8->v[vx] >> 7) & 0x1;
            myChip8->v[vx] <<= 1;
        } break;

        default: {
            Assert(!"invalid code path");
        }
    }
}

static void Op_9(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;
    u8 vy = ((myChip8->opcode) >> 4) & 0xf;

    if (myChip8->v[vx] != myChip8->v[vy]) {
        myChip8->pc += 2;
    }
}
static void Op_A(chip8* myChip8) {
    myChip8->i = myChip8->opcode & 0xfff;
}

static void Op_B(chip8* myChip8) {
    myChip8->pc = (myChip8->opcode & 0xfff) + (myChip8->v[0x0]);
}

static void Op_C(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;
    srand((unsigned)time(0));

    myChip8->v[vx] = (rand() % 256) & (myChip8->opcode & 0xff);
}

static void Op_D(chip8* myChip8) {
    u8 vx = ((myChip8->opcode) >> 8) & 0xf;
    u8 vy = ((myChip8->opcode) >> 4) & 0xf;
    u8 height = myChip8->opcode & 0xf;
    u8 row_graph;

    for (int yline = 0; yline < height; yline++) {
        row_graph = myChip8->memory[myChip8->i + yline];
        for (int xline = 0; xline < 8; xline++) {
            myChip8->v[0xf] = myChip8->gfx[vx + (vy + yline) * SCREEN_WIDTH] ^
                                      (row_graph >> xline) == 1
                                  ? 1
                                  : 0;
            myChip8->gfx[vx + (vy + yline) * SCREEN_WIDTH] ^=
                row_graph >> xline;
        }
    }
}

static void Op_E(chip8* myChip8) {
    u8 vx = (myChip8->opcode >> 8) & 0xf;

    switch (myChip8->opcode & 0xff) {
        // EX9E
        case 0x9e: {
            if (myChip8->key[myChip8->v[vx]]) {
                myChip8->pc += 2;
            }
        } break;

        // EXA1
        case 0xa1: {
            if (!myChip8->key[myChip8->v[vx]]) {
                myChip8->pc += 2;
            }
        } break;

        default:
            break;
    }
}

static void Op_F(chip8* myChip8) {
    u8 vx = (myChip8->opcode >> 8) & 0xf;

    switch (myChip8->opcode & 0xff) {
        // FX07
        case 0x07: {
            myChip8->v[vx] = myChip8->delay_timer;
        } break;

        // FX0A
        case 0x0a: {
            myChip8->wait = true;
            myChip8->wait_reg = myChip8->v[vx];
        } break;

        // FX15
        case 0x15: {
            myChip8->delay_timer = myChip8->v[vx];
        } break;

        // FX18
        case 0x18: {
            myChip8->sound_timer = myChip8->v[vx];
        } break;

        // FX1E
        case 0x1e: {
            myChip8->i += myChip8->v[vx];
        } break;

        // FX29
        case 0x29: {
            myChip8->i = myChip8->v[vx] * 5;
        } break;

        // FX33
        case 0x33: {
            u8 temp = myChip8->v[vx];
            myChip8->memory[myChip8->i] = temp / 100;
            myChip8->memory[myChip8->i + 1] = (temp / 10) % 10;
            myChip8->memory[myChip8->i + 2] = temp % 10;
        } break;

        // FX55
        case 0x55: {
            for (int i = 0; i <= vx; i++) {
                myChip8->memory[myChip8->i + i] = myChip8->v[i];
            }
        } break;

        // FX65
        case 0x65: {
            for (int i = 0; i <= vx; i++) {
                myChip8->v[i] = myChip8->memory[myChip8->i + i];
            }
        } break;
    }
}