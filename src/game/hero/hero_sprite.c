#include "hero_internal.h"

/* Delete.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  sprite->phl=-0.400;
  sprite->phr=0.400;
  sprite->pht=-0.250; // extra headroom
  sprite->phb=0.500; // bottom of tiles tends to be exact
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
    case EGG_BTN_LEFT: sprite_move(sprite,-6.0*elapsed,0.0); SPRITE->walking=1; break;
    case EGG_BTN_RIGHT: sprite_move(sprite,6.0*elapsed,0.0); SPRITE->walking=1; break;
    default: if ((SPRITE->facedir==DIR_W)||(SPRITE->facedir==DIR_E)) {
        switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
          case EGG_BTN_UP: SPRITE->facedir=DIR_N; break;
          case EGG_BTN_DOWN: SPRITE->facedir=DIR_S; break;
        }
      }
  }
  switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
    case EGG_BTN_UP: sprite_move(sprite,0.0,-6.0*elapsed); SPRITE->walking=1; break;
    case EGG_BTN_DOWN: sprite_move(sprite,0.0,6.0*elapsed); SPRITE->walking=1; break;
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
