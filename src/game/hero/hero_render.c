#include "hero_internal.h"

/* Riding broom.
 * Nothing else renders.
 */
 
static void hero_render_broom(struct sprite *sprite,int x,int y) {
  uint8_t shadow_tileid=(SPRITE->renderclock&32)?0x15:0x16;
  uint8_t dot_tileid=SPRITE->indx?0x14:0x13;
  uint8_t xform=(SPRITE->facedx>0)?EGG_XFORM_XREV:0;
  
  // TODO Can we render the shadow in a separate pass before all sprites?
  if (SPRITE->renderclock&1) { // Shadow only displays on alternate frames.
    graf_draw_tile(&g.graf,g.texid_sprites,x,y+8,shadow_tileid,0);
  }
  
  if (shadow_tileid==0x16) y-=3; else y-=4;
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,dot_tileid,xform);
}

/* Using snowglobe.
 */
 
static void hero_render_snowglobe(struct sprite *sprite,int x,int y) {
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,0x00,0); // Always the South orientation.
  // If one and only one input direction is indicated, globe in that direction.
  if ((SPRITE->indx<0)&&!SPRITE->indy) {
    graf_draw_tile(&g.graf,g.texid_sprites,x-NS_sys_tilesize,y+4,0x35,EGG_XFORM_SWAP);
    graf_draw_tile(&g.graf,g.texid_sprites,x-8,y+2,0x03,EGG_XFORM_XREV);
  } else if ((SPRITE->indx>0)&&!SPRITE->indy) {
    graf_draw_tile(&g.graf,g.texid_sprites,x+NS_sys_tilesize,y+4,0x35,EGG_XFORM_SWAP|EGG_XFORM_YREV);
    graf_draw_tile(&g.graf,g.texid_sprites,x+8,y+2,0x03,0);
  } else if (!SPRITE->indx&&(SPRITE->indy<0)) {
    graf_draw_tile(&g.graf,g.texid_sprites,x,y-9,0x35,0);
    graf_draw_tile(&g.graf,g.texid_sprites,x-6,y-1,0x03,EGG_XFORM_SWAP|EGG_XFORM_XREV);
    graf_draw_tile(&g.graf,g.texid_sprites,x+2,y-1,0x03,EGG_XFORM_SWAP|EGG_XFORM_XREV);
  } else if (!SPRITE->indx&&(SPRITE->indy>0)) {
    graf_draw_tile(&g.graf,g.texid_sprites,x,y+NS_sys_tilesize,0x35,EGG_XFORM_YREV);
    graf_draw_tile(&g.graf,g.texid_sprites,x-6,y+7,0x03,EGG_XFORM_SWAP);
    graf_draw_tile(&g.graf,g.texid_sprites,x+2,y+7,0x03,EGG_XFORM_SWAP);
  } else {
    graf_draw_tile(&g.graf,g.texid_sprites,x,y,0x35,0);
  }
}

/* If an item is equipped and its quantity at least one, render Dot's arm carrying the item.
 * Arm is 0x03 and items are 0x04..0x0c (in canonical order).
 */
 
static void hero_render_item(struct sprite *sprite,int x,int y,int itemid) {
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

/* Compass overlay.
 * It's an indicator in two parts that rotates around the hero, orienting to the direction of the target.
 * Doing this instead of the more obvious rotating arrow, since rotating sprites was definitely not a thing on the NES.
 */
 
static void hero_render_compass(struct sprite *sprite,int x,int y) {
  if ((SPRITE->renderclock&0x1f)>22) return; // blink a little, but bias duty cycle toward visible
  
  double targetx,targety;
  if (g.princess) { // Princess sprite exists, awesome. Once rescued, this is usually the case, even when she's offscreen.
    targetx=g.princess->x;
    targety=g.princess->y;
  } else {
    return; // TODO Important! Determine the relative world position of the dungeon where we rescue the Princess.
  }
  
  double dx=targetx-sprite->x;
  double dy=targety-sprite->y;
  if ((dx>=-1.0)&&(dx<=1.0)&&(dy>=-1.0)&&(dy<=1.0)) {
    // This close, I hope you don't need further help from the compass.
    // And mostly, aborting to dodge divide-by-zero or rounding errors at very short distances.
    return;
  }
  double distance=sqrt(dx*dx+dy*dy);
  dx/=distance; // Normalize (dx,dy).
  dy/=distance;
  int ax=x+(int)(1.000*dx*NS_sys_tilesize);
  int ay=y+(int)(1.000*dy*NS_sys_tilesize);
  int bx=x+(int)(1.500*dx*NS_sys_tilesize);
  int by=y+(int)(1.500*dy*NS_sys_tilesize);
  graf_draw_tile(&g.graf,g.texid_sprites,ax,ay,0x1e,0);
  graf_draw_tile(&g.graf,g.texid_sprites,bx,by,0x1f,0);
}

/* Render, main entry point.
 * The hero's rendering is complicated enough to deserve its own file.
 */
 
void hero_render(struct sprite *sprite,int x,int y) {
  SPRITE->renderclock++;

  // A few in-use items are special enough to get their own whole thing.
  switch (SPRITE->using_item) {
    case NS_fld_got_broom: hero_render_broom(sprite,x,y); return;
    case NS_fld_got_snowglobe: hero_render_snowglobe(sprite,x,y); return;
  }

  // If facing north, the arm renders before the body.
  int equipped=store_get(NS_fld_equipped);
  if (SPRITE->facedy<0) hero_render_item(sprite,x,y,equipped);

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
  if (SPRITE->facedy>=0) hero_render_item(sprite,x,y,equipped);
  
  // If the compass is equipped, it's entirely passive, render its output above the hero always.
  if (equipped==NS_fld_got_compass) hero_render_compass(sprite,x,y);
}
