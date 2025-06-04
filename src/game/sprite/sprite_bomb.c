/* sprite_bomb.c
 * The bomb's logical timing is driven by its animation clock.
 */
 
#include "game/game.h"

struct sprite_bomb {
  struct sprite hdr;
  double animclock;
  int animframe;
  int fuse; // 0..3, counts up
};

#define SPRITE ((struct sprite_bomb*)sprite)

static int _bomb_init(struct sprite *sprite) {
  sprite->solid=1;
  sprite->phl=-0.375;
  sprite->phr=0.375;
  sprite->pht=-0.375;
  sprite->phb=0.375;
  SPRITE->animframe=-1; // will advance at the first update
  SPRITE->fuse=-1;
  return 0;
}

static void bomb_explode(struct sprite *sprite) {
  sprite->defunct=1;
  egg_play_sound(RID_sound_explode);
  //TODO hurt sprites
  //TODO smoke, "babang!", etc
}

static void _bomb_update(struct sprite *sprite,double elapsed) {
  if (g.time_stopped) return;
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=3) {
      SPRITE->animframe=0;
    }
    if (SPRITE->animframe==1) { // Advance fuse when we hit the Big frame.
      if (++(SPRITE->fuse)>=4) {
        SPRITE->fuse=3;
        bomb_explode(sprite);
      }
    }
  }
}

static void _bomb_render(struct sprite *sprite,int x,int y) {
  uint8_t tileid;
  int fusey=y;
  switch (SPRITE->animframe) {
    case 1: tileid=0x18; fusey--; break;
    case 3: tileid=0x19; fusey++; break;
    default: tileid=0x17; break;
  }
  uint8_t fusetileid=0x29;
  if (SPRITE->fuse>0) fusetileid+=SPRITE->fuse;
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,tileid,0);
  graf_draw_tile(&g.graf,g.texid_sprites,x,fusey,fusetileid,0);
}

const struct sprite_type sprite_type_bomb={
  .name="bomb",
  .objlen=sizeof(struct sprite_bomb),
  .init=_bomb_init,
  .update=_bomb_update,
  .render=_bomb_render,
};
