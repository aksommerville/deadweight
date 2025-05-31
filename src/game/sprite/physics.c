#include "game/game.h"

#define SMALL 0.001

/* Nonzero if there's any impassable cell for this sprite anywhere in the bounds provided.
 * OOB cells take their nearest neighbor, so the map's edge effectively extends to infinity.
 */
 
static int map_impassable(const struct sprite *sprite,int x,int y,int w,int h) {
  if ((w<1)||(h<1)) return 0;
  if (!sprite->solid) return 0;
  int cola=x; if (cola<0) cola=0; else if (cola>=NS_sys_mapw) cola=NS_sys_mapw-1;
  int colz=x+w-1; if (colz<0) colz=0; else if (colz>=NS_sys_mapw) colz=NS_sys_mapw-1;
  int rowa=y; if (rowa<0) rowa=0; else if (rowa>=NS_sys_maph) rowa=NS_sys_maph-1;
  int rowz=y+h-1; if (rowz<0) rowz=0; else if (rowz>=NS_sys_maph) rowz=NS_sys_maph-1;
  const uint8_t *major=g.map->cellv+rowa*NS_sys_mapw+cola;
  int row=rowa; for (;row<=rowz;row++,major+=NS_sys_mapw) {
    const uint8_t *minor=major;
    int col=cola; for (;col<=colz;col++,minor++) {
      uint8_t physics=g.physics[*minor];
      if (physics==NS_physics_solid) return 1;
      if (!sprite->airborne&&(physics==NS_physics_hole)) return 1;
    }
  }
  return 0;
}

/* How far can this sprite move in the direction indicated by (dx,dy)?
 * One of (dx,dy) must be zero.
 * Returns 0..abs(d).
 */
 
static double sprite_measure_freedom(const struct sprite *sprite,double dx,double dy) {

  /* If it crosses a map boundary, check the newly-covered cells.
   * If there's a collision, assume there's nothing else.
   */
  if (dx<0.0) {
    int cola=(int)(sprite->x+sprite->phl);
    int colz=(int)(sprite->x+sprite->phl+dx);
    if (colz<cola) {
      int row=(int)(sprite->y+sprite->pht);
      int rowc=(int)(sprite->y+sprite->phb-SMALL)-row+1;
      if (map_impassable(sprite,colz,row,cola-colz+1,rowc)) {
        return sprite->x+sprite->phl-(double)cola;
      }
    }
  } else if (dx>0.0) {
    int cola=(int)(sprite->x+sprite->phr-SMALL);
    int colz=(int)(sprite->x+sprite->phr-SMALL+dx);
    if (colz>cola) {
      int row=(int)(sprite->y+sprite->pht);
      int rowc=(int)(sprite->y+sprite->phb-SMALL)-row+1;
      if (map_impassable(sprite,cola+1,row,colz-cola,rowc)) {
        return (double)cola+1.0-sprite->phr-sprite->x;
      }
    }
  } else if (dy<0.0) {
    int rowa=(int)(sprite->y+sprite->pht);
    int rowz=(int)(sprite->y+sprite->pht+dy);
    if (rowz<rowa) {
      int col=(int)(sprite->x+sprite->phl);
      int colc=(int)(sprite->x+sprite->phr-SMALL)-col+1;
      if (map_impassable(sprite,col,rowz,colc,rowa-rowz+1)) {
        return sprite->y+sprite->pht-(double)rowa;
      }
    }
  } else if (dy>0.0) {
    int rowa=(int)(sprite->y+sprite->phb-SMALL);
    int rowz=(int)(sprite->y+sprite->phb-SMALL+dy);
    if (rowz>rowa) {
      int col=(int)(sprite->x+sprite->phl);
      int colc=(int)(sprite->x+sprite->phr-SMALL)-col+1;
      if (map_impassable(sprite,col,rowa+1,colc,rowz-rowa)) {
        return (double)rowa+1.0-sprite->phb-sprite->y;
      }
    }
  } else {
    return 0.0;
  }
  
  double freedom;
       if (dx<0.0) freedom=-dx;
  else if (dx>0.0) freedom=dx;
  else if (dy<0.0) freedom=-dy;
  else if (dy>0.0) freedom=dy;
  else return 0.0;
  
  double l=sprite->x+sprite->phl;
  double r=sprite->x+sprite->phr;
  double t=sprite->y+sprite->pht;
  double b=sprite->y+sprite->phb;
  struct sprite **p=g.spritev;
  int i=g.spritec;
  for (;i-->0;p++) {
    struct sprite *other=*p;
    if (other==sprite) continue;
    if (other->defunct) continue;
    if (!other->solid) continue;
    double free1;
    if (dx<0.0) {
      if (other->y+other->pht>=b) continue;
      if (other->y+other->phb<=t) continue;
      free1=l-other->phr-other->x;
    } else if (dx>0.0) {
      if (other->y+other->pht>=b) continue;
      if (other->y+other->phb<=t) continue;
      free1=other->x+other->phl-r;
    } else if (dy<0.0) {
      if (other->x+other->phl>=r) continue;
      if (other->x+other->phr<=l) continue;
      free1=t-other->phb-other->y;
    } else if (dy>0.0) {
      if (other->x+other->phl>=r) continue;
      if (other->x+other->phr<=l) continue;
      free1=other->y+other->pht-b;
    }
    if (free1<0.0) continue;
    if (free1<freedom) freedom=free1;
  }
  
  return freedom;
}

