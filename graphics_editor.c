/*
 * 2D Graphics Editor
 * ------------------
 * A menu-driven graphics editor that uses a 2D character array as a drawing
 * canvas.  Blank cells are '_' (underscore) and drawn pixels are '*' (asterisk).
 *
 * Supported shapes : Line, Rectangle, Circle, Triangle
 * Operations       : Add, Delete, Modify, Display, Clear
 *
 * Compile: gcc -o graphics_editor graphics_editor.c -lm
 * Run    : ./graphics_editor
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ----- Canvas dimensions ----- */
#define CANVAS_ROWS 40
#define CANVAS_COLS 80

/* ----- Maximum objects we can store ----- */
#define MAX_OBJECTS 64

/* ----- Shape types ----- */
typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

/* ----- A single stored object ----- */
typedef struct {
    ShapeType type;
    /* Generic parameters – meaning depends on type:
     *   LINE      : (p[0],p[1]) -> (p[2],p[3])
     *   RECTANGLE : top-left (p[0],p[1])  width p[2]  height p[3]
     *   CIRCLE    : centre (p[0],p[1])  radius p[2]
     *   TRIANGLE  : vertices (p[0],p[1]) (p[2],p[3]) (p[4],p[5])
     */
    int p[6];
    int active;          /* 1 = visible, 0 = deleted */
} GfxObject;

/* ----- Global state ----- */
static char canvas[CANVAS_ROWS][CANVAS_COLS];
static GfxObject objects[MAX_OBJECTS];
static int object_count = 0;

/* ================================================================
 *                     CANVAS UTILITIES
 * ================================================================ */

/* Fill every cell with '_' */
void canvas_clear(void)
{
    for (int r = 0; r < CANVAS_ROWS; r++)
        for (int c = 0; c < CANVAS_COLS; c++)
            canvas[r][c] = '_';
}

/* Safely plot a single point */
static void plot(int row, int col)
{
    if (row >= 0 && row < CANVAS_ROWS && col >= 0 && col < CANVAS_COLS)
        canvas[row][col] = '*';
}

/* Print the canvas with a border and axis labels */
void canvas_display(void)
{
    printf("\n");

    /* Column numbers (tens) */
    printf("    ");
    for (int c = 0; c < CANVAS_COLS; c++)
        printf("%c", (c % 10 == 0) ? ('0' + (c / 10) % 10) : ' ');
    printf("\n");

    /* Column numbers (units) */
    printf("    ");
    for (int c = 0; c < CANVAS_COLS; c++)
        printf("%d", c % 10);
    printf("\n");

    /* Top border */
    printf("   +");
    for (int c = 0; c < CANVAS_COLS; c++) printf("-");
    printf("+\n");

    /* Rows */
    for (int r = 0; r < CANVAS_ROWS; r++) {
        printf("%2d |", r);
        for (int c = 0; c < CANVAS_COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }

    /* Bottom border */
    printf("   +");
    for (int c = 0; c < CANVAS_COLS; c++) printf("-");
    printf("+\n");
}

/* ================================================================
 *                     DRAWING PRIMITIVES
 * ================================================================ */

/* Bresenham line from (r0,c0) to (r1,c1) */
void draw_line(int r0, int c0, int r1, int c1)
{
    int dr = abs(r1 - r0), dc = abs(c1 - c0);
    int sr = (r0 < r1) ? 1 : -1;
    int sc = (c0 < c1) ? 1 : -1;
    int err = dr - dc;

    while (1) {
        plot(r0, c0);
        if (r0 == r1 && c0 == c1) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r0 += sr; }
        if (e2 <  dr) { err += dr; c0 += sc; }
    }
}

/* Axis-aligned rectangle given top-left corner, width, height */
void draw_rectangle(int top, int left, int width, int height)
{
    draw_line(top, left, top, left + width - 1);                   /* top edge    */
    draw_line(top + height - 1, left, top + height - 1, left + width - 1); /* bottom edge */
    draw_line(top, left, top + height - 1, left);                  /* left edge   */
    draw_line(top, left + width - 1, top + height - 1, left + width - 1);  /* right edge  */
}

