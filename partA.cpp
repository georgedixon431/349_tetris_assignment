//arm-none-eabi-objcopy -O ihex build/MICROBIT build/MICROBIT.hex

#include "MicroBit.h"

MicroBit uBit;

int playerX = 2;
int range = 5;
int fallingSpeed = 200;
int y = 0;
int dy = 1;
bool gameOver = false;
int score = 0;

void onButtonA(MicroBitEvent e)
{
    if (!gameOver && playerX > 0)
    {
        uBit.display.image.setPixelValue(playerX, 4, 0);
        playerX--;

        // Debug output
        uBit.serial.send("Button A - Player X: ");
        uBit.serial.send(ManagedString(playerX));
        uBit.serial.send("\r\n");
    }
}

void onButtonB(MicroBitEvent e)
{
    if (!gameOver && playerX < 4)
    {
        uBit.display.image.setPixelValue(playerX, 4, 0);
        playerX++;

        // Debug output
        uBit.serial.send("Button B - Player X: ");
        uBit.serial.send(ManagedString(playerX));
        uBit.serial.send("\r\n");
    }
}

void displayPlayer()
{
    while (!gameOver)
    {
        uBit.display.image.setPixelValue(playerX, 4, 255);
        uBit.sleep(100);
    }
}

void pixelDescent()
{
    int x = microbit_random(range);

    uBit.serial.send("Game started\r\n");

    while (!gameOver)
    {
        // Check whether direction needs changing
        if (y == 4)
        {

            if (x == playerX)
            {
                dy = -1;
                fallingSpeed = fallingSpeed * 0.9;
                score++;

                uBit.serial.send("HIT - Score: ");
                uBit.serial.send(ManagedString(score));
                uBit.serial.send("\r\n");
            }
            else
            {
                uBit.serial.send("MISS - GAME OVER\r\n");
                gameOver = true;
                break;
            }
        }

        if (y == 0)
        {
            x = microbit_random(range);
            dy = 1;

            uBit.serial.send("Pixel reached top - New X: ");
            uBit.serial.send(ManagedString(x));
            uBit.serial.send("\r\n");
        }

        // Display current position
        uBit.display.image.setPixelValue(x, y, 255);
        uBit.sleep(fallingSpeed);

        // Flash off
        uBit.display.image.setPixelValue(x, y, 0);
        uBit.sleep(fallingSpeed);

        // Move to next row
        y += dy;
    }

    uBit.serial.send("Final score: ");
    uBit.serial.send(ManagedString(score));
    uBit.serial.send("\r\n");

    uBit.display.image.clear();
    uBit.display.scroll("LOSER! Score: ");
    uBit.display.scroll(ManagedString(score));

    uBit.reset();
}

int main()
{
    uBit.init();

    uBit.messageBus.listen(
        MICROBIT_ID_BUTTON_AB, 
        MICROBIT_BUTTON_EVT_CLICK, 
        onButtonAB);

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

    create_fiber(displayPlayer);
    create_fiber(pixelDescent);

    release_fiber();
}


