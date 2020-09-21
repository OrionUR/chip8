#include "chip-8.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_FONT 80
#define PROGRAM_START 0x200
#define MAX_ROM 0xea0 - 0x200

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
    chip8* c8 = malloc(sizeof(chip8));

    memset(c8, 0, sizeof(chip8));
    c8->pc = PROGRAM_START; /* Program counter starts at 0x200 */
    c8->draw_flag = true;   /* Clear screen once */
    /* Load fontset */
    for (int i = 0; i < MAX_FONT; i++) {
        c8->memory[i] = chip8_fontset[i];
    }

    return c8;
}

bool loadGame(chip8* c8, const char* filename) {
    printf("Loading: %s\n", filename);

    /* Open file */
    FILE* pFile = fopen(filename, "rb");
    if (pFile == NULL) {
        fputs("File error", stderr);
        return false;
    }

    /* Check file size */
    fseek(pFile, 0, SEEK_END);
    size_t lSize = ftell(pFile);
    rewind(pFile);
    if (lSize > MAX_ROM) {
        fputs("Error: ROM too big for memory", stderr);
        return false;
    } else {
        printf("File size: %d\n", lSize);
    }

    /* Allocate memory to contain the whole file */
    char* buffer = malloc(lSize);
    if (buffer == NULL) {
        fputs("Memory error", stderr);
        return false;
    }

    /* Copy the file into the buffer */
    size_t result = fread(buffer, 1, lSize, pFile);
    if (result != lSize) {
        fputs("Reading error", stderr);
        return false;
    }

    /* Copy buffer to Chip8 memory */
    for (int i = 0; i < lSize; i++) {
        c8->memory[i + PROGRAM_START] = buffer[i];
    }

    /* Close file, free buffer */
    fclose(pFile);
    free(buffer);

    return true;
}

void emulateCycle(chip8* c8) {
    /* Fetch opcode */
    c8->opcode = c8->memory[c8->pc] << 8 | c8->memory[c8->pc + 1];
    /* Decode opcode and execute opcode */
    switch (c8->opcode >> 12) {
        case 0:
            Op0(c8);
            break;
        case 1:
            Op1(c8);
            break;
        case 2:
            Op2(c8);
            break;
        case 3:
            Op3(c8);
            break;
        case 4:
            Op4(c8);
            break;
        case 5:
            Op5(c8);
            break;
        case 6:
            Op6(c8);
            break;
        case 7:
            Op7(c8);
            break;
        case 8:
            Op8(c8);
            break;
        case 9:
            Op9(c8);
            break;
        case 10:
            OpA(c8);
            break;
        case 11:
            OpB(c8);
            break;
        case 12:
            OpC(c8);
            break;
        case 13:
            OpD(c8);
            break;
        case 14:
            OpE(c8);
            break;
        case 15:
            OpF(c8);
            break;
    }
    c8->pc += 2;

    /* Update timers */
    if (c8->delay_timer > 0) {
        c8->delay_timer--;
    }
    if (c8->sound_timer > 0) {
        c8->sound_timer--;
    }
}

/*************************/
/* static function below */
/*************************/

static void Op0(chip8* c8) {
    switch (c8->opcode & 0xff) {
        /* 00E0: Clears the screen. */
        case 0xe0: {
            for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
                c8->gfx[i] = 0;
            }
            c8->draw_flag = true;
        } break;

        /* 00EE: Returns from a subroutine. */
        case 0xee: {
            if (!(c8->sp)) {
                printf("sp error\n");
                abort();
            }
            c8->pc = c8->stack[--c8->sp];
        } break;

        /* 0NNN: Calls machine code routine (RCA 1802 for COSMAC VIP) at address
         * NNN. Not necessary for most ROMs. */
        default: {
            c8->pc = c8->opcode & 0xfff;
        }
    }
}

/* 1NNN: Jumps to address NNN. */
static void Op1(chip8* c8) {
    c8->pc = c8->opcode & 0xfff;
}

/* 2NNN: Calls subroutine at NNN. */
static void Op2(chip8* c8) {
    if (c8->sp >= MAX_S) {
        printf("sp error\n");
        abort();
    }
    c8->stack[c8->sp++] = c8->pc;
    c8->pc = c8->opcode & 0xfff;
}

/* 3XNN: Skips the next instruction if VX equals NN. (Usually the next
 * instruction is a jump to skip a code block) */