/* Mid-point circle algorithm */
void draw_circle(int cr, int cc, int radius)
{
    int x = 0, y = radius;
    int d = 1 - radius;

    while (x <= y) {
        plot(cr + y, cc + x);
        plot(cr + y, cc - x);
        plot(cr - y, cc + x);
        plot(cr - y, cc - x);
        plot(cr + x, cc + y);
        plot(cr + x, cc - y);
        plot(cr - x, cc + y);
        plot(cr - x, cc - y);

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

/* Triangle = three lines between three vertices */
void draw_triangle(int r0, int c0, int r1, int c1, int r2, int c2)
{
    draw_line(r0, c0, r1, c1);
    draw_line(r1, c1, r2, c2);
    draw_line(r2, c2, r0, c0);
}

/* ================================================================
 *                 RENDER ALL ACTIVE OBJECTS
 * ================================================================ */

/* Clears the canvas and redraws every active object */
void redraw_all(void)
{
    canvas_clear();
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].active) continue;
        GfxObject *o = &objects[i];
        switch (o->type) {
            case SHAPE_LINE:
                draw_line(o->p[0], o->p[1], o->p[2], o->p[3]);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(o->p[0], o->p[1], o->p[2], o->p[3]);
                break;
            case SHAPE_CIRCLE:
                draw_circle(o->p[0], o->p[1], o->p[2]);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(o->p[0], o->p[1], o->p[2],
                              o->p[3], o->p[4], o->p[5]);
                break;
        }
    }
}

/* ================================================================
 *                     OBJECT LIST HELPERS
 * ================================================================ */

static const char *shape_name(ShapeType t)
{
    switch (t) {
        case SHAPE_LINE:      return "Line";
        case SHAPE_RECTANGLE: return "Rectangle";
        case SHAPE_CIRCLE:    return "Circle";
        case SHAPE_TRIANGLE:  return "Triangle";
    }
    return "Unknown";
}

void list_objects(void)
{
    int found = 0;
    printf("\n--- Object List ---\n");
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].active) continue;
        found = 1;
        GfxObject *o = &objects[i];
        printf("  [%d] %s  ", i, shape_name(o->type));
        switch (o->type) {
            case SHAPE_LINE:
                printf("(%d,%d)->(%d,%d)", o->p[0], o->p[1], o->p[2], o->p[3]);
                break;
            case SHAPE_RECTANGLE:
                printf("top-left(%d,%d) w=%d h=%d", o->p[0], o->p[1], o->p[2], o->p[3]);
                break;
            case SHAPE_CIRCLE:
                printf("centre(%d,%d) r=%d", o->p[0], o->p[1], o->p[2]);
                break;
            case SHAPE_TRIANGLE:
                printf("(%d,%d) (%d,%d) (%d,%d)",
                       o->p[0], o->p[1], o->p[2], o->p[3], o->p[4], o->p[5]);
                break;
        }
        printf("\n");
    }
    if (!found) printf("  (no objects)\n");
}

/* ================================================================
 *                     MENU ACTIONS
 * ================================================================ */

/* ---------- ADD ---------- */
void action_add(void)
{
    if (object_count >= MAX_OBJECTS) {
        printf("Object limit reached (%d).\n", MAX_OBJECTS);
        return;
    }

    int choice;
    printf("\n  Add shape:\n");
    printf("    1) Line\n");
    printf("    2) Rectangle\n");
    printf("    3) Circle\n");
    printf("    4) Triangle\n");
    printf("  Choice: ");
    scanf("%d", &choice);

    GfxObject obj;
    memset(&obj, 0, sizeof(obj));
    obj.active = 1;

    switch (choice) {
        case 1:
            obj.type = SHAPE_LINE;
            printf("  Start row col: ");  scanf("%d %d", &obj.p[0], &obj.p[1]);
            printf("  End   row col: ");  scanf("%d %d", &obj.p[2], &obj.p[3]);
            break;
        case 2:
            obj.type = SHAPE_RECTANGLE;
            printf("  Top-left row col: ");  scanf("%d %d", &obj.p[0], &obj.p[1]);
            printf("  Width Height: ");      scanf("%d %d", &obj.p[2], &obj.p[3]);
            break;
        case 3:
            obj.type = SHAPE_CIRCLE;
            printf("  Centre row col: ");  scanf("%d %d", &obj.p[0], &obj.p[1]);
            printf("  Radius: ");          scanf("%d", &obj.p[2]);
            break;
        case 4:
            obj.type = SHAPE_TRIANGLE;
            printf("  Vertex-1 row col: ");  scanf("%d %d", &obj.p[0], &obj.p[1]);
            printf("  Vertex-2 row col: ");  scanf("%d %d", &obj.p[2], &obj.p[3]);
            printf("  Vertex-3 row col: ");  scanf("%d %d", &obj.p[4], &obj.p[5]);
            break;
        default:
            printf("  Invalid shape choice.\n");
            return;
    }

    objects[object_count++] = obj;
    printf("  Object #%d (%s) added.\n", object_count - 1, shape_name(obj.type));
    redraw_all();
}

