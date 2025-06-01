/* sprite_selfie.c
 * Entirely decorative floating photograph to expose the warp point.
 */
 
#include "game/game.h"

struct sprite_selfie {
  struct sprite hdr;
  double animclock;
  int animframe;
};

#define SPRITE ((struct sprite_selfie*)sprite)

static void _selfie_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.333;
    if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
    switch (SPRITE->animframe) {
      case 0: sprite->tileid=0x1c; break;
      case 1: sprite->tileid=0x1d; break;
    }
  }
}

const struct sprite_type sprite_type_selfie={
  .name="selfie",
  .objlen=sizeof(struct sprite_selfie),
  .update=_selfie_update,
};
