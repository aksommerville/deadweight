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
  SPRITE->respawn_cooldown=0.500;
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
  
  // Some items inhibit walking.
  switch (SPRITE->using_item) {
    case NS_fld_got_snowglobe:
    case NS_fld_got_wand:
      return;
  }
  
  SPRITE->walking=1;
  SPRITE->animclock=0.0;
  SPRITE->animframe=0;
}

static void hero_walk_end(struct sprite *sprite) {
  if (!SPRITE->walking) return;
  SPRITE->walking=0;
}

static void hero_walk_update(struct sprite *sprite,double elapsed) {
  if (SPRITE->walking) {
    double speed=HERO_WALK_SPEED;
    if (SPRITE->using_item==NS_fld_got_broom) speed=HERO_BROOM_SPEED;
    sprite_move(sprite,speed*SPRITE->indx*elapsed,speed*SPRITE->indy*elapsed);
  }
}

/* Update (indx,indy), motion state, and (faced) in response to an input change.
 */
 
void hero_update_ind(struct sprite *sprite) {
  
  // Some items inhibit movement, and should not update (ind).
  // These should call us again after unsetting (using_item), when the action ends.
  switch (SPRITE->using_item) {
    case NS_fld_got_wand:
        return;
  }
  
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
   * When the broom is engaged, do not change to a vertical face.
   */
  if (nindx&&(nindx!=SPRITE->indx)) {
    SPRITE->facedx=nindx;
    SPRITE->facedy=0;
  } else if (nindy&&(nindy!=SPRITE->indy)&&(SPRITE->using_item!=NS_fld_got_broom)) {
    SPRITE->facedx=0;
    SPRITE->facedy=nindy;
  } else if (nindx) {
    SPRITE->facedx=nindx;
    SPRITE->facedy=0;
  } else if (nindy&&(SPRITE->using_item!=NS_fld_got_broom)) {
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
  if (SPRITE->item_cooldown>0.0) SPRITE->item_cooldown-=elapsed;
  if (SPRITE->respawn_cooldown>0.0) {
    SPRITE->respawn_cooldown-=elapsed;
    return;
  }
  if ((g.input!=g.pvinput)||(g.input!=SPRITE->pvinput)) {
    SPRITE->pvinput=g.input;
    if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) {
      hero_item_end(sprite);
      hero_choose(sprite);
      SPRITE->indx=SPRITE->indy=0;
      SPRITE->walking=0;
      return;
    }
    if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) hero_item_begin(sprite);
    hero_update_ind(sprite);
  }
  if (g.input&EGG_BTN_SOUTH) hero_item_update(sprite,elapsed);
  else if (!(g.input&EGG_BTN_SOUTH)&&SPRITE->using_item) hero_item_end(sprite);
  hero_walk_update(sprite,elapsed);
  hero_update_animation(sprite,elapsed);
}

/* Hurt.
 */
 
static int _hero_hurt(struct sprite *sprite,struct sprite *assailant) {
  if (SPRITE->respawn_cooldown>0.0) return 0;
  if (assailant->type==&sprite_type_ssflame) return 0; // Pepper doesn't hurt us, we like it spicy.
  egg_play_sound(RID_sound_hurt_hero);
  sprite->defunct=1; // Framework manages soulballs and respawn.
  hero_item_end(sprite);
  store_set(NS_fld_death_count,store_get(NS_fld_death_count)+1);
  return 1;
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
  .hurt=_hero_hurt,
};

/* Notify of lost focus.
 */

void sprite_hero_losing_focus(struct sprite *sprite) {
  if (!sprite||(sprite->type!=&sprite_type_hero)) return;
  hero_item_end(sprite);
  SPRITE->indx=SPRITE->indy=0;
  SPRITE->walking=0;
}
