/* sprite_splash.c
 * Decorative.
 */
 
#include "game/game.h"

struct sprite_splash {
  struct sprite hdr;
  double clock;
};

#define SPRITE ((struct sprite_splash*)sprite)

static void _splash_update(struct sprite *sprite,double elapsed) {
  sprite->decorative=1;
  SPRITE->clock+=elapsed;
       if (SPRITE->clock>=0.800) sprite->defunct=1;
  else if (SPRITE->clock>=0.600) sprite->tileid=0x8f;
  else if (SPRITE->clock>=0.400) sprite->tileid=0x8e;
  else if (SPRITE->clock>=0.200) sprite->tileid=0x8d;
  else                           sprite->tileid=0x8c;
}

const struct sprite_type sprite_type_splash={
  .name="splash",
  .objlen=sizeof(struct sprite_splash),
  .update=_splash_update,
};
