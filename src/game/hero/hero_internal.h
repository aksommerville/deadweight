#ifndef HERO_INTERNAL_H
#define HERO_INTERNAL_H

#include "game/game.h"

#define HERO_WALK_SPEED 6.0
#define HERO_BROOM_SPEED 12.0

struct sprite_hero {
  struct sprite hdr;
  int indx,indy; // dpad state, doesn't necessarily mean we're moving.
  int facedx,facedy; // Visible state. Always cardinal and never zero.
  int walking;
  double animclock;
  int animframe;
  int mapid,cellx,celly;
  uint8_t using_item; // 0 if none, or NS_fld_got_broom..NS_fld_got_candy, when actively used.
  int renderclock; // Counts render frames, for high-frequency flicker.
};

#define SPRITE ((struct sprite_hero*)sprite)

void hero_item_begin(struct sprite *sprite);
void hero_item_end(struct sprite *sprite);
void hero_item_update(struct sprite *sprite,double elapsed);

void hero_render(struct sprite *sprite,int x,int y);

#endif
