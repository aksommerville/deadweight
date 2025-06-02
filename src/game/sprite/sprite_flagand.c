/* sprite_flagand.c
 * Looks like a weight scale, but its main purpose is managing three flags.
 * Two inputs, and when they both go true, a third flag is set true (and latches).
 */
 
#include "game/game.h"

struct sprite_flagand {
  struct sprite hdr;
  uint8_t ka,kb,kc;
  uint8_t tileid0;
  int listenerida;
  int listeneridb;
};

#define SPRITE ((struct sprite_flagand*)sprite)

static void _flagand_change(int k,int v,void *userdata) {
  struct sprite *sprite=userdata;
  if (sprite->xform) return; // Already latched, ignore further changes.
  switch (store_get(SPRITE->ka)+store_get(SPRITE->kb)) {
    case 0: sprite->tileid=SPRITE->tileid0; break;
    case 1: sprite->tileid=SPRITE->tileid0+1; break;
    case 2: {
        sprite->tileid=SPRITE->tileid0;
        sprite->xform=EGG_XFORM_XREV;
        store_set(SPRITE->kc,1);
      } break;
  }
}

static void _flagand_del(struct sprite *sprite) {
  store_unlisten(SPRITE->listenerida);
  store_unlisten(SPRITE->listeneridb);
}

static int _flagand_init(struct sprite *sprite) {
  sprite->airborne=1; // prevents earthquakes moving us
  SPRITE->tileid0=sprite->tileid;
  SPRITE->ka=sprite->arg>>24;
  SPRITE->kb=sprite->arg>>16;
  SPRITE->kc=sprite->arg>>8;
  if (store_get(SPRITE->kc)) { // Output already on; we're ornamental.
    sprite->xform=EGG_XFORM_XREV;
  } else {
    SPRITE->listenerida=store_listen(SPRITE->ka,_flagand_change,sprite);
    SPRITE->listeneridb=store_listen(SPRITE->kb,_flagand_change,sprite);
  }
  return 0;
}

const struct sprite_type sprite_type_flagand={
  .name="flagand",
  .objlen=sizeof(struct sprite_flagand),
  .del=_flagand_del,
  .init=_flagand_init,
};
