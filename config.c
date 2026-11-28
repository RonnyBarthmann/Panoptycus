#define _POSIX_C_SOURCE 200809L
#include "config.h"

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int confErrors = 0;

/* ================================================================== */
/* Generischer Parser                                                 */
/* ================================================================== */

void ConfError(const ConfDoc *d, int line, const char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "%s:%d: ", d ? d->path : "?", line);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    confErrors++;
}

static char *Trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    char *e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) *--e = 0;
    return s;
}

int ConfLoad(ConfDoc *d, const char *path) {
    memset(d, 0, sizeof *d);
    snprintf(d->path, sizeof d->path, "%s", path);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char line[4096];
    int ln = 0, sec = 0;
    while (fgets(line, sizeof line, f)) {
        ln++;
        char *p = line;
        char *hash = strchr(p, '#');
        if (hash) *hash = 0;
        p = Trim(p);
        if (!*p) continue;
        if (*p == '[') {
            char *end = strchr(p, ']');
            if (!end) { ConfError(d, ln, "']' fehlt"); continue; }
            *end = 0;
            if (d->nsec + 1 >= 256) { ConfError(d, ln, "zu viele Abschnitte"); continue; }
            sec = ++d->nsec;
            snprintf(d->secName[sec], sizeof d->secName[sec], "%s", Trim(p + 1));
            continue;
        }
        char *eq = strchr(p, '=');
        if (!eq) { ConfError(d, ln, "Zeile ohne '=' ignoriert: %s", p); continue; }
        *eq = 0;
        char *key = Trim(p), *val = Trim(eq + 1);
        if (d->n >= d->cap) {
            d->cap = d->cap ? d->cap * 2 : 256;
            d->e = realloc(d->e, (size_t)d->cap * sizeof *d->e);
            if (!d->e) { fclose(f); return 0; }
        }
        ConfEntry *e = &d->e[d->n++];
        e->sec = sec; e->line = ln;
        snprintf(e->key, sizeof e->key, "%s", key);
        e->val = strdup(val);
    }
    fclose(f);
    return 1;
}

void ConfFree(ConfDoc *d) {
    for (int i = 0; i < d->n; i++) free(d->e[i].val);
    free(d->e);
    memset(d, 0, sizeof *d);
}

int ConfSectionCount(const ConfDoc *d, const char *name) {
    int c = 0;
    for (int s = 1; s <= d->nsec; s++) if (!strcmp(d->secName[s], name)) c++;
    return c;
}
int ConfSection(const ConfDoc *d, const char *name, int k) {
    for (int s = 1; s <= d->nsec; s++) if (!strcmp(d->secName[s], name) && k-- == 0) return s;
    return -1;
}

int ConfNext(const ConfDoc *d, int sec, const char *key, int *iter, const char **val, int *line) {
    for (int i = *iter; i < d->n; i++)
        if (d->e[i].sec == sec && !strcmp(d->e[i].key, key)) { *iter = i + 1; *val = d->e[i].val; if (line) *line = d->e[i].line; return 1; }
    *iter = d->n;
    return 0;
}

const char *ConfStr(ConfDoc *d, int sec, const char *key, int required) {
    int it = 0; const char *v;
    if (ConfNext(d, sec, key, &it, &v, NULL)) return v;
    if (required) ConfError(d, 0, "Pflichtwert '%s' fehlt%s%s", key, sec > 0 ? " im Abschnitt " : "", sec > 0 ? d->secName[sec] : "");
    return NULL;
}

int ConfParseNums(const char *s, double *out, int max) {
    int n = 0;
    while (*s && n < max) {
        while (*s == ' ' || *s == '\t' || *s == ',') s++;
        if (!*s) break;
        char *end;
        double v = strtod(s, &end);
        if (end == s) return -1;
        out[n++] = v;
        s = end;
    }
    return n;
}

int ConfNums(ConfDoc *d, int sec, const char *key, double *out, int max, int required) {
    const char *v = ConfStr(d, sec, key, required);
    if (!v) return 0;
    int n = ConfParseNums(v, out, max);
    if (n < 0) { ConfError(d, 0, "'%s': ungültige Zahl in '%s'", key, v); return 0; }
    return n;
}

