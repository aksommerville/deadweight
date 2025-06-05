/* sprite_king.c
 * Largely decorative; only used in the throne room.
 */
 
#include "game/game.h"

struct sprite_king {
  struct sprite hdr;
  uint8_t tileid0;
};

#define SPRITE ((struct sprite_king*)sprite)

static int _king_init(struct sprite *sprite) {
  sprite->x-=0.5;
  sprite->y-=0.5;
  SPRITE->tileid0=sprite->tileid;
  return 0;
}

const struct sprite_type sprite_type_king={
  .name="king",
  .objlen=sizeof(struct sprite_king),
  .init=_king_init,
};
