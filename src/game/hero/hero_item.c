#include "hero_internal.h"

/* Pepper, bomb, candy: Check quantity, then create another sprite that does the real work.
 * Play "reject" if quantity zero, just as if nothing were equipped.
 */
 
static void hero_pepper_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_pepper);
  if (qty<1) {
    egg_play_sound(RID_sound_reject);
    return;
  }
  store_set(NS_fld_qty_pepper,qty-1);
  fprintf(stderr,"%s\n",__func__);
  //TODO create pepper sprite
}
 
static void hero_bomb_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_bomb);
  if (qty<1) {
    egg_play_sound(RID_sound_reject);
    return;
  }
  store_set(NS_fld_qty_bomb,qty-1);
  fprintf(stderr,"%s\n",__func__);
  //TODO create bomb sprite
}
 
static void hero_candy_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_candy);
  if (qty<1) {
    egg_play_sound(RID_sound_reject);
    return;
  }
  store_set(NS_fld_qty_candy,qty-1);
  fprintf(stderr,"%s\n",__func__);
  //TODO create candy sprite
}

/* Broom.
 */
 
static void hero_broom_begin(struct sprite *sprite) {
  SPRITE->using_item=NS_fld_got_broom;
  sprite->airborne=1;
  // When flying, we can move in any direction but face dir is constrained to horizontal.
  if (!SPRITE->facedx) {
    if (SPRITE->indx) SPRITE->facedx=SPRITE->indx;
    else SPRITE->facedx=-1; // got to be one of them, whatever.
    SPRITE->facedy=0;
  }
}

static void hero_broom_end(struct sprite *sprite) {
  //TODO If we're over a hole, quietly noop.
  sprite->airborne=0;
  SPRITE->using_item=0;
}

/* Stopwatch.
 */
 
static void hero_stopwatch_begin(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

static void hero_stopwatch_end(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

/* Camera.
 */
 
static void hero_camera_begin(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

/* Snowglobe.
 */
 
static void hero_snowglobe_begin(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

static void hero_snowglobe_end(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

/* Wand.
 */
 
static void hero_wand_begin(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

static void hero_wand_end(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

/* Begin item.
 */
 
void hero_item_begin(struct sprite *sprite) {

  /* Don't begin using an item if one is already in use.
   * This is unusual, because normally releasing A ends the item usage (plus most items are not sustained).
   * But it can happen for the Broom, if the player released A while over a hole.
   * So it's a very passive noop.
   */
  if (SPRITE->using_item) return;
  
  /* Normally if you press A and there's nothing equipped, a rejection sound is in order.
   */
  int equipped=store_get(NS_fld_equipped);
  if ((equipped<NS_fld_got_broom)||(equipped>NS_fld_got_candy)||!store_get(equipped)) {
    egg_play_sound(RID_sound_reject);
    return;
  }

  switch (equipped) {
    case NS_fld_got_broom: hero_broom_begin(sprite); break;
    case NS_fld_got_pepper: hero_pepper_begin(sprite); break;
    case NS_fld_got_compass: break; // nothing, it's entirely passive
    case NS_fld_got_stopwatch: hero_stopwatch_begin(sprite); break;
    case NS_fld_got_camera: hero_camera_begin(sprite); break;
    case NS_fld_got_snowglobe: hero_snowglobe_begin(sprite); break;
    case NS_fld_got_wand: hero_wand_begin(sprite); break;
    case NS_fld_got_bomb: hero_bomb_begin(sprite); break;
    case NS_fld_got_candy: hero_candy_begin(sprite); break;
  }
}

/* End item.
 */
 
void hero_item_end(struct sprite *sprite) {
  switch (SPRITE->using_item) {
    case NS_fld_got_broom: hero_broom_end(sprite); break;
    case NS_fld_got_stopwatch: hero_stopwatch_end(sprite); break;
    case NS_fld_got_snowglobe: hero_snowglobe_end(sprite); break;
    case NS_fld_got_wand: hero_wand_end(sprite); break;
    default: SPRITE->using_item=0;
  }
}

/* Continue item use.
 */
 
void hero_item_update(struct sprite *sprite,double elapsed) {
}
