#include "hero_internal.h"

/* Pepper, bomb, candy: Check quantity, then create another sprite that does the real work.
 */
 
static void hero_pepper_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_pepper);
  if (qty<1) return;
  store_set(NS_fld_qty_pepper,qty-1);
  fprintf(stderr,"%s\n",__func__);
  //TODO create pepper sprite
}
 
static void hero_bomb_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_bomb);
  if (qty<1) return;
  store_set(NS_fld_qty_bomb,qty-1);
  fprintf(stderr,"%s\n",__func__);
  //TODO create bomb sprite
}
 
static void hero_candy_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_candy);
  if (qty<1) return;
  store_set(NS_fld_qty_candy,qty-1);
  fprintf(stderr,"%s\n",__func__);
  //TODO create candy sprite
}

/* Broom.
 */
 
static void hero_broom_begin(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

/* Stopwatch.
 */
 
static void hero_stopwatch_begin(struct sprite *sprite) {
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

/* Wand.
 */
 
static void hero_wand_begin(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);//TODO
}

/* Begin item.
 */
 
void hero_item_begin(struct sprite *sprite) {
  int equipped=store_get(NS_fld_equipped);
  if ((equipped<NS_fld_got_broom)||(equipped>NS_fld_got_candy)) return;
  if (!store_get(equipped)) return;
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
  fprintf(stderr,"%s\n",__func__);
}

/* Continue item use.
 */
 
void hero_item_update(struct sprite *sprite,double elapsed) {
}
