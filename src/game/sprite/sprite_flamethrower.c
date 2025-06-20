/* sprite_flamethrower.c
 * args: u8:dir(0x40,0x10,0x08,0x02) u8:length u16:fld
 * (length|0x80) to reverse flag.
 */

#include "game/game.h"

#define FLAMETHROWER_RADIUS 0.500

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
  int reverse;
};

#define SPRITE ((struct sprite_flamethrower*)sprite)

static void _flamethrower_cb_fld(int k,int v,void *userdata) {
  struct sprite *sprite=userdata;
  if (v!=SPRITE->reverse) {
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
  if (SPRITE->len&0x80) {
    SPRITE->len&=0x7f;
    SPRITE->reverse=1;
  }
  if (SPRITE->k&&SPRITE->len) {
    if (store_get(SPRITE->k)==SPRITE->reverse) SPRITE->burning=0;
    else SPRITE->burning=1;
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
  if (!g.time_stopped) { // Stop animating during a stopwatch action (but continue working)
    if ((SPRITE->animclock-=elapsed)<=0.0) {
      SPRITE->animclock+=0.125;
      if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
    }
  }
  if (SPRITE->burning) {
    double l=sprite->x-FLAMETHROWER_RADIUS;
    double r=sprite->x+FLAMETHROWER_RADIUS;
    double t=sprite->y-FLAMETHROWER_RADIUS;
    double b=sprite->y+FLAMETHROWER_RADIUS;
    switch (SPRITE->dir) {
      case 0x40: t-=SPRITE->len; b-=FLAMETHROWER_RADIUS*2.0; break;
      case 0x10: l-=SPRITE->len; r-=FLAMETHROWER_RADIUS*2.0; break;
      case 0x08: r+=SPRITE->len; l+=FLAMETHROWER_RADIUS*2.0; break;
      case 0x02: b+=SPRITE->len; t+=FLAMETHROWER_RADIUS*2.0; break;
    }
    int i=g.spritec;
    while (i-->0) {
      struct sprite *victim=g.spritev[i];
      if (victim->defunct) continue;
      if (!victim->type->hurt) continue;
      if (victim->x<l) continue;
      if (victim->x>r) continue;
      if (victim->y<t) continue;
      if (victim->y>b) continue;
      victim->type->hurt(victim,sprite);
    }
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
