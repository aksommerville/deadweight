/* sprite_flamethrower.c
 * args: u8:dir(0x40,0x10,0x08,0x02) u8:length u16:fld
 */

#include "game/game.h"

struct sprite_flamethrower {
  struct sprite hdr;
  uint8_t dir;
  uint8_t len;
  uint16_t k;
  int listenerid;
  uint8_t tileid0;
  int burning;
  double animclock;
  int animframe;
};

#define SPRITE ((struct sprite_flamethrower*)sprite)

static void _flamethrower_cb_fld(int k,int v,void *userdata) {
  struct sprite *sprite=userdata;
  if (v) {
    SPRITE->burning=1;
    sprite->tileid=SPRITE->tileid0+1;
  } else {
    SPRITE->burning=0;
    sprite->tileid=SPRITE->tileid0;
  }
}

static void _flamethrower_del(struct sprite *sprite) {
  store_unlisten(SPRITE->listenerid);
}

static int _flamethrower_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  SPRITE->dir=sprite->arg>>24;
  SPRITE->len=sprite->arg>>16;
  SPRITE->k=sprite->arg;
  if (SPRITE->k&&SPRITE->len) {
    SPRITE->listenerid=store_listen(SPRITE->k,_flamethrower_cb_fld,sprite);
  }
  switch (SPRITE->dir) {
    case 0x40: sprite->xform=EGG_XFORM_SWAP|EGG_XFORM_XREV; break;
    case 0x10: sprite->xform=EGG_XFORM_XREV; break;
    case 0x08: break; // right=natural orientation
    case 0x02: sprite->xform=EGG_XFORM_SWAP; break;
  }
  return 0;
}

static void _flamethrower_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.125;
    if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
  }
  if (SPRITE->burning) {
    //TODO Hurt hero and princess
  }
}

static void _flamethrower_render(struct sprite *sprite,int x,int y) {

  graf_draw_tile(&g.graf,g.texid_sprites,x,y,sprite->tileid,sprite->xform);

  if (SPRITE->burning) {
    int dx=0,dy=0;
    switch (SPRITE->dir) {
      case 0x40: dy=-NS_sys_tilesize; break;
      case 0x10: dx=-NS_sys_tilesize; break;
      case 0x08: dx=NS_sys_tilesize; break;
      case 0x02: dy=NS_sys_tilesize; break;
    }
    int dstx=x+dx,dsty=y+dy;
    int i=SPRITE->len;
    for (;i-->0;dstx+=dx,dsty+=dy) {
      uint8_t tileid=SPRITE->tileid0+2;
      if (SPRITE->animframe) tileid+=2;
      if (!i) tileid++;
      graf_draw_tile(&g.graf,g.texid_sprites,dstx,dsty,tileid,sprite->xform);
    }
  }
  
  int eyedx=0,eyedy=0;
  struct sprite *target=get_preferred_monster_target();
  if (target) {
    double dx=target->x-sprite->x;
    double dy=target->y-sprite->y;
    if (dx<-1.0) eyedx=-1; else if (dx>1.0) eyedx=1;
    if (dy<-1.0) eyedy=-1; else if (dy>1.0) eyedy=1;
  }
  graf_draw_tile(&g.graf,g.texid_sprites,x+eyedx,y+eyedy,SPRITE->tileid0-1,sprite->xform); // eyes
}

const struct sprite_type sprite_type_flamethrower={
  .name="flamethrower",
  .objlen=sizeof(struct sprite_flamethrower),
  .del=_flamethrower_del,
  .init=_flamethrower_init,
  .update=_flamethrower_update,
  .render=_flamethrower_render,
};
