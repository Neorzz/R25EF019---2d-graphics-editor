# 2D Graphics Editor in C

A menu-driven 2D graphics editor that uses a **2D character array** (`40×80`) as the drawing canvas.

- Blank cells → `_` (underscore)  
- Drawn pixels → `*` (asterisk)

## Features

| Operation | Description |
|-----------|-------------|
| **Add**    | Draw a Line, Rectangle, Circle, or Triangle on the canvas |
| **Delete** | Remove a previously added shape (canvas redraws automatically) |
| **Modify** | Change the parameters of an existing shape |
| **Display**| Print the canvas with coordinate axes and border |
| **List**   | Show all active objects with their parameters |
| **Clear**  | Wipe the canvas and remove all objects |

### Drawing Algorithms

- **Line** — Bresenham's line algorithm for clean diagonal/straight lines.
- **Rectangle** — Four Bresenham lines forming the edges.
- **Circle** — Midpoint circle algorithm (8-way symmetry).
- **Triangle** — Three connected Bresenham lines between three vertices.

## Build & Run

```bash
# Compile
make

# Run
./graphics_editor
```

Or directly:

```bash
gcc -Wall -std=c99 -o graphics_editor graphics_editor.c -lm
./graphics_editor
```

## Usage

The program presents a numbered menu. Enter the number to pick an action, then follow the prompts to supply coordinates.

Coordinates are **(row, col)** — row 0 is the top of the canvas, column 0 is the left edge.

### Quick demo

```
Choice: 1          ← Add shape
  Add shape:
    3) Circle
  Choice: 3
  Centre row col: 15 40
  Radius: 10

Choice: 4          ← Display canvas
```

## Project Structure

```
.
├── graphics_editor.c   # All source code (single-file)
├── Makefile             # Build automation
└── README.md            # This file
```

## Requirements

- GCC (or any C99-compatible compiler)
- `math.h` (linked with `-lm`)
- A terminal that can display 80+ columns

## License

Academic project — REVA University, IoT Programme.