double ConfNum(ConfDoc *d, int sec, const char *key) {
    const char *v = ConfStr(d, sec, key, 1);
    if (!v) return 0;
    char *end; double x = strtod(v, &end);
    if (end == v) { ConfError(d, 0, "'%s': keine Zahl: '%s'", key, v); return 0; }
    return x;
}
double ConfNumOpt(ConfDoc *d, int sec, const char *key, double def) {
    const char *v = ConfStr(d, sec, key, 0);
    if (!v) return def;
    char *end; double x = strtod(v, &end);
    return end == v ? def : x;
}

/* ================================================================== */
/* game.conf                                                          */
/* ================================================================== */

Cfg cfg;
CfgDef cfgDefs[] = {
#define X(n, d, mi, ma, r, g, desc) { #n, &cfg.n, d, mi, ma, r, g, desc, mi, ma },
    CFG_LIST(X)
#undef X
};
int cfgCount = (int)(sizeof cfgDefs / sizeof cfgDefs[0]);

void CfgInit(void) {
    for (int i = 0; i < cfgCount; i++) { *cfgDefs[i].val = cfgDefs[i].def; cfgDefs[i].userMin = cfgDefs[i].min; cfgDefs[i].userMax = cfgDefs[i].max; }
}

double CfgClampUser(const CfgDef *d, double v) {
    if (d->ranged) { if (v < d->userMin) v = d->userMin; if (v > d->userMax) v = d->userMax; }
    return v;
}

static CfgDef *FindCfg(const char *name, int *suffix) {   /* suffix: 0 = Wert, 1 = _min, 2 = _max */
    size_t l = strlen(name);
    for (int i = 0; i < cfgCount; i++) {
        if (!strcmp(cfgDefs[i].name, name)) { *suffix = 0; return &cfgDefs[i]; }
        size_t n = strlen(cfgDefs[i].name);
        if (cfgDefs[i].ranged && l == n + 4 && !strncmp(name, cfgDefs[i].name, n)) {
            if (!strcmp(name + n, "_min")) { *suffix = 1; return &cfgDefs[i]; }
            if (!strcmp(name + n, "_max")) { *suffix = 2; return &cfgDefs[i]; }
        }
    }
    return NULL;
}

int CfgLoad(const char *path) {
    ConfDoc d;
    if (!ConfLoad(&d, path)) return 0;
    for (int i = 0; i < d.n; i++) {
        int suf;
        /* alte Schlüsselnamen (frühere Version von game.conf) */
        if (!strcmp(d.e[i].key, "renderHeight")) snprintf(d.e[i].key, sizeof d.e[i].key, "render_height");
        if (!strcmp(d.e[i].key, "renderDistance")) snprintf(d.e[i].key, sizeof d.e[i].key, "render_distance");
        CfgDef *def = FindCfg(d.e[i].key, &suf);
        if (!def) { fprintf(stderr, "%s:%d: unbekannte Einstellung '%s' ignoriert\n", path, d.e[i].line, d.e[i].key); continue; }
        char *end; double v = strtod(d.e[i].val, &end);
        if (end == d.e[i].val) { fprintf(stderr, "%s:%d: '%s' ist keine Zahl\n", path, d.e[i].line, d.e[i].val); continue; }
        if (suf == 0) *def->val = v; else if (suf == 1) def->userMin = v; else def->userMax = v;
    }
    ConfFree(&d);
    return 1;
}

int CfgSave(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "# Panoptycus - game.conf\n"
               "# Format:  name = wert     (Kommentare mit #)\n"
               "# Einstellungen, die im Spiel unter \"Einstellungen\" änderbar sind, haben einen\n"
               "# sinnvollen Bereich: name_min / name_max. Werte außerhalb des Bereichs werden\n"
               "# aus dieser Datei unverändert übernommen und erst korrigiert, wenn sie im Spiel geändert werden.\n"
               "# Weitere Dateien: strength.conf, tiers.conf, monsters.conf, maps.dat\n");
    const char *group = "";
    for (int i = 0; i < cfgCount; i++) {
        const CfgDef *d = &cfgDefs[i];
        if (strcmp(group, d->group)) { group = d->group; fprintf(f, "\n# ---- %s ----\n", group); }
        fprintf(f, "# %s\n%s = %.10g\n", d->desc, d->name, *d->val);
        if (d->ranged) fprintf(f, "%s_min = %.10g\n%s_max = %.10g\n", d->name, d->userMin, d->name, d->userMax);
    }
    fclose(f);
    return 1;
}