/* ---------- DELETE ---------- */
void action_delete(void)
{
    list_objects();
    int id;
    printf("  Enter object ID to delete (-1 to cancel): ");
    scanf("%d", &id);
    if (id < 0 || id >= object_count || !objects[id].active) {
        printf("  Invalid or already deleted.\n");
        return;
    }
    objects[id].active = 0;
    printf("  Object #%d deleted.\n", id);
    redraw_all();
}

/* ---------- MODIFY ---------- */
void action_modify(void)
{
    list_objects();
    int id;
    printf("  Enter object ID to modify (-1 to cancel): ");
    scanf("%d", &id);
    if (id < 0 || id >= object_count || !objects[id].active) {
        printf("  Invalid or deleted.\n");
        return;
    }

    GfxObject *o = &objects[id];
    printf("  Modifying %s #%d — enter new parameters:\n", shape_name(o->type), id);

    switch (o->type) {
        case SHAPE_LINE:
            printf("  Start row col: ");  scanf("%d %d", &o->p[0], &o->p[1]);
            printf("  End   row col: ");  scanf("%d %d", &o->p[2], &o->p[3]);
            break;
        case SHAPE_RECTANGLE:
            printf("  Top-left row col: ");  scanf("%d %d", &o->p[0], &o->p[1]);
            printf("  Width Height: ");      scanf("%d %d", &o->p[2], &o->p[3]);
            break;
        case SHAPE_CIRCLE:
            printf("  Centre row col: ");  scanf("%d %d", &o->p[0], &o->p[1]);
            printf("  Radius: ");          scanf("%d", &o->p[2]);
            break;
        case SHAPE_TRIANGLE:
            printf("  Vertex-1 row col: ");  scanf("%d %d", &o->p[0], &o->p[1]);
            printf("  Vertex-2 row col: ");  scanf("%d %d", &o->p[2], &o->p[3]);
            printf("  Vertex-3 row col: ");  scanf("%d %d", &o->p[4], &o->p[5]);
            break;
    }

    printf("  Object #%d modified.\n", id);
    redraw_all();
}

/* ---------- CLEAR ALL ---------- */
void action_clear_all(void)
{
    for (int i = 0; i < object_count; i++)
        objects[i].active = 0;
    object_count = 0;
    canvas_clear();
    printf("  Canvas cleared.\n");
}

/* ================================================================
 *                          MAIN MENU
 * ================================================================ */

int main(void)
{
    canvas_clear();

    int running = 1;
    while (running) {
        printf("\n========================================\n");
        printf("       2D GRAPHICS EDITOR  (C)\n");
        printf("   Canvas: %d rows x %d cols\n", CANVAS_ROWS, CANVAS_COLS);
        printf("========================================\n");
        printf("  1) Add shape\n");
        printf("  2) Delete shape\n");
        printf("  3) Modify shape\n");
        printf("  4) Display canvas\n");
        printf("  5) List objects\n");
        printf("  6) Clear all\n");
        printf("  0) Exit\n");
        printf("----------------------------------------\n");
        printf("  Choice: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            /* Flush bad input */
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf("  Please enter a number.\n");
            continue;
        }

        switch (choice) {
            case 1: action_add();       break;
            case 2: action_delete();    break;
            case 3: action_modify();    break;
            case 4: canvas_display();   break;
            case 5: list_objects();     break;
            case 6: action_clear_all(); break;
            case 0:
                running = 0;
                printf("  Goodbye!\n");
                break;
            default:
                printf("  Invalid choice.\n");
                break;
        }
    }

    return 0;
}
