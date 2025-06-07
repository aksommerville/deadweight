/* sprite_king.c
 * Largely decorative; only used in the throne room.
 */
 
#include "game/game.h"

struct sprite_king {
  struct sprite hdr;
  uint8_t tileid0;
  int listenerid;
};

#define SPRITE ((struct sprite_king*)sprite)

static void king_cb_win(int k,int v,void *userdata) {
  struct sprite *sprite=userdata;
  sprite->tileid=SPRITE->tileid0+1;
}

static void _king_del(struct sprite *sprite) {
  store_unlisten(SPRITE->listenerid);
}

static int _king_init(struct sprite *sprite) {
  sprite->x-=0.5;
  sprite->y-=0.5;
  SPRITE->tileid0=sprite->tileid;
  SPRITE->listenerid=store_listen(NS_fld_win,king_cb_win,sprite);
  return 0;
}

const struct sprite_type sprite_type_king={
  .name="king",
  .objlen=sizeof(struct sprite_king),
  .del=_king_del,
  .init=_king_init,
};
