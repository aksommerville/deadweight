/* sprite_endprincess.c
 * Largely decorative; only used in the throne room.
 */
 
#include "game/game.h"

struct sprite_endprincess {
  struct sprite hdr;
  uint8_t tileid0;
};

#define SPRITE ((struct sprite_endprincess*)sprite)

static int _endprincess_init(struct sprite *sprite) {
  sprite->x-=0.5;
  sprite->y-=0.5;
  SPRITE->tileid0=sprite->tileid;
  sprite->tileid=0xff;
  return 0;
}

const struct sprite_type sprite_type_endprincess={
  .name="endprincess",
  .objlen=sizeof(struct sprite_endprincess),
  .init=_endprincess_init,
};
