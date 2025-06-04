/* sprite_candy.c
 */
 
#include "game/game.h"

struct sprite_candy {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_candy*)sprite)

static int _candy_init(struct sprite *sprite) {
  sprite->tileid=0x38;
  sprite->layer=127;
  return 0;
}

static int _candy_hurt(struct sprite *sprite,struct sprite *assailant) {
  // When the hero attracts a bunch of monsters together, then kills them, it's weird for the candy to stick around.
  if ((assailant->type==&sprite_type_ssflame)||(assailant->type==&sprite_type_explode)) {
    sprite->defunct=1;
    return 1;
  }
  return 0;
}

const struct sprite_type sprite_type_candy={
  .name="candy",
  .objlen=sizeof(struct sprite_candy),
  .init=_candy_init,
  .hurt=_candy_hurt,
};
