/* sprite_endprincess.c
 * Largely decorative; only used in the throne room.
 */
 
#include "game/game.h"

struct sprite_endprincess {
  struct sprite hdr;
  uint8_t tileid0;
  double countdown;
};

#define SPRITE ((struct sprite_endprincess*)sprite)

static int _endprincess_init(struct sprite *sprite) {
  sprite->x-=0.5;
  sprite->y-=0.5;
  SPRITE->tileid0=sprite->tileid;
  if (!store_get(NS_fld_win)) {
    sprite->tileid=0xff;
  }
  return 0;
}

static void _endprincess_update(struct sprite *sprite,double elapsed) {
  // 'princess' sprite sets our (tileid) to 0x30 when her animation is complete.
  // At that point, we should begin the countdown, and then trigger the gameover modal.
  if (sprite->tileid==0xff) return;
  if (SPRITE->countdown>0.0) {
    if ((SPRITE->countdown-=elapsed)<=0.0) {
      modal_new_gameover();
    }
  } else {
    SPRITE->countdown=1.500;
  }
}

const struct sprite_type sprite_type_endprincess={
  .name="endprincess",
  .objlen=sizeof(struct sprite_endprincess),
  .init=_endprincess_init,
  .update=_endprincess_update,
};
