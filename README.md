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
- when a block lands = 1 point for each pixel in the block (eg block with 3 pixels = 3 points)
- One cleared line = 10 points
- Two lines cleared together = 40 points

The current points multiplier also applies to these scores.

## Tetris Event

Clearing two lines at the same time creates a user-defined Tetris event.

The Tetris event runs a short animation where the two completed lines disappear from the centre outwards. The game pauses while this animation is running.

## Accelerometer

The micro:bit accelerometer controls the difficulty.

Tilting the micro:bit towards the player makes the blocks fall faster but also increases the points multiplier.

| Tilt   | Speed  | Multiplier |

| Low    | 500 ms | x1         |
| Slight | 400 ms | x2         |
| Medium | 300 ms | x3         |
| Large  | 200 ms | x4         |

## Fibers and Events

The program uses two fibers:

- `fallingblocks` controls the falling blocks and main game.
- `tiltControl` reads the accelerometer and changes the game speed.

Button presses are handled using micro:bit events.

A user-defined Tetris event is generated when two lines are cleared at the same time. Its event handler controls the Tetris animation.

## Serial Output

Serial messages are used for debugging and show events such as:

- New blocks
- Landed blocks
- Cleared lines
- Tetris
- Falling speed changes
- Final score

## Build

Build the project using:

    python build.py

Then copy the generated `MICROBIT.hex` file onto the micro:bit.