static void Op3(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;

    if (c8->v[vx] == c8->opcode & 0xff) {
        c8->pc += 2;
    }
}

/* 4XNN: Skips the next instruction if VX doesn't equal NN. (Usually the next
 * instruction is a jump to skip a code block) */
static void Op4(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;

    if (c8->v[vx] != c8->opcode & 0xff) {
        c8->pc += 2;
    }
}

/* 5XY0: Skips the next instruction if VX equals VY. (Usually the next
 * instruction is a jump to skip a code block) */
static void Op5(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;
    u8 vy = ((c8->opcode) >> 4) & 0xf;

    if (c8->v[vx] == c8->v[vy]) {
        c8->pc += 2;
    }
}

/* 6XNN: Sets VX to NN. */
static void Op6(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;

    c8->v[vx] = c8->opcode & 0xff;
}

/* 7XNN: Adds NN to VX. (Carry flag is not changed) */
static void Op7(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;

    c8->v[vx] += c8->opcode & 0xff;
}

static void Op8(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;
    u8 vy = ((c8->opcode) >> 4) & 0xf;

    switch (c8->opcode & 0xf) {
        /* 8XY0: Sets VX to the value of VY. */
        case 0: {
            c8->v[vx] = c8->v[vy];
        } break;

        /* 8XY1: Sets VX to VX or VY. (Bitwise OR operation) */
        case 1: {
            c8->v[vx] = c8->v[vx] | c8->v[vy];
        } break;

        /* 8XY2: Sets VX to VX and VY. (Bitwise AND operation) */
        case 2: {
            c8->v[vx] = c8->v[vx] & c8->v[vy];
        } break;

        /* 8XY3: Sets VX to VX xor VY. */
        case 3: {
            c8->v[vx] = c8->v[vx] ^ c8->v[vy];
        } break;

        /* 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0
         * when there isn't. */
        case 4: {
            c8->v[0xf] = ((u16)c8->v[vx] + (u16)c8->v[vy]) > 255 ? 1 : 0;
            c8->v[vx] += c8->v[vy];
        } break;

        /* 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow,
         * and 1 when there isn't. */
        case 5: {
            c8->v[0xf] = c8->v[vx] < c8->v[vy] ? 0 : 1;
            c8->v[vx] -= c8->v[vy];
        } break;

        /* 8XY6: Stores the least significant bit of VX in VF and then shifts VX
         * to the right by 1. */
        case 6: {
            c8->v[0xf] = c8->v[vx] & 0x1;
            c8->v[vx] >>= 1;
        } break;

        /* 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow,
         * and 1 when there isn't. */
        case 7:

            c8->v[0xf] = c8->v[vx] > c8->v[vy] ? 0 : 1;
            c8->v[vx] = c8->v[vy] - c8->v[vx];
            break;

        /* 8XYE: Stores the most significant bit of VX in VF and then shifts VX
         * to the left by 1. */
        case 0xe: {
            c8->v[0xf] = (c8->v[vx] >> 7) & 0x1;
            c8->v[vx] <<= 1;
        } break;

        default: {
            printf("invalid code path\n");
            abort();
        }
    }
}

/* 9XY0: Skips the next instruction if VX doesn't equal VY. (Usually the next
 * instruction is a jump to skip a code block) */
static void Op9(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;
    u8 vy = ((c8->opcode) >> 4) & 0xf;

    if (c8->v[vx] != c8->v[vy]) {
        c8->pc += 2;
    }
}

/* ANNN: Sets I to the address NNN. */
static void OpA(chip8* c8) {
    c8->i = c8->opcode & 0xfff;
}

/* BNNN: Jumps to the address NNN plus V0. */
static void OpB(chip8* c8) {
    c8->pc = (c8->opcode & 0xfff) + (c8->v[0x0]);
}

/* CXNN: Sets VX to the result of a bitwise and operation on a random number
 * (Typically: 0 to 255) and NN. */
static void OpC(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;
    srand((unsigned)time(0));

    c8->v[vx] = (rand() % 256) & (c8->opcode & 0xff);
}

/* DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and
 * a height of N pixels. Each row of 8 pixels is read as bit-coded starting
 * from memory location I; I value doesn’t change after the execution of this
 * instruction. As described above, VF is set to 1 if any screen pixels are
 * flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t
 * happen */
