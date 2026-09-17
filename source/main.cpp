//arm-none-eabi-objcopy -O ihex build/MICROBIT build/MICROBIT.hex
#include <vector>
#include <array>
#include "MicroBit.h"

MicroBit uBit;

bool gameOver = false;
bool newblock = true;
int blockX = 0;
int range = 4;
int blocks = 4;
int fallingSpeed = 500;
int blockY = 0;
int dy = 1;
int shape = 0;
std::array<std::array<int, 2>, 2> block;


//initialises grid
const int ROWS = 7;
const int COLS = 7;
std::vector<std::vector<int>> grid(ROWS, std::vector<int>(COLS, 0));

//initialises different blocks
std::array<std::array<std::array<int, 2>, 2>, 4> data = {{
    {{{9, 9}, {9, 0}}},
    {{{9, 9}, {0, 9}}},
    {{{9, 9}, {9, 9}}},
    {{{9, 9}, {0, 0}}}
}};

void initialiseGrid()
{
    for (int x = 0; x < COLS; ++x)
    {
        for (int y = 0; y < ROWS; ++y)
        {
            if (y == 6 || x == 0 || x == 6)
            {
                grid[y][x] = 1;
            }
        }
    }
}

void removeBlock()
{
    grid[blockY][blockX] = 0;
    grid[blockY][blockX + 1] = 0;
    grid[blockY + 1][blockX] = 0;
    grid[blockY + 1][blockX + 1] = 0;
}

void placeBlock()
{
    grid[blockY][blockX]         = block[0][0];
    grid[blockY][blockX + 1]     = block[0][1];
    grid[blockY + 1][blockX]     = block[1][0];
    grid[blockY + 1][blockX + 1] = block[1][1];
}

void displaygrid(){
    for (int x = 0; x < ROWS; ++x) {
            for (int y = 0; y < COLS; ++y) {
                if (x > 0 && y > 0 && x < 6 && y < 6){
                uBit.display.image.setPixelValue(x - 1, y - 1, grid[y][x] * 28); //255/9 = 28.3333 else 0*28.3 = 0
                }
            }
        }
}

void onButtonA(MicroBitEvent e)
{
    if (!gameOver && blockX> 1)
    {
        removeBlock();
        blockX--;
        placeBlock();
        displaygrid();
    }
}

void onButtonB(MicroBitEvent e)
{
    if (!gameOver && blockX < 4)
    {
        removeBlock();
        blockX++;
        placeBlock();
        displaygrid();
    }
}

void fallingblocks(){

    while (!gameOver){
        if (newblock){
            newblock = false;
            blockY = 0;
            blockX = microbit_random(range) + 1;
            shape = microbit_random(blocks);
            block = data[shape];
        }
        else{
            blockY += dy;
        }
        if (blockY >= 5){
                newblock = true;
                continue;
            }

        placeBlock();
        displaygrid();
        uBit.sleep(fallingSpeed);
        removeBlock();
    }
}

int main()
{
    uBit.init();
    initialiseGrid();

    uBit.messageBus.listen(
        MICROBIT_ID_BUTTON_A,
        MICROBIT_BUTTON_EVT_CLICK,
        onButtonA
    );

    uBit.messageBus.listen(
        MICROBIT_ID_BUTTON_B,
        MICROBIT_BUTTON_EVT_CLICK,
        onButtonB
    );

    create_fiber(fallingblocks);
    release_fiber();
}
