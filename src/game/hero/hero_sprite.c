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
  SPRITE->facedy=1; // Default facing south.
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

/* Walking.
 */
 
static void hero_walk_begin(struct sprite *sprite) {
  if (SPRITE->walking) return;
  //TODO Some items should inhibit walking.
  SPRITE->walking=1;
  SPRITE->animclock=0.0;
  SPRITE->animframe=0;
}

static void hero_walk_end(struct sprite *sprite) {
  if (!SPRITE->walking) return;
  SPRITE->walking=0;
}

static void hero_walk_update(struct sprite *sprite,double elapsed) {
  if (!SPRITE->walking) return;
  sprite_move(sprite,HERO_WALK_SPEED*SPRITE->indx*elapsed,HERO_WALK_SPEED*SPRITE->indy*elapsed);
}

/* Update (indx,indy), motion state, and (faced) in response to an input change.
 */
 
static void hero_update_ind(struct sprite *sprite) {
  int nindx=0,nindy=0;
  switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: nindx=-1; break;
    case EGG_BTN_RIGHT: nindx=1; break;
  }
  switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
    case EGG_BTN_UP: nindy=-1; break;
    case EGG_BTN_DOWN: nindy=1; break;
  }
  if ((nindx==SPRITE->indx)&&(nindy==SPRITE->indy)) return; // No change.
  
  /* If an axis changed to nonzero, face that way.
   * Otherwise if one existing axis is nonzero, face that way.
   * Otherwise don't turn.
   * Ties break toward horizontal, but ties should be rare.
   */
  if (nindx&&(nindx!=SPRITE->indx)) {
    SPRITE->facedx=nindx;
    SPRITE->facedy=0;
  } else if (nindy&&(nindy!=SPRITE->indy)) {
    SPRITE->facedx=0;
    SPRITE->facedy=nindy;
  } else if (nindx) {
    SPRITE->facedx=nindx;
    SPRITE->facedy=0;
  } else if (nindy) {
    SPRITE->facedx=0;
    SPRITE->facedy=nindy;
  }
  
  SPRITE->indx=nindx;
  SPRITE->indy=nindy;
  
  /* Start or stop walking.
   */
  if (nindx||nindy) {
    hero_walk_begin(sprite);
  } else {
    hero_walk_end(sprite);
  }
}

/* Update animation.
 * For now we tick 4 frames at a fixed rate regardless of state.
 * What those frames mean exactly, is render's problem.
 * Easy to imagine that will need to change eventually.
 */
 
static void hero_update_animation(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  if (g.input!=g.pvinput) {
    if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) hero_choose(sprite);
    if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) hero_item_begin(sprite);
    else if (!(g.input&EGG_BTN_SOUTH)&&(g.pvinput&EGG_BTN_SOUTH)) hero_item_end(sprite);
    hero_update_ind(sprite);
  }
  if (g.input&EGG_BTN_SOUTH) hero_item_update(sprite,elapsed);
  if (SPRITE->walking) hero_walk_update(sprite,elapsed);
  hero_update_animation(sprite,elapsed);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
  .render=hero_render,
};