static void OpD(chip8* c8) {
    u8 vx = ((c8->opcode) >> 8) & 0xf;
    u8 vy = ((c8->opcode) >> 4) & 0xf;
    u8 height = c8->opcode & 0xf;
    u8 width;
    u8 sprite;

    c8->v[0xf] = 0;
    /* overflow protect */
    width = vx + 8 < SCREEN_WIDTH ? 8 : (SCREEN_WIDTH - vx);
    height = vy + height < SCREEN_HEIGHT ? height : (SCREEN_HEIGHT - vy);

    for (int y = 0; y < height; y++) {
        sprite = c8->memory[c8->i + y];
        for (int x = 0; x < width; x++) {
            if (sprite & (0x80 >> x)) {
                if (c8->gfx[vx + x + ((vy + y) * SCREEN_WIDTH)]) {
                    c8->v[0xf] = 1;
                }
                c8->gfx[vx + x + ((vy + y) * SCREEN_WIDTH)] ^= 1;
            }
        }
    }

    c8->draw_flag = true;
}

static void OpE(chip8* c8) {
    u8 vx = (c8->opcode >> 8) & 0xf;

    switch (c8->opcode & 0xff) {
        /* EX9E: Skips the next instruction if the key stored in VX is pressed.
         * (Usually the next instruction is a jump to skip a code block) */
        case 0x9e: {
            if (c8->key[c8->v[vx]]) {
                c8->pc += 2;
            }
        } break;

        /* EXA1: Skips the next instruction if the key stored in VX isn't
         * pressed. (Usually the next instruction is a jump to skip a code
         * block) */
        case 0xa1: {
            if (!c8->key[c8->v[vx]]) {
                c8->pc += 2;
            }
        } break;

        default:
            break;
    }
}

static void OpF(chip8* c8) {
    u8 vx = (c8->opcode >> 8) & 0xf;

    switch (c8->opcode & 0xff) {
        /* FX07: Sets VX to the value of the delay timer. */
        case 0x07: {
            c8->v[vx] = c8->delay_timer;
        } break;

        /* FX0A: A key press is awaited, and then stored in VX. (Blocking
         * Operation. All instruction halted until next key event) */
        case 0x0a: {
            bool key_press = false;

            for (int i = 0; i < MAX_K; i++) {
                if (c8->key[i] != 0) {
                    c8->v[vx] = i;
                    key_press = true;
                }
            }

            if (!key_press) {
                return;
            }

            c8->pc += 2;
        } break;

        /* FX15: Sets the delay timer to VX. */
        case 0x15: {
            c8->delay_timer = c8->v[vx];
        } break;

        /* FX18: Sets the sound timer to VX. */
        case 0x18: {
            c8->sound_timer = c8->v[vx];
        } break;

        /* FX1E: Adds VX to I. VF is not affected. */
        case 0x1e: {
            c8->i += c8->v[vx];
        } break;

        /* FX29: Sets I to the location of the sprite for the character in VX.
         * Characters 0-F (in hexadecimal) are represented by a 4x5 font. */
        case 0x29: {
            c8->i = c8->v[vx] * 5;
        } break;

        /* FX33: Stores the binary-coded decimal representation of VX, with the
         * most significant of three digits at the address in I, the middle
         * digit at I plus 1, and the least significant digit at I plus 2. (In
         * other words, take the decimal representation of VX, place the
         * hundreds digit in memory at location in I, the tens digit at location
         * I+1, and the ones digit at location I+2.) */
        case 0x33: {
            u8 temp = c8->v[vx];
            c8->memory[c8->i] = temp / 100;
            c8->memory[c8->i + 1] = (temp / 10) % 10;
            c8->memory[c8->i + 2] = temp % 10;
        } break;

        /* FX55: Stores V0 to VX (including VX) in memory starting at address I.
         * The offset from I is increased by 1 for each value written, but I
         * itself is left unmodified. */
        case 0x55: {
            for (int i = 0; i <= vx; i++) {
                c8->memory[c8->i + i] = c8->v[i];
            }
        } break;

        /* FX65: Fills V0 to VX (including VX) with values from memory starting
         * at address I. The offset from I is increased by 1 for each value
         * written, but I itself is left unmodified. */
        case 0x65: {
            for (int i = 0; i <= vx; i++) {
                c8->v[i] = c8->memory[c8->i + i];
            }
        } break;
    }
}