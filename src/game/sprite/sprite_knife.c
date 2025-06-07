/* sprite_knife.c
 * Oscillates back and forth while appearing to rotate.
 * A global flags "knivesoff" turns off knives everywhere (they're only used in one place).
 */
 
#include "game/game.h"

#define KNIFE_SPEED 7.0
#define KNIFE_RADIUS 0.500 /* square */

struct sprite_knife {
  struct sprite hdr;
  double animclock;
  int animframe;
  uint8_t tileid0;
  double dx,dy; // Speed baked in.
  double xlo,xhi,ylo,yhi; // Bounds of motion, determined at init.
  int reset;
};

#define SPRITE ((struct sprite_knife*)sprite)

// For a valid cell (x,y), populate (xlo,xhi,ylo,yhi) with the two axis-aligned paths we could follow.
static void knife_measure_paths(double *xlo,double *xhi,double *ylo,double *yhi,int x,int y) {
  *xlo=*xhi=x+0.5;
  *ylo=*yhi=y+0.5;
  int x0=x,y0=y;
  for (x=x0,y=y0;;) {
    x--;
    if (x<0) break;
    if (g.physics[g.map->cellv[y*NS_sys_mapw+x]]==NS_physics_solid) break;
    *xlo=x+0.5;
  }
  for (x=x0,y=y0;;) {
    x++;
    if (x>=NS_sys_mapw) break;
    if (g.physics[g.map->cellv[y*NS_sys_mapw+x]]==NS_physics_solid) break;
    *xhi=x+0.5;
  }
  for (x=x0,y=y0;;) {
    y--;
    if (y<0) break;
    if (g.physics[g.map->cellv[y*NS_sys_mapw+x]]==NS_physics_solid) break;
    *ylo=y+0.5;
  }
  for (x=x0,y=y0;;) {
    y++;
    if (y>=NS_sys_maph) break;
    if (g.physics[g.map->cellv[y*NS_sys_mapw+x]]==NS_physics_solid) break;
    *yhi=y+0.5;
  }
}

static void knife_reset_path(struct sprite *sprite) {
  // Determine initial direction of motion, which also determines permanent axis, speed, and path.
  SPRITE->xlo=SPRITE->xhi=sprite->x;
  SPRITE->ylo=SPRITE->yhi=sprite->y;
  if (g.map) {
    int x=(int)sprite->x;
    int y=(int)sprite->y;
    if ((x>=0)&&(y>=0)&&(x<NS_sys_mapw)&&(y<NS_sys_maph)) {
      double xlo,xhi,ylo,yhi;
      knife_measure_paths(&xlo,&xhi,&ylo,&yhi,x,y);
      double xrange=xhi-xlo;
      double yrange=yhi-ylo;
      // Do not accept any range 2 or shorter, and if they're both long enough take the shorter.
      if (xrange<2.0) xrange=999.0;
      if (yrange<2.0) yrange=999.0;
      if (xrange<=yrange) {
        SPRITE->xlo=xlo;
        SPRITE->xhi=xhi;
        SPRITE->dx=KNIFE_SPEED;
        double dlo=sprite->x-xlo;
        double dhi=xhi-sprite->x;
        if (dlo>dhi) SPRITE->dx=-SPRITE->dx;
      } else {
        SPRITE->ylo=ylo;
        SPRITE->yhi=yhi;
        SPRITE->dy=KNIFE_SPEED;
        double dlo=sprite->y-ylo;
        double dhi=yhi-sprite->y;
        if (dlo>dhi) SPRITE->dy=-SPRITE->dy;
      }
    }
  }
  SPRITE->reset=0;
}

static int _knife_init(struct sprite *sprite) {
  if (store_get(NS_fld_knivesoff)) {
    sprite->defunct=1;
    return 0;
  }
  sprite->airborne=1;
  SPRITE->tileid0=sprite->tileid;
  knife_reset_path(sprite);
  return 0;
}

static void _knife_update(struct sprite *sprite,double elapsed) {
  
  // Deal damage.
  int i=g.spritec;
  while (i-->0) {
    struct sprite *victim=g.spritev[i];
    if (victim->defunct) continue;
    if (!victim->type->hurt) continue;
    
    double dx=victim->x-sprite->x;
    if ((dx<-KNIFE_RADIUS)||(dx>KNIFE_RADIUS)) continue;
    double dy=victim->y-sprite->y;
    if ((dy<-KNIFE_RADIUS)||(dy>KNIFE_RADIUS)) continue;
    
    victim->type->hurt(victim,sprite);
  }
  
  if (g.time_stopped) return;
  
  if (sprite->summoning) SPRITE->reset=1;
  else if (SPRITE->reset) knife_reset_path(sprite);

  // 8 frames of animation, using 2 tiles with 4 transforms.
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.0625;
    if (++(SPRITE->animframe)>=8) SPRITE->animframe=0;
    sprite->tileid=SPRITE->tileid0+(SPRITE->animframe&1);
    switch (SPRITE->animframe>>1) {
      case 0: sprite->xform=0; break;
      case 1: sprite->xform=EGG_XFORM_SWAP|EGG_XFORM_YREV; break;
      case 2: sprite->xform=EGG_XFORM_XREV|EGG_XFORM_YREV; break;
      case 3: sprite->xform=EGG_XFORM_XREV|EGG_XFORM_SWAP; break;
    }
  }
  
  // Move.
  double dx=SPRITE->dx*elapsed,dy=SPRITE->dy*elapsed;
  sprite->x+=dx;
  sprite->y+=dy;
  
  // If we exceeded our bounds, back out the move and reverse direction.
  if ((sprite->x<SPRITE->xlo)||(sprite->x>SPRITE->xhi)||(sprite->y<SPRITE->ylo)||(sprite->y>SPRITE->yhi)) {
    sprite->x-=dx;
    sprite->y-=dy;
    SPRITE->dx*=-1.0;
    SPRITE->dy*=-1.0;
  }
}

const struct sprite_type sprite_type_knife={
  .name="knife",
  .objlen=sizeof(struct sprite_knife),
  .init=_knife_init,
  .update=_knife_update,
};
