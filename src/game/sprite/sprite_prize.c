/* sprite_prize.c
 * A bomb, pepper, or candy that you can pick up.
 * Args: u8:fld u24:reserved
 */
 
#include "game/game.h"

#define PRIZE_QUANTITY 3 /* how many per pickup */
#define PRIZE_GOT_TTL 1.000

struct sprite_prize {
  struct sprite hdr;
  uint8_t fldid;
  double got; // Counts down while flashing. We are both the prize and the "you got a prize" flasher.
};

#define SPRITE ((struct sprite_prize*)sprite)

static int _prize_init(struct sprite *sprite) {
  SPRITE->fldid=sprite->arg>>24;
  switch (SPRITE->fldid) {
    case NS_fld_got_bomb: sprite->tileid=0x47; break;
    case NS_fld_got_candy: sprite->tileid=0x48; break;
    case NS_fld_got_pepper: sprite->tileid=0x49; break;
    default: return -1;
  }
  return 0;
}

static void _prize_update(struct sprite *sprite,double elapsed) {
  if (SPRITE->got>0.0) {
    if ((SPRITE->got-=elapsed)<0.0) {
      sprite->defunct=1;
    }
    if (((int)(SPRITE->got*8.0))&1) {
      sprite->tileid=0xff;
    } else {
      switch (SPRITE->fldid) {
        case NS_fld_got_bomb: sprite->tileid=0x37; break;
        case NS_fld_got_candy: sprite->tileid=0x38; break;
        case NS_fld_got_pepper: sprite->tileid=0x31; break;
      }
    }
    if (g.hero) {
      sprite->x=g.hero->x;
      sprite->y=g.hero->y-1.0;
    }
  } else if (g.hero) {
    const double radius=0.500;
    double dx=g.hero->x-sprite->x;
    double dy=g.hero->y-sprite->y;
    if ((dx>=-radius)&&(dx<=radius)&&(dy>=-radius)&&(dy<=radius)) {
      SPRITE->got=PRIZE_GOT_TTL;
      egg_play_sound(RID_sound_prize);
      int qtyid=0;
      switch (SPRITE->fldid) {
        case NS_fld_got_bomb: qtyid=NS_fld_qty_bomb; break;
        case NS_fld_got_candy: qtyid=NS_fld_qty_candy; break;
        case NS_fld_got_pepper: qtyid=NS_fld_qty_pepper; break;
      }
      int qty=store_get(qtyid);
      if ((qty+=PRIZE_QUANTITY)>99) qty=99;
      store_set(qtyid,qty);
    }
  }
}

const struct sprite_type sprite_type_prize={
  .name="prize",
  .objlen=sizeof(struct sprite_prize),
  .init=_prize_init,
  .update=_prize_update,
};
