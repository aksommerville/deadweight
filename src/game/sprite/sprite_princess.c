#include "game/game.h"

struct sprite_princess {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_princess*)sprite)

/* Delete.
 */
 
static void _princess_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _princess_init(struct sprite *sprite) {
  return 0;
}

/* Update.
 */
 
static void _princess_update(struct sprite *sprite,double elapsed) {
  //XXX very temporary. advance toward the hero dumbly.
  if (g.hero) {
    const double SPEED=5.0;
    double dx=g.hero->x-sprite->x;
    double dy=g.hero->y-sprite->y;
    if (dx<-2.0) sprite->x-=SPEED*elapsed;
    else if (dx>2.0) sprite->x+=SPEED*elapsed;
    if (dy<-2.0) sprite->y-=SPEED*elapsed;
    else if (dy>2.0) sprite->y+=SPEED*elapsed;
  }
}

/* Render.
 */
 
static void _princess_render(struct sprite *sprite,int x,int y) {
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,sprite->tileid,sprite->xform);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_princess={
  .name="princess",
  .objlen=sizeof(struct sprite_princess),
  .del=_princess_del,
  .init=_princess_init,
  .update=_princess_update,
  .render=_princess_render,
};
