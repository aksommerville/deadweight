#ifndef HERO_INTERNAL_H
#define HERO_INTERNAL_H

#include "game/game.h"

#define HERO_WALK_SPEED 6.0

struct sprite_hero {
  struct sprite hdr;
  int indx,indy; // dpad state, doesn't necessarily mean we're moving.
  int facedx,facedy; // Visible state. Always cardinal and never zero.
  int walking;
  double animclock;
  int animframe;
  int mapid,cellx,celly;
};

#define SPRITE ((struct sprite_hero*)sprite)

void hero_item_begin(struct sprite *sprite);
void hero_item_end(struct sprite *sprite);
void hero_item_update(struct sprite *sprite,double elapsed);

void hero_render(struct sprite *sprite,int x,int y);

#endif
