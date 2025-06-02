/* sprite_treasure.c
 * arg: u8=item u8=qty u8=reappear-if-empty u8=reserved
 */
 
#include "game/game.h"

struct sprite_treasure {
  struct sprite hdr;
  uint8_t kgot,kqty,qty;
};

#define SPRITE ((struct sprite_treasure*)sprite)

static int _treasure_init(struct sprite *sprite) {
  SPRITE->kgot=sprite->arg>>24;
  if (SPRITE->qty=sprite->arg>>16) {
    switch (SPRITE->kgot) {
      case NS_fld_got_candy: SPRITE->kqty=NS_fld_qty_candy; break;
      case NS_fld_got_bomb: SPRITE->kqty=NS_fld_qty_bomb; break;
      case NS_fld_got_pepper: SPRITE->kqty=NS_fld_qty_pepper; break;
    }
  }
  if (store_get(SPRITE->kgot)) {
    if ((sprite->arg&0x0000ff00)&&SPRITE->kqty&&!store_get(SPRITE->kqty)) {
      // reappear-if-empty, and it's empty, so reappear.
    } else {
      return -1;
    }
  }
  return 0;
}

static void _treasure_update(struct sprite *sprite,double elapsed) {
  if (g.hero) {
    const double radius=0.5;
    double dx=g.hero->x-sprite->x;
    double dy=g.hero->y-sprite->y;
    if ((dx>-radius)&&(dy>-radius)&&(dx<radius)&&(dy<radius)) {
      sprite->defunct=1;
      store_set(SPRITE->kgot,1);
      if (SPRITE->kqty) store_set(SPRITE->kqty,SPRITE->qty);
      store_set(NS_fld_equipped,SPRITE->kgot); // Equip it. Is that too presumptuous of us?
      egg_play_sound(RID_sound_treasure);
      struct strings_insertion insv[]={
        {'r',.r={1,8+SPRITE->kgot-NS_fld_got_broom}},
      };
      modal_new_dialogue(
        (int)(sprite->x*NS_sys_tilesize),
        (int)(sprite->y*NS_sys_tilesize),
        1,19,insv,sizeof(insv)/sizeof(insv[0])
      );
    }
  }
}

const struct sprite_type sprite_type_treasure={
  .name="treasure",
  .objlen=sizeof(struct sprite_treasure),
  .init=_treasure_init,
  .update=_treasure_update,
};
