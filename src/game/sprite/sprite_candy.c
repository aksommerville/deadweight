/* sprite_candy.c
 */
 
#include "game/game.h"

struct sprite_candy {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_candy*)sprite)

static int _candy_init(struct sprite *sprite) {
  sprite->tileid=0x38;
  return 0;
}

static void _candy_update(struct sprite *sprite,double elapsed) {
  //TODO Attract monsters. How does that work?
}

const struct sprite_type sprite_type_candy={
  .name="candy",
  .objlen=sizeof(struct sprite_candy),
  .init=_candy_init,
  .update=_candy_update,
};
