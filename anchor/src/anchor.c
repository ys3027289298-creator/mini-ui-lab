
/* anchor: constraint solving for anchored rects with over-constraint detection. */
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAXC 8

/* flag bits: which sides are pinned */
#define L 1
#define R 2
#define T 4
#define B 8
#define W 16
#define H 32

typedef struct { float x, y, w, h; } Rect;
typedef struct { int flags; float left, right, top, bottom, width, height; } Cons;

static int solve(const Cons* c, Rect* out) {
    if (c == NULL || out == NULL) return -1;
    if (c->flags == 0) return -2;

    int have_x = 0;
    float x = 0, w = 0;

    if ((c->flags & L) && (c->flags & R) && (c->flags & W)) return -3;  /* over-constrained */
    if ((c->flags & T) && (c->flags & B) && (c->flags & H)) return -3;

    if ((c->flags & L) && (c->flags & R)) {
        w = c->right - c->left;
        x = c->left;
        have_x = 1;
        if (w < 0) return -4;
    } else if ((c->flags & L) && (c->flags & W)) {
        x = c->left; w = c->width; have_x = 1;
    } else if ((c->flags & R) && (c->flags & W)) {
        w = c->width; x = c->right - c->width; have_x = 1;
    } else if (c->flags & W) {
        w = c->width; x = 0; have_x = 1;
    }
    if (!have_x) return -5;

    int have_y = 0;
    float y = 0, h = 0;
    if ((c->flags & T) && (c->flags & B)) {
        h = c->bottom - c->top;
        y = c->top;
        have_y = 1;
        if (h < 0) return -4;
    } else if ((c->flags & T) && (c->flags & H)) {
        y = c->top; h = c->height; have_y = 1;
    } else if ((c->flags & B) && (c->flags & H)) {
        h = c->height; y = c->bottom - c->height; have_y = 1;
    } else if (c->flags & H) {
        h = c->height; y = 0; have_y = 1;
    }
    if (!have_y) return -5;

    out->x = x; out->y = y; out->w = w; out->h = h;
    return 0;
}

static int failures = 0;
static void check(const char* name, int ok) {
    printf("%s=%d\n", name, ok ? 1 : 0);
    if (!ok) failures++;
}

int main(void) {
    Cons c; Rect r;

    memset(&c, 0, sizeof(c));
    c.flags = L | T | W | H;
    c.left = 10; c.top = 20; c.width = 100; c.height = 50;
    check("basic_ok", solve(&c, &r) == 0);
    check("basic_vals", r.x == 10 && r.y == 20 && r.w == 100 && r.h == 50);

    memset(&c, 0, sizeof(c));
    c.flags = L | R | T | B;
    c.left = 10; c.right = 110; c.top = 20; c.bottom = 70;
    check("stretch_ok", solve(&c, &r) == 0);
    check("stretch_size", fabsf(r.w - 100) < 1e-4f && fabsf(r.h - 50) < 1e-4f);

    memset(&c, 0, sizeof(c));
    c.flags = L | R | W | T | H;
    check("over_constrained", solve(&c, &r) == -3);

    memset(&c, 0, sizeof(c));
    c.flags = R | W | B | H;
    c.right = 200; c.width = 50; c.bottom = 300; c.height = 40;
    check("anchor_right_bottom", solve(&c, &r) == 0);
    check("rb_values", fabsf(r.x - 150) < 1e-4f && fabsf(r.y - 260) < 1e-4f);

    memset(&c, 0, sizeof(c));
    c.flags = L | R | T | H;
    c.left = 100; c.right = 50;
    check("negative_width", solve(&c, &r) == -4);

    memset(&c, 0, sizeof(c));
    c.flags = T | H;
    check("under_constrained_x", solve(&c, &r) == -5);

    Cons z; memset(&z, 0, sizeof(z));
    check("no_flags", solve(&z, &r) == -2);

    printf("all_ok=%d\n", failures == 0);
    return failures == 0 ? 0 : 1;
}
