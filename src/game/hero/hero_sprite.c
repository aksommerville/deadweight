#include "hero_internal.h"

/* Delete.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  fprintf(stderr,"%s %f,%f\n",__func__,sprite->x,sprite->y);
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
  }
  if (g.input&EGG_BTN_SOUTH) hero_item_update(sprite,elapsed);
  
  switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: sprite->x-=6.0*elapsed; break;
    case EGG_BTN_RIGHT: sprite->x+=6.0*elapsed; break;
  }
  switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
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
