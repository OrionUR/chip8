#include "chip-8.h"

chip8* myChip8;

int main(int argc, char **argv)
{
    // Set up render system and register input callbacks
    setupGraphics();
    setupInput();

    // Initialize the Chip8 system and load the game into the memory
    myChip8 = initialize();
    loadGame(myChip8, "pong", 4);

    // Emulation loop
    while(1)
    {
        // Emulate one cycle
        emulateCycle(myChip8);

        // If the draw flag is set, update the screen
        if(drawFlag(myChip8))
            drawGraphics(myChip8);
        
        // Store key press state (Press and Release)
        setKeys(myChip8);
    }

    return 0;
}