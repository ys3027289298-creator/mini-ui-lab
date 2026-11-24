
/* text: greedy line breaking over per-character advance widths. */
#include <stdio.h>
#include <string.h>

#define MAXC 128
#define MAXL 32

typedef struct {
    float adv[MAXC];      /* advance width per char */
    int breakable[MAXC];  /* 1 if a break is allowed after this char */
    int n;
} Run;

typedef struct {
    int line_start[MAXL];
    int line_len[MAXL];
    float line_width[MAXL];
    int nlines;
    int overflow;
} Lines;

static int wrap(const Run* r, float maxw, Lines* out) {
    if (r->n <= 0 || maxw <= 0.0f) return -1;
    memset(out, 0, sizeof(*out));
    int i = 0;
    while (i < r->n) {
        if (out->nlines >= MAXL) return -2;
        int start = i, last_break = -1;
        float w = 0.0f;
        while (i < r->n) {
            if (w + r->adv[i] > maxw && i > start) break;
            w += r->adv[i];
            if (r->breakable[i]) last_break = i;
            i++;
            if (w > maxw) { out->overflow = 1; break; }
        }
        int end = i;
        if (i < r->n && last_break >= start) {
            end = last_break + 1;
            i = last_break + 1;
            w = 0.0f;
            for (int k = start; k < end; k++) w += r->adv[k];
        }
        out->line_start[out->nlines] = start;
        out->line_len[out->nlines] = end - start;
        out->line_width[out->nlines] = w;
        out->nlines++;
        if (end == start) return -3;   /* no progress */
    }
    return out->nlines;
}

static int failures = 0;
static void check(const char* name, int ok) {
    printf("%s=%d\n", name, ok ? 1 : 0);
    if (!ok) failures++;
}

static void run_uniform(Run* r, int n, float adv, int breakable) {
    memset(r, 0, sizeof(*r));
    r->n = n;
    for (int i = 0; i < n; i++) { r->adv[i] = adv; r->breakable[i] = (i == n - 1) ? 1 : breakable; }
}

int main(void) {
    Run r;
    Lines out;

    run_uniform(&r, 6, 10.0f, 0);
    r.breakable[2] = 1;
    check("wrap_ok", wrap(&r, 35.0f, &out) > 0);
    check("no_overflow", out.overflow == 0);
    check("lines_positive", out.nlines >= 2);
    float maxw = 0;
    for (int i = 0; i < out.nlines; i++) if (out.line_width[i] > maxw) maxw = out.line_width[i];
    check("within_max", maxw <= 35.0f + 1e-4f);

    /* every char accounted exactly once */
    int sum = 0;
    for (int i = 0; i < out.nlines; i++) sum += out.line_len[i];
    check("chars_conserved", sum == r.n);

    /* single char wider than container must overflow, not loop */
    run_uniform(&r, 1, 100.0f, 0);
    r.breakable[0] = 1;
    int rc = wrap(&r, 10.0f, &out);
    check("too_wide_flagged", rc == 1 && out.overflow == 1);

    check("bad_maxw", wrap(&r, 0.0f, &out) == -1);
    Run e; memset(&e, 0, sizeof(e));
    check("empty_rejected", wrap(&e, 10.0f, &out) == -1);

    printf("all_ok=%d\n", failures == 0);
    return failures == 0 ? 0 : 1;
}
