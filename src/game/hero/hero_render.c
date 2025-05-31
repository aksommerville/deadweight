#include "hero_internal.h"

/* Render, main entry point.
 * The hero's rendering is complicated enough to deserve its own file.
 */
 
void hero_render(struct sprite *sprite,int x,int y) {
  uint8_t tileid=sprite->tileid;
  uint8_t xform=0;
  if (SPRITE->facedy>0) {
    // Facing south, the default.
  } else if (SPRITE->facedy<0) {
    tileid+=1; // North faces are right of south.
  } else if (SPRITE->facedx<0) {
    tileid+=2; // West faces are two columns right of south; west is the natural orientation.
  } else {
    tileid+=2;
    xform=EGG_XFORM_XREV;
  }
  if (SPRITE->walking) switch (SPRITE->animframe) {
    case 1: tileid+=0x10; break;
    case 3: tileid+=0x20; break;
  }
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,tileid,xform);
}