/* Relative motion.
 */
 
int sprite_move(struct sprite *sprite,double dx,double dy) {
  if (!sprite->solid) {
    sprite->x+=dx;
    sprite->y+=dy;
    return 2;
  }
  double ox=sprite->x;
  double oy=sprite->y;
  double nx=ox+dx;
  double ny=oy+dy;
  
  /* Firstly, if the desired position is valid go with it exactly.
   * This may cause skippage if the delta is too wide.
   */
  sprite->x=nx;
  sprite->y=ny;
  if (sprite_position_valid(sprite)) return 2;
  sprite->x=ox;
  sprite->y=oy;
  
  /* If we can advance in the target direction at all, take all we can get and nothing else.
   */
  double available=sprite_measure_freedom(sprite,dx,dy);
  //fprintf(stderr,"%s freedom toward (%+f,%+f): %f\n",sprite->type->name,dx,dy,available);
  if (available>0.0) {
         if (dx<0.0) sprite->x-=available;
    else if (dx>0.0) sprite->x+=available; 
    else if (dy<0.0) sprite->y-=available;
    else if (dy>0.0) sprite->y+=available;
    else return 0;
    return 1;
  }
  
  /* Try warping to the desired position.
   * This enables off-axis corrections.
   */
  if (sprite_warp(sprite,nx,ny)>0) return 1;
  
  /* Nope? OK, can't move.
   */
  sprite->x=ox;
  sprite->y=oy;
  return 0;
}

/* Absolute motion.
 */
 
