#pragma once
#include "constants.hpp"

#define P2M(x) (x) / SCALE
#define M2P(x) (x) * SCALE

#define P2M_VEC(vec) for (b2Vec2& v : vec) { \
    v *= (1 / SCALE); \
}