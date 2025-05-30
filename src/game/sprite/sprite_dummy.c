#include "game/game.h"

struct sprite_dummy {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_dummy*)sprite)

/* Delete.
 */
 
static void _dummy_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _dummy_init(struct sprite *sprite) {
  return 0;
}

/* Update.
 */
 
static void _dummy_update(struct sprite *sprite,double elapsed) {
}

/* Render.
 */
 
static void _dummy_render(struct sprite *sprite,int x,int y) {
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,sprite->tileid,sprite->xform);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_dummy={
  .name="dummy",
  .objlen=sizeof(struct sprite_dummy),
  //.del=_dummy_del,
  //.init=_dummy_init,
  //.update=_dummy_update,
  //.render=_dummy_render,
};
