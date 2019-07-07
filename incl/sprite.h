#pragma once

#include <cstdlib>

struct Sprite {
    float x, y;
    size_t tex_id;
    float player_dist;
    bool operator < (const Sprite& s) const;
};

