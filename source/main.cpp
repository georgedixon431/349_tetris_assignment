//arm-none-eabi-objcopy -O ihex build/MICROBIT build/MICROBIT.hex
// git add .
// git commit -m ""
// git push

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
int score = 0;
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

void clearLine(){
    for (int y = 0; y < COLS; ++y) {
        int sum = 0;
        for (int x = 0; x < ROWS; ++x){
            sum += grid[y][x];
        }
        if (sum == 47){ //9 * 5 = 45 for full row + 2 for edges
            score += 10;
            for (int j = y; j > 0; --j) {
                grid[j] = grid[j - 1];
            }
            grid[0] = {1, 0, 0, 0, 0, 0, 1};
        }
    }
}

void removeBlock()
{
    if (block[0][0] > 0)
        grid[blockY][blockX] = 0;
    if (block[0][1] > 0)
        grid[blockY][blockX + 1]     = 0;
    if (block[1][0] > 0)
        grid[blockY + 1][blockX]     = 0;
    if (block[1][1]>0)
        grid[blockY + 1][blockX + 1] = 0;
}

bool canMoveDown()
{
    for (int row = 0; row < 2; row++){
        for (int col = 0; col < 2; col++){
            if (block[row][col] > 0){
                int nextY = blockY + row + 1;
                int nextX = blockX + col;
                if (grid[nextY][nextX] > 0)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool canMoveLeft(){
    if (blockX <= 1){
        return false;
    }
    for (int col = 0; col < 2; col++){
        if (block[col][0] > 0 && grid[blockY + col][blockX - 1] > 0){
            return false;
        }
    }
    return true;
}

bool canMoveRight(){
    if (blockX >= 4){
        return false;
    }
    for (int col = 0; col < 2; col++){
        if (block[col][1] > 0 && grid[blockY + col][blockX + 2] > 0){
            return false;
        }
    }
    return true;
}
bool canRotate() {
    std::array<std::array<int, 2>, 2> rotated = block;

    int spare = rotated[0][0];
    rotated[0][0] = rotated[0][1];
    rotated[0][1] = rotated[1][1];
    rotated[1][1] = rotated[1][0];
    rotated[1][0] = spare;

    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            if (rotated[row][col] > 0 && block[row][col] == 0 && grid[blockY + row][blockX + col]) {
                return false;
            }
        }
    }
    return true;
}

void setBlock(){
    if (block[0][0] > 0)
        grid[blockY][blockX] = block[0][0];
    if (block[0][1] > 0)
        grid[blockY][blockX + 1]     = block[0][1];
    if (block[1][0] > 0)
        grid[blockY + 1][blockX]     = block[1][0];
    if (block[1][1]>0)
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

void onButtonAB(MicroBitEvent e)
{
    if (!gameOver && canRotate()){
        removeBlock();
        int spare = block[0][0];
        block[0][0] = block[1][0];
        block[1][0] = block[1][1];
        block[1][1] = block[0][1];
        block[0][1] = spare;
        setBlock();
        displaygrid();
    }
}

void onButtonA(MicroBitEvent e)
{
    if (!gameOver && canMoveLeft())
    {
        removeBlock();
        blockX--;
        setBlock();
        displaygrid();
    }
}

void onButtonB(MicroBitEvent e)
{
    if (!gameOver && canMoveRight())
    {
        removeBlock();
        blockX++;
        setBlock();
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
            if (!canMoveDown()){
                gameOver = true;
                break;
            }
        } else if(canMoveDown()){
            blockY += dy;
        } else{
                newblock = true;
                setBlock();
                clearLine();
                continue;
            }
        setBlock();
        displaygrid();
        uBit.sleep(fallingSpeed);
        removeBlock();
    }
    uBit.serial.send("Final score: ");
    uBit.serial.send(ManagedString(score));
    uBit.serial.send("\r\n");

    uBit.display.image.clear();
    uBit.display.scroll("LOSER! Score: " + ManagedString(score));
    

    uBit.reset();
}

int main()
{
    uBit.init();
    initialiseGrid();

    uBit.messageBus.listen(
        MICROBIT_ID_BUTTON_AB,
        MICROBIT_BUTTON_EVT_CLICK,
        onButtonAB
    );

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
    create_fiber(clearLine);
    release_fiber();
}
