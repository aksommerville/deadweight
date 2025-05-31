#include "hero_internal.h"

/* Delete.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  SPRITE->facedir=DIR_S;
  return 0;
}

/* Launch the pause modal.
 */
 
static void hero_choose(struct sprite *sprite) {
  int x=(int)(sprite->x*NS_sys_tilesize);
  int y=(int)(sprite->y*NS_sys_tilesize);
  modal_new_pause(x,y);
  g.input_blackout|=EGG_BTN_WEST;
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  if (g.input!=g.pvinput) {
    if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) { hero_choose(sprite); return; }
    if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) hero_item_begin(sprite);
    else if (!(g.input&EGG_BTN_SOUTH)&&(g.pvinput&EGG_BTN_SOUTH)) hero_item_end(sprite);
    if ((g.input&EGG_BTN_LEFT)&&!(g.pvinput&EGG_BTN_LEFT)) SPRITE->facedir=DIR_W;
    if ((g.input&EGG_BTN_RIGHT)&&!(g.pvinput&EGG_BTN_RIGHT)) SPRITE->facedir=DIR_E;
    if ((g.input&EGG_BTN_UP)&&!(g.pvinput&EGG_BTN_UP)) SPRITE->facedir=DIR_N;
    if ((g.input&EGG_BTN_DOWN)&&!(g.pvinput&EGG_BTN_DOWN)) SPRITE->facedir=DIR_S;
  }
  if (g.input&EGG_BTN_SOUTH) hero_item_update(sprite,elapsed);
  
  //XXX very temporary
  SPRITE->walking=0;
  switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: sprite->x-=6.0*elapsed; SPRITE->walking=1; break;
    case EGG_BTN_RIGHT: sprite->x+=6.0*elapsed; SPRITE->walking=1; break;
    default: if ((SPRITE->facedir==DIR_W)||(SPRITE->facedir==DIR_E)) {
        switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
          case EGG_BTN_UP: SPRITE->facedir=DIR_N; break;
          case EGG_BTN_DOWN: SPRITE->facedir=DIR_S; break;
        }
      }
  }
  switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
    case EGG_BTN_UP: sprite->y-=6.0*elapsed; SPRITE->walking=1; break;
    case EGG_BTN_DOWN: sprite->y+=6.0*elapsed; SPRITE->walking=1; break;
    default: if ((SPRITE->facedir==DIR_N)||(SPRITE->facedir==DIR_S)) {
        switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
          case EGG_BTN_LEFT: SPRITE->facedir=DIR_W; break;
          case EGG_BTN_RIGHT: SPRITE->facedir=DIR_E; break;
        }
      } break;
  }
  if (SPRITE->walking) {
    if ((SPRITE->animclock-=elapsed)<0.0) {
      SPRITE->animclock+=0.200;
      if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
    }
  } else {
    SPRITE->animclock=0.0;
    SPRITE->animframe=0;
  }
  int talking_now=0;
  int i=g.spritec;
  int px=0,py=0;
  while (i-->0) {
    struct sprite *princess=g.spritev[i];
    if (princess->defunct||(princess->type!=&sprite_type_princess)) continue;
    double dx=princess->x-sprite->x;
    if ((dx<-1.0)||(dx>1.0)) continue;
    double dy=princess->y-sprite->y;
    if ((dy<-1.0)||(dy>1.0)) continue;
    talking_now=1;
    px=(int)(princess->x*NS_sys_tilesize);
    py=(int)(princess->y*NS_sys_tilesize);
    break;
  }
  if (talking_now!=SPRITE->talking) {
    if (SPRITE->talking=talking_now) {
      struct strings_insertion insv[]={
        {'s',.s={.v="Dot",.c=3}},
        {'i',.i=123},
        {'r',.r={.rid=1,.ix=2}},
      };
      modal_new_dialogue(px,py,1,5,insv,sizeof(insv)/sizeof(insv[0]));
    }
  }
}

/* Render.
 */
 
static void _hero_render(struct sprite *sprite,int x,int y) {
  uint8_t tileid=sprite->tileid;
  uint8_t xform=0;
  switch (SPRITE->facedir) {
    case DIR_N: tileid+=1; break;
    case DIR_W: tileid+=2; break;
    case DIR_E: tileid+=2; xform=EGG_XFORM_XREV; break;
  }
  if (SPRITE->walking) switch (SPRITE->animframe) {
    case 1: tileid+=0x10; break;
    case 3: tileid+=0x20; break;
  }
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,tileid,xform);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
  .render=_hero_render,
};
