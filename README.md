# COMPX349 Assignment 2 - Tetris

This project is a Tetris-style game made for the BBC micro:bit V2 using C++.

## How to Play

Blocks fall down the 5x5 LED display.

- Button A moves the block left
- Button B moves the block right
- A+B rotates the block

The game ends when there is no room for a new block at the top.

## Blocks

There are four different block shapes. A random block and starting position are chosen each time a new block is created.

Blocks cannot move through the sides, bottom, or other blocks.

## Scoring

Points are awarded when blocks land and when lines are cleared.

- One cleared line = 10 points
- Two lines cleared together = 40 points

## Accelerometer

The micro:bit accelerometer controls the difficulty.

Tilting the micro:bit towards the player makes the blocks fall faster but also increases the points multiplier.

| Tilt | Speed | Multiplier |
|---|---:|---:|
| Low | 500 ms | x1 |
| Slight | 400 ms | x2 |
| Medium | 300 ms | x3 |
| Large | 200 ms | x4 |

## Fibers and Events

The program uses two fibers:

- `fallingblocks` controls the falling blocks and main game.
- `tiltControl` reads the accelerometer and changes the game speed.

Button presses are handled using micro:bit events.

## Serial Output

Serial messages are used for debugging and show events such as new blocks, landed blocks, cleared lines, speed changes and the final score.

## Build

Build the project using:

    python build.py

Then copy the generated `MICROBIT.hex` file onto the micro:bit.