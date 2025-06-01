#include "hero_internal.h"

/* If an item is equipped and its quantity at least one, render Dot's arm carrying the item.
 * Arm is 0x03 and items are 0x04..0x0c (in canonical order).
 */
 
static void hero_render_item(struct sprite *sprite,int x,int y) {
  int itemid=store_get(NS_fld_equipped);
  if ((itemid<NS_fld_got_broom)||(itemid>NS_fld_got_candy)) return; // No item equipped.
  if (!store_get(itemid)) return; // Item not possessed.
  switch (itemid) { // Check items with quantity; if zero, don't render the arm.
    case NS_fld_got_pepper: if (!store_get(NS_fld_qty_pepper)) return; break;
    case NS_fld_got_bomb: if (!store_get(NS_fld_qty_bomb)) return; break;
    case NS_fld_got_candy: if (!store_get(NS_fld_qty_candy)) return; break;
  }
  /* OK we're rendering it.
   * Arm and item go at the same position and transform, at an offset from the body determined by face dir.
   */
  uint8_t xform=0;
  y+=2; // Same vertical offset regardless of orientation.
  if (SPRITE->facedx<0) {
    xform=EGG_XFORM_XREV;
    x-=6;
  } else if (SPRITE->facedx>0) {
    x+=6;
  } else if (SPRITE->facedy<0) {
    xform=EGG_XFORM_XREV;
    x-=6;
  } else {
    x+=6;
  }
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,0x04+itemid-NS_fld_got_broom,xform);
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,0x03,xform);
}

/* Render, main entry point.
 * The hero's rendering is complicated enough to deserve its own file.
 */
 
void hero_render(struct sprite *sprite,int x,int y) {

  // If facing north, the arm renders before the body.
  if (SPRITE->facedy<0) hero_render_item(sprite,x,y);

  // Main body. TODO This is not going to be adequate, once we add item states.
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
  
  // Any direction except north, the arm renders after the body.
  if (SPRITE->facedy>=0) hero_render_item(sprite,x,y);
}
