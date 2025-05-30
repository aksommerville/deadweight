#include "hero_internal.h"

/* Delete.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  fprintf(stderr,"%s %f,%f\n",__func__,sprite->x,sprite->y);
  SPRITE->input_blackout=1; // Buttons must go false before we acknowledge.
  return 0;
}

/* Launch the pause modal.
 */
 
static void hero_choose(struct sprite *sprite) {
  int x=(int)(sprite->x*NS_sys_tilesize);
  int y=(int)(sprite->y*NS_sys_tilesize);
  modal_new_pause(x,y);
  SPRITE->input_blackout=1;
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  int input=egg_input_get_one(0);
  if (SPRITE->input_blackout) {
    if (input&(EGG_BTN_LEFT|EGG_BTN_RIGHT|EGG_BTN_UP|EGG_BTN_DOWN|EGG_BTN_SOUTH|EGG_BTN_WEST)) {
      input=0;
    } else { // All meaningful buttons are released. End the blackout.
      SPRITE->input_blackout=0;
    }
  }
  if (input!=SPRITE->pvinput) {
    if ((input&EGG_BTN_WEST)&&!(SPRITE->pvinput&EGG_BTN_WEST)) { hero_choose(sprite); return; }
    if ((input&EGG_BTN_SOUTH)&&!(SPRITE->pvinput&EGG_BTN_SOUTH)) hero_item_begin(sprite);
    else if (!(input&EGG_BTN_SOUTH)&&(SPRITE->pvinput&EGG_BTN_SOUTH)) hero_item_end(sprite);
    SPRITE->pvinput=input;
  }
  if (input&EGG_BTN_SOUTH) hero_item_update(sprite,elapsed);
  
  switch (input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: sprite->x-=6.0*elapsed; break;
    case EGG_BTN_RIGHT: sprite->x+=6.0*elapsed; break;
  }
  switch (input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
    case EGG_BTN_UP: sprite->y-=6.0*elapsed; break;
    case EGG_BTN_DOWN: sprite->y+=6.0*elapsed; break;
  }
}

/* Render.
 */
 
static void _hero_render(struct sprite *sprite,int x,int y) {
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,sprite->tileid,sprite->xform);
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
