#pragma once

#include "verlet.h"

typedef struct {
    VerletBody body;
} Monke;

void init_monke(Monke*);
