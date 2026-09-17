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

std::array<std::array<std::array<int, 2>, 2>*, 4> grid_pointers;

void initialiseGrid()
{
    for (int i = 0; i < ROWS; ++i)
    {
        for (int j = 0; j < COLS; ++j)
        {
            if (i == 6 || j == 0 || j == 6)
            {
                grid[i][j] = 1;
            }
        }
    }

    for (size_t i = 0; i < data.size(); ++i)
    {
        grid_pointers[i] = &data[i];
    }
}


void onButtonA(MicroBitEvent e)
{
    if (!gameOver && blockX> 1)
    {
        blockX--;
    }
}

void onButtonB(MicroBitEvent e)
{
    if (!gameOver && blockX < 5)
    {
        blockX++;
    }
}

void fallingblocks(){

    while (!gameOver){
        if (newblock){
            newblock = false;
            blockY = 0;
            blockX = microbit_random(range) + 1;
            shape = microbit_random(blocks);
            block = (*grid_pointers[shape]);
            grid[blockY][blockX]         = block[0][0];
            grid[blockY][blockX + 1]     = block[0][1];
            grid[blockY + 1][blockX]     = block[1][0];
            grid[blockY + 1][blockX + 1] = block[1][1];
        }
        else{
            blockY += dy;
            grid[blockY][blockX]         = block[0][0];
            grid[blockY][blockX + 1]     = block[0][1];
            grid[blockY + 1][blockX]     = block[1][0];
            grid[blockY + 1][blockX + 1] = block[1][1];
        }

        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (i > 1 && j > 1 && i < 6 && j < 6){
                uBit.display.image.setPixelValue(i - 1, j - 1, grid[j][i] * 28.3); //255/9 = 28.3333 else 0*28.3 = 0
                }
            }
        }
        uBit.sleep(fallingSpeed);
        uBit.display.image.setPixelValue(blockY, blockX, 0);
        uBit.display.image.setPixelValue(blockY + 1, blockX, 0);
        uBit.display.image.setPixelValue(blockY, blockX + 1, 0);
        uBit.display.image.setPixelValue(blockY + 1, blockX + 1, 0);
        uBit.sleep(fallingSpeed);
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
