//arm-none-eabi-objcopy -O ihex build/MICROBIT build/MICROBIT.hex
// git add .
// git commit -m ""
// git push

#include <vector>
#include <array>
#include "MicroBit.h"

MicroBit uBit;

//tetris event
const int TETRIS_ID = 1001;
const int TETRIS_EVENT = 1;
int tetrisRow1 = 0;
int tetrisRow2 = 0;
bool animation = false;

// Game constants
const int ROWS = 7;
const int COLS = 7;
const int playableWidth = 4;
const int blocks = 4;
const int brightness = 255;
const int fullRow = (brightness * 5) + 2;
const int dy = 1;
const int delay = 200;

// Game state
bool gameOver = false;
bool newblock = true;
int score = 0;
int linesCleared = 0;

// Current block
int blockX = 0;
int blockY = 0;
int shape = 0;
std::array<std::array<int, 2>, 2> block;

// Scoring
std::array<int, 4> landingPoints = {3, 3, 4, 2};
int pointsMultiplier = 1;
const int LineClearPoints = 10;
const int Tetris = 40;

// Difficulty and accelerometer
int fallingSpeed = 500;
int currentSpeed = fallingSpeed;
int theta = 0;

// 7x7 grid provides a 5x5 playable area surrounded by hidden boundaries
std::vector<std::vector<int>> grid(ROWS, std::vector<int>(COLS, 0));

// Different block shapes
std::array<std::array<std::array<int, 2>, 2>, 4> data = {{
    {{{brightness, brightness}, {brightness, 0}}},
    {{{brightness, brightness}, {0, brightness}}},
    {{{brightness, brightness}, {brightness, brightness}}},
    {{{brightness, brightness}, {0, 0}}}
}};
//Creates hidden side walls and floor around the 5x5 playable grid
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

//Checks for completed rows, updates the score and moves rows above down
void clearLine(){
    linesCleared = 0;
    for (int y = 0; y < COLS; ++y) {
        int sum = 0;
        for (int x = 0; x < ROWS; ++x){
            sum += grid[y][x];
        }
        //A complete row contains five blocks plus the two boundary cells
        if (sum == fullRow){ 
            if (linesCleared == 0){
                tetrisRow1 = y;
            } else{
                tetrisRow2 = y;
            }
            linesCleared++;
        }
    }

    //if 2 lines cleared at once counts as tetris so is double points
    if (linesCleared == 2){
        score += Tetris * pointsMultiplier;
        uBit.serial.send("TETRIS! - score:" + ManagedString(score) + "\r\n");
        animation = true;
        MicroBitEvent(TETRIS_ID, TETRIS_EVENT);
    } else if (linesCleared == 1){
        score += LineClearPoints * pointsMultiplier;
        uBit.serial.send("One Line cleared - score:" + ManagedString(score) + "\r\n");

        //Shift all rows above the completed row down by one
        for (int j = tetrisRow1; j > 0; --j) {
            grid[j] = grid[j - 1];
        }
        //Reset the top row while keeping the side boundaries
        grid[0] = {1, 0, 0, 0, 0, 0, 1};
    }
}

//Removes only the occupied cells of the current falling block from the grid
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

