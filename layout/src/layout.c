
/* layout: single-axis flex distribution with min/max clamping. */
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAXIT 8

typedef struct { float basis; float grow; float minw; float maxw; float out; } Item;

static int distribute(Item* it, int n, float avail, float gap) {
    if (n <= 0 || avail < 0.0f || gap < 0.0f) return -1;
    int frozen[MAXIT];
    for (int i = 0; i < n; i++) {
        if (it[i].minw > it[i].maxw) return -2;
        float w = it[i].basis;
        if (w < it[i].minw) w = it[i].minw;
        if (w > it[i].maxw) w = it[i].maxw;
        it[i].out = w;
        frozen[i] = 0;
    }
    for (int pass = 0; pass < MAXIT; pass++) {
        float used = gap * (float)(n - 1);
        float totalgrow = 0.0f;
        for (int i = 0; i < n; i++) {
            used += it[i].out;
            if (!frozen[i]) totalgrow += it[i].grow;
        }
        float free_space = avail - used;
        if (free_space <= 0.0f || totalgrow <= 0.0f) break;
        int clamped = 0;
        for (int i = 0; i < n; i++) {
            if (frozen[i]) continue;
            float w = it[i].out + free_space * it[i].grow / totalgrow;
            if (w > it[i].maxw) { it[i].out = it[i].maxw; frozen[i] = 1; clamped = 1; }
        }
        if (clamped) continue;
        for (int i = 0; i < n; i++) {
            if (frozen[i]) continue;
            it[i].out += free_space * it[i].grow / totalgrow;
        }
        break;
    }
    return 0;
}

static float total(const Item* it, int n, float gap) {
    float s = gap * (float)(n - 1);
    for (int i = 0; i < n; i++) s += it[i].out;
    return s;
}

static int failures = 0;
static void check(const char* name, int ok) {
    printf("%s=%d\n", name, ok ? 1 : 0);
    if (!ok) failures++;
}

int main(void) {
    Item it[3];
    memset(it, 0, sizeof(it));
    it[0].basis = 50; it[0].minw = 0; it[0].maxw = 1000; it[0].grow = 1;
    it[1].basis = 50; it[1].minw = 0; it[1].maxw = 1000; it[1].grow = 1;
    check("dist_ok", distribute(it, 2, 200, 0) == 0);
    check("sum_fits", fabsf(total(it, 2, 0) - 200.0f) < 1e-3f);
    check("equal_split", fabsf(it[0].out - it[1].out) < 1e-3f);

    /* grow weights */
    it[0].basis = 0; it[0].grow = 1; it[0].maxw = 1000;
    it[1].basis = 0; it[1].grow = 3; it[1].maxw = 1000;
    distribute(it, 2, 200, 0);
    check("grow_ratio", fabsf(it[1].out / it[0].out - 3.0f) < 1e-2f);

    /* max clamp must stop grow */
    it[0].basis = 0; it[0].grow = 1; it[0].maxw = 20;
    it[1].basis = 0; it[1].grow = 1; it[1].maxw = 1000;
    distribute(it, 2, 200, 0);
    check("clamped_at_max", fabsf(it[0].out - 20.0f) < 1e-3f);
    check("other_takes_rest", fabsf(it[1].out - 180.0f) < 1e-3f);

    /* gap accounted */
    it[0].basis = 0; it[0].grow = 1; it[0].maxw = 1000;
    it[1].basis = 0; it[1].grow = 1; it[1].maxw = 1000;
    distribute(it, 2, 210, 10);
    check("gap_total", fabsf(total(it, 2, 10) - 210.0f) < 1e-3f);

    check("bad_n", distribute(it, 0, 100, 0) == -1);
    check("bad_avail", distribute(it, 2, -1, 0) == -1);
    Item bad[2];
    memset(bad, 0, sizeof(bad));
    bad[0].minw = 100; bad[0].maxw = 10; bad[0].basis = 50;
    bad[1].basis = 50; bad[1].maxw = 1000;
    check("min_gt_max", distribute(bad, 2, 200, 0) == -2);

    printf("all_ok=%d\n", failures == 0);
    return failures == 0 ? 0 : 1;
}