int sprite_warp(struct sprite *sprite,double x,double y) {
  sprite->x=x;
  sprite->y=y;
  if (!sprite->solid) return 2;
  
  /* Look at each map cell and solid sprite that intersects me.
   * The first we find establishes up to four rectangular regions where we could place ourself.
   * Subsequent blockages reduce the size of those regions and remove empty regions.
   * If we run out of regions after the first is found, there is no legal position in range, so return zero.
   */
  const double jump_limit=0.500; // How far from (x,y) will we allow to deviate, per axis?
  double xlo=sprite->x-jump_limit;
  double ylo=sprite->y-jump_limit;
  double xhi=sprite->x+jump_limit;
  double yhi=sprite->y+jump_limit;
  struct range { double l,r,t,b; } rangev[4];
  int rangec=0;
  #define COLLISION(cl,ct,cr,cb) { \
    if (!rangec) { /* First collision: Populate (rangev) from scratch. */ \
      if (cl>xlo) { \
        rangev[rangec++]=(struct range){xlo,cl,ylo,yhi}; \
      } \
      if (ct>ylo) { \
        rangev[rangec++]=(struct range){xlo,xhi,ylo,ct}; \
      } \
      if (cr<xhi) { \
        rangev[rangec++]=(struct range){cr,xhi,ylo,yhi}; \
      } \
      if (cb<yhi) { \
        rangev[rangec++]=(struct range){xlo,xhi,cb,yhi}; \
      } \
    } else { /* Additional collision: Reduce size of existing ranges. */ \
      struct range *range=rangev+rangec-1; \
      int ii=rangec; \
      for (;ii-->0;range--) { \
        double escl=(cr)-range->l; if (escl<=0.0) continue; \
        double escr=range->r-(cl); if (escr<=0.0) continue; \
        double esct=(cb)-range->t; if (esct<=0.0) continue; \
        double escb=range->b-(ct); if (escb<=0.0) continue; \
        if ((escl<=escr)&&(escl<=esct)&&(escl<=escr)) range->l=cr; \
        else if ((escr<=esct)&&(escr<=escb)) range->r=cl; \
        else if (esct<=escb) range->t=cb; \
        else range->b=ct; \
        if ((range->l>=range->r)||(range->t>=range->b)) { \
          /* Range has become empty. Remove it. If it was the last range, abort. */ \
          rangec--; \
          if (!rangec) return 0; \
          memmove(range,range+1,sizeof(struct range)*(rangec-ii)); \
        } \
      } \
    } \
  }
  
  /* Check map, building up (rangev) and aborting if impossible.
   */
  int cola=(int)(sprite->x+sprite->phl); if (cola<0) cola=0; else if (cola>=NS_sys_mapw) cola=NS_sys_mapw-1;
  int colz=(int)(sprite->x+sprite->phr-SMALL); if (colz<0) colz=0; else if (colz>=NS_sys_mapw) colz=NS_sys_mapw-1;
  int rowa=(int)(sprite->y+sprite->pht); if (rowa<0) rowa=0; else if (rowa>=NS_sys_maph) rowa=NS_sys_maph-1;
  int rowz=(int)(sprite->y+sprite->phb-SMALL); if (rowz<0) rowz=0; else if (rowz>=NS_sys_maph) rowz=NS_sys_maph-1;
  if ((cola<=colz)&&(rowa<=rowz)) {
    const uint8_t *major=g.map->cellv+rowa*NS_sys_mapw+cola;
    int row=rowa; for (;row<=rowz;row++,major+=NS_sys_mapw) {
      const uint8_t *minor=major;
      int col=cola; for (;col<=colz;col++,minor++) {
        uint8_t physics=g.physics[*minor];
        if (
          (physics==NS_physics_solid)||
          ((physics==NS_physics_hole)&&!sprite->airborne)
        ) {
          COLLISION(col,row,col+1.0,row+1.0)
        }
      }
    }
  }
  
  /* Check sprites, building up (rangev) and aborting if impossible.
   */
  double l=sprite->x+sprite->phl;
  double r=sprite->x+sprite->phr;
  double t=sprite->y+sprite->pht;
  double b=sprite->y+sprite->phb;
  struct sprite **p=g.spritev;
  int i=g.spritec;
  for (;i-->0;p++) {
    struct sprite *other=*p;
    if (other==sprite) continue;
    if (other->defunct) continue;
    if (!other->solid) continue;
    if (other->x+other->phl>=r) continue;
    if (other->x+other->phr<=l) continue;
    if (other->y+other->pht>=b) continue;
    if (other->y+other->phb<=t) continue;
    COLLISION(other->x+other->phl,other->y+other->pht,other->x+other->phr,other->y+other->phb)
  }
  
  /* If we don't have any ranges at this point, it means we never had one, ie the move is perfect.
   */
  if (!rangec) return 2;
  
  /* Ranges haven't accounted for our own margins yet.
   * Apply those, and remove invalid ranges.
   * Entirely possible to abort at this point.
   */
  for (i=rangec;i-->0;) {
    struct range *range=rangev+i;
    range->l-=sprite->phl;
    range->r-=sprite->phr;
    range->t-=sprite->pht;
    range->b-=sprite->phb;
    if ((range->l>=range->r)||(range->t>=range->b)) {
      rangec--;
      if (!rangec) return 0;
      memmove(range,range+1,sizeof(struct range)*(rangec-i));
    }
  }
  
  /* Find the point nearest (x,y) in any of (rangev); that's our answer.
   * A score of zero might be possible due to round-off or something; in that case call it perfect.
   */
  double targetx=sprite->x,targety=sprite->y;
  double bestx=targetx,besty=targety;
  double bestscore=999999.0; // squared distance
  struct range *range=rangev;
  for (i=rangec;i-->0;range++) {
    double x,y;
    if (targetx<range->l) x=range->l;
    else if (targetx>range->r) x=range->r;
    else x=targetx;
    if (targety<range->t) y=range->t;
    else if (targety>range->b) y=range->b;
    else y=targety;
    double score=(x-targetx)*(x-targetx)+(y-targety)*(y-targety);
    if (score<=0.0) return 2; // (target) ie sprite's position, is already legal.
    if (score<bestscore) {
      bestx=x;
      besty=y;
      bestscore=score;
    }
  }
  if (bestscore>999000.0) return 0; // oops i miscalculated somewhere
  sprite->x=bestx;
  sprite->y=besty;
  return 1;
}

/* Test position.
 */

int sprite_position_valid(const struct sprite *sprite) {
  if (!sprite->solid) return 1;
  
  int col=(int)(sprite->x+sprite->phl);
  int row=(int)(sprite->y+sprite->pht);
  int colc=(int)(sprite->x+sprite->phr-SMALL)-col+1;
  int rowc=(int)(sprite->y+sprite->phb-SMALL)-row+1;
  if (map_impassable(sprite,col,row,colc,rowc)) return 0;
  
  double l=sprite->x+sprite->phl;
  double r=sprite->x+sprite->phr;
  double t=sprite->y+sprite->pht;
  double b=sprite->y+sprite->phb;
  struct sprite **p=g.spritev;
  int i=g.spritec;
  for (;i-->0;p++) {
    struct sprite *other=*p;
    if (other==sprite) continue;
    if (other->defunct) continue;
    if (!other->solid) continue;
    if (other->x+other->phl>=r) continue;
    if (other->x+other->phr<=l) continue;
    if (other->y+other->pht>=b) continue;
    if (other->y+other->phb<=t) continue;
    return 0;
  }
  
  //fprintf(stderr,"%s TRUE %s @ %f,%f\n",__func__,sprite->type->name,sprite->x,sprite->y);
  return 1;
}