//Checks whether a newly generated block can be placed without overlapping another block
bool canPlaceBlock()
{
    for (int row = 0; row < 2; row++){
        for (int col = 0; col < 2; col++){
            if (block[row][col] > 0){
                if (grid[blockY + row][blockX + col] > 0)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

//Checks whether every occupied cell of the block can move down one row
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

//Checks the left edge of the block for the wall or another block
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

//Checks the right edge of the block for the wall or another block
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

//Creates the rotated block and checks whether rotation would cause a collision
bool canRotate() {
    std::array<std::array<int, 2>, 2> rotated = block;

    //Rotate a temporary copy to determine which new cells would become occupied
    int spare = rotated[0][0];
    rotated[0][0] = rotated[1][0];
    rotated[1][0] = rotated[1][1];
    rotated[1][1] = rotated[0][1];
    rotated[0][1] = spare;

    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            //Only check cells that will become occupied after rotation
            if (rotated[row][col] > 0 && block[row][col] == 0 && grid[blockY + row][blockX + col]) {
                uBit.serial.send("Rotation blocked\r\n");
                return false;
            }
        }
    }
    return true;
}

//Places only the occupied cells of the current block into the grid
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

//Maps the internal 5x5 playable area onto the micro:bit LED display
void displaygrid(){
    for (int x = 0; x < ROWS; ++x) {
            for (int y = 0; y < COLS; ++y) {
                if (x > 0 && y > 0 && x < 6 && y < 6){
                uBit.display.image.setPixelValue(x - 1, y - 1, grid[y][x]); 
                }
            }
        }
}

//A+B event handler rotates the current block if the rotation is valid
void onButtonAB(MicroBitEvent e)
{
    if (!gameOver && canRotate()){
        removeBlock();
        //Rotate the current 2x2 block
        int spare = block[0][0];
        block[0][0] = block[1][0];
        block[1][0] = block[1][1];
        block[1][1] = block[0][1];
        block[0][1] = spare;
        setBlock();
        displaygrid();
    }
}

//Button A event handler moves the current block left if there is space
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

//Button B event handler moves the current block right if there is space
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

//creates animation when user gets a tetris (wipes 2 lines at same time)
void onTetris(MicroBitEvent e)
{
    //Remove middle block from both rows
    grid[tetrisRow1][3] = 0;
    grid[tetrisRow2][3] = 0;
    displaygrid();
    uBit.sleep(300);

    //Remove blocks either side of the middle
    grid[tetrisRow1][2] = 0;
    grid[tetrisRow1][4] = 0;
    grid[tetrisRow2][2] = 0;
    grid[tetrisRow2][4] = 0;
    displaygrid();
    uBit.sleep(300);

    //Remove the final outside blocks
    grid[tetrisRow1][1] = 0;
    grid[tetrisRow1][5] = 0;
    grid[tetrisRow2][1] = 0;
    grid[tetrisRow2][5] = 0;
    displaygrid();
    uBit.sleep(300);
    animation = false;
}

//Main gameplay fiber responsible for creating and automatically dropping blocks
void fallingblocks(){

    while (!gameOver){
        //make the tetris animation blocking
        while (animation){
            uBit.sleep(50);
        }
        //Generate a new random block at a random valid horizontal position
        if (newblock){
            newblock = false;
            blockY = 0;
            blockX = microbit_random(playableWidth) + 1;
            shape = microbit_random(blocks);
            block = data[shape];
            //Game ends if the new block overlaps blocks already at the top
            if (!canPlaceBlock()){
                gameOver = true;
                break;
            }
            uBit.serial.send("New block at x = " + ManagedString(blockX) + "\r\n");
        } else if(canMoveDown()){
            //Move the current block down one row
            blockY += dy;
        } else{
                //Block cannot move further, so leave it in place and check for completed lines
                newblock = true;
                uBit.serial.send("block Landed\r\n");
                score += landingPoints[shape] * pointsMultiplier;
                setBlock();
                clearLine();
                continue;
            }
        //Display the current position before waiting for the next falling step
        setBlock();
        displaygrid();
        uBit.sleep(fallingSpeed);
        //Temporarily remove the falling block so collision checks use the fixed grid
        removeBlock();
    }

    //Output final score over serial and show the result on the LED display
    uBit.serial.send("Final score: ");
    uBit.serial.send(ManagedString(score));
    uBit.serial.send("\r\n");

    uBit.display.image.clear();
    uBit.display.scroll("LOSER! Score: " + ManagedString(score));
    

    //Restart the micro:bit after displaying the final score
    uBit.reset();
}

void tiltControl(){
    while(!gameOver){
        currentSpeed = fallingSpeed;
        //gets the angle of the microBit and uses it to determine the falling speed of the blocks
        //faster speed means more points
        theta = uBit.accelerometer.getY();
        if (theta >= 750){
            fallingSpeed = 200;
            pointsMultiplier = 4;
        } else if (theta >= 500){
            fallingSpeed = 300;
            pointsMultiplier = 3;
        } else if (theta >= 250){
            fallingSpeed = 400;
            pointsMultiplier = 2;
        } else{
            fallingSpeed = 500;
            pointsMultiplier = 1;
        }
        //only prints the falling speed serial port if change in speed
        if (currentSpeed != fallingSpeed){
            uBit.serial.send("falling speed changed to: " + ManagedString(fallingSpeed) + "\r\n");
        }
        uBit.sleep(delay);
    }
}

int main()
{
    //Initialise the micro:bit runtime and game grid
    uBit.init();
    initialiseGrid();

    //Register A+B event for block rotation
    uBit.messageBus.listen(
        MICROBIT_ID_BUTTON_AB,
        MICROBIT_BUTTON_EVT_CLICK,
        onButtonAB
    );

    //Register button A event for left movement
    uBit.messageBus.listen(
        MICROBIT_ID_BUTTON_A,
        MICROBIT_BUTTON_EVT_CLICK,
        onButtonA
    );

    //Register button B event for right movement
    uBit.messageBus.listen(
        MICROBIT_ID_BUTTON_B,
        MICROBIT_BUTTON_EVT_CLICK,
        onButtonB
    );

    uBit.messageBus.listen(
        TETRIS_ID,
        TETRIS_EVENT,
        onTetris
    );

    //Run automatic block falling independently from button event handlers
    create_fiber(fallingblocks);
    create_fiber(tiltControl);
    release_fiber();
}