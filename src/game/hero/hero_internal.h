#ifndef HERO_INTERNAL_H
#define HERO_INTERNAL_H

#include "game/game.h"

struct sprite_hero {
  struct sprite hdr;
  int talking;//XXX
  int walking;
  double animclock;
  int animframe;
  int facedir;
};

#define SPRITE ((struct sprite_hero*)sprite)

void hero_item_begin(struct sprite *sprite);
void hero_item_end(struct sprite *sprite);
void hero_item_update(struct sprite *sprite,double elapsed);

#endif
