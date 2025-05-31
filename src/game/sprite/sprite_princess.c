#include "game/game.h"

// The path needn't be very long, it's fine (desirable even) to recalculate often.
#define PRINCESS_PATH_LIMIT 8
#define PRINCESS_SPEED 5.0

struct sprite_princess {
  struct sprite hdr;
  uint8_t tileid0;
  int tilecol;
  double animclock;
  int animframe;
  uint8_t pathv[PRINCESS_PATH_LIMIT]; // Cell index ie (y*NS_sys_mapw+x), all neighbors here are cardinally adjacent.
  int pathp,pathc;
  int mapid;
  int idle;
};

#define SPRITE ((struct sprite_princess*)sprite)

/* Delete.
 */
 
static void _princess_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _princess_init(struct sprite *sprite) {
  sprite->phl=-0.400;
  sprite->phr=0.400;
  sprite->pht=-0.250;
  sprite->phb=0.500;
  SPRITE->tileid0=sprite->tileid;
  return 0;
}

/* Return a valid column in one row, or a valid row in one column.
 * Valid as in, we can walk on it.
 * Provided point must be in bounds.
 */
 
static inline int princess_cell_is_valid(int x,int y) {
  if ((x<0)||(x>=NS_sys_mapw)||(y<0)||(y>=NS_sys_maph)) return 0;
  uint8_t physics=g.physics[g.map->cellv[y*NS_sys_mapw+x]];
  if (physics==NS_physics_vacant) return 1;
  return 0;
}
 
static int princess_find_valid_col(int col,int row) {
  if ((col<0)||(col>=NS_sys_mapw)||(row<0)||(row>=NS_sys_maph)) return -1;
  int d=0; for (;d<NS_sys_mapw;d++) {
    if (princess_cell_is_valid(col-d,row)) return col-d;
    if (princess_cell_is_valid(col+d,row)) return col+d;
  }
  return -1;
}

static int princess_find_valid_row(int col,int row) {
  if ((col<0)||(col>=NS_sys_mapw)||(row<0)||(row>=NS_sys_maph)) return -1;
  int d=0; for (;d<NS_sys_maph;d++) {
    if (princess_cell_is_valid(col,row-d)) return row-d;
    if (princess_cell_is_valid(col,row+d)) return row+d;
  }
  return -1;
}

/* Examine the current state of affairs and generate a new path if we need one.
 */
 
static void princess_rebuild_path_if_needed(struct sprite *sprite) {
  SPRITE->idle=1;
  if (!g.hero) return;
  int sprcol=(int)sprite->x;
  int sprrow=(int)sprite->y;
  int dotcol=(int)g.hero->x;
  int dotrow=(int)g.hero->y;
  int sprcol0=sprcol;
  int sprrow0=sprrow;
  int dx=dotcol-sprcol;
  int dy=dotrow-sprrow;
  if ((dx>=-1)&&(dx<=1)&&(dy>=-1)&&(dy<=1)) return;
  SPRITE->pathp=0;
  SPRITE->pathc=0;
  
  /* If we're separated by more than a screen, forget it.
   * Likewise, OOB on multiple axes.
   * These situations arise when you leave the princess far behind.
   * It's sensible, and it's also a technical requirement: We only want to examine one map, the current one.
   */
  if ((dx<-NS_sys_mapw)||(dx>NS_sys_mapw)||(dy<-NS_sys_maph)||(dy>NS_sys_maph)) return;
  if ((sprcol<0)||(sprcol>=NS_sys_mapw)) {
    if ((sprrow<0)||(sprrow>=NS_sys_maph)) return;
  }
  
  /* One of (sprcol,sprrow) may be OOB at this point, eg they usually are, right after a map change.
   * Clamp to the nearest in-bounds cell, then slide along the edge to find the nearest legal cell.
   * If we don't find one on the edge, abort. eg hero broomed across a lake.
   * In this OOB case, tighten the distance limit. She might have been trapped on that other map and we wouldn't know from here.
   */
  const int too_far_oob=5;
  if ((sprcol<=-too_far_oob)||(sprrow<=-too_far_oob)||(sprcol>=NS_sys_mapw+too_far_oob)||(sprrow>=NS_sys_maph+too_far_oob)) return;
  if (sprcol<0) {
    sprcol=0;
    if ((sprrow=princess_find_valid_row(sprcol,sprrow))<0) return;
  } else if (sprcol>=NS_sys_mapw) {
    sprcol=NS_sys_mapw-1;
    if ((sprrow=princess_find_valid_row(sprcol,sprrow))<0) return;
  } else if (sprrow<0) {
    sprrow=0;
    if ((sprcol=princess_find_valid_col(sprcol,sprrow))<0) return;
  } else if (sprrow>=NS_sys_maph) {
    sprrow=NS_sys_maph-1;
    if ((sprcol=princess_find_valid_col(sprcol,sprrow))<0) return;
  }
  
  /* (dotcol,dotrow) should be in bounds already, but if not, clamp it.
   */
  if (dotcol<0) dotcol=0; else if (dotcol>=NS_sys_mapw) dotcol=NS_sys_mapw-1;
  if (dotrow<0) dotrow=0; else if (dotrow>=NS_sys_maph) dotrow=NS_sys_maph-1;
  
  /* Now the real meat-and-potatoes of path generation, from (sprcol,sprrow) to (dotcol,dotrow).
   * The princess is going to be dumb: She will only advance toward Dot, never stepping in a suboptimal direction even to walk around some blockage.
   * If a path exists such that every step brings her closer, we will find it.
   */
  struct pathgen_step {
    uint8_t x,y; // Absolute coordinates, as a convenience.
    int8_t dx,dy; // Which is the optimal direction. Can be zero, if we're already aligned.
    uint8_t xok,yok; // Nonzero if the given axis looks doable.
    uint8_t choice; // 0=horizontal, 1=vertical, which one are we trying right now?
  } stepv[NS_sys_mapw+NS_sys_maph];
  int stepc=0;
  
  /* Seed generator with the first step.
   */
  struct pathgen_step *step=stepv+stepc++;
  step->x=sprcol;
  step->y=sprrow;
  if (dotcol<step->x) step->dx=-1; else if (dotcol>step->x) step->dx=1; else step->dx=0;
  if (dotrow<step->y) step->dy=-1; else if (dotrow>step->y) step->dy=1; else step->dy=0;
  if (step->dx) step->xok=princess_cell_is_valid(step->x+step->dx,step->y); else step->xok=0;
  if (step->dy) step->yok=princess_cell_is_valid(step->x,step->y+step->dy); else step->yok=0;
  
  /* Iterate until we connect the Dot:
   *  - If the step list is empty, abort.
   *  - If the list's head is within range, commit.
   *  - If the head has further options, advance and continue.
   *  - Back out the head and drop it from the previous step.
   */
  for (;;) {
    if (stepc<1) return;
    step=stepv+stepc-1;
    int dx=dotcol-step->x;
    int dy=dotrow-step->y;
    if ((dx>=-1)&&(dx<=1)&&(dy>=-1)&&(dy<=1)) break;
    
    if (step->xok&&step->yok) {
      int adx=dotcol-step->x; if (adx<0) adx=-adx;
      int ady=dotrow-step->y; if (ady<0) ady=-ady;
      if (adx<=ady) step->choice=0;
      else step->choice=1;
    } else if (step->xok) {
      step->choice=0;
    } else if (step->yok) {
      step->choice=1;
    } else { // Can't advance from here. Back out and either abort or poison this direction.
      stepc--;
      if (!stepc) return;
      step--;
      if (step->choice) step->yok=0;
      else step->xok=0;
      continue;
    }
    
    // We can advance.
    if (stepc>=NS_sys_mapw+NS_sys_maph) break; // ...oh hey, no we can't. Take this path, whatever it is.
    struct pathgen_step *next=stepv+stepc++;
    if (step->choice) {
      next->x=step->x;
      next->y=step->y+step->dy;
    } else {
      next->x=step->x+step->dx;
      next->y=step->y;
    }
    if (dotcol<next->x) next->dx=-1; else if (dotcol>next->x) next->dx=1; else next->dx=0;
    if (dotrow<next->y) next->dy=-1; else if (dotrow>next->y) next->dy=1; else next->dy=0;
    if (next->dx) next->xok=princess_cell_is_valid(next->x+next->dx,next->y); else next->xok=0;
    if (next->dy) next->yok=princess_cell_is_valid(next->x,next->y+next->dy); else next->yok=0;
  }
  
  /* Commit steps to our finished path.
   */
  int i=stepc;
  for (step=stepv;i-->0;step++) {
    if (SPRITE->pathc>=PRINCESS_PATH_LIMIT) break;
    SPRITE->pathv[SPRITE->pathc++]=step->y*NS_sys_mapw+step->x;
  }
  
  // Update face for the first step, if we got anything.
  if (SPRITE->pathc>0) {
    SPRITE->idle=0;
    int dstcol=SPRITE->pathv[0]%NS_sys_mapw;
    int dstrow=SPRITE->pathv[0]/NS_sys_mapw;
    if (dstrow>sprrow0) {
      sprite->xform=0;
      SPRITE->tilecol=0;
    } else if (dstrow<sprrow0) {
      sprite->xform=0;
      SPRITE->tilecol=1;
    } else if (dstcol<sprcol0) {
      sprite->xform=0;
      SPRITE->tilecol=2;
    } else if (dstcol>sprcol0) {
      sprite->xform=EGG_XFORM_XREV;
      SPRITE->tilecol=2;
    }
  }
}

/* Update.
 */
 
static void _princess_update(struct sprite *sprite,double elapsed) {

  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
  
  // If the map changed, force a path rebuild. Princess sprite is special, it does survive map changes.
  if (g.map&&(g.map->rid!=SPRITE->mapid)) {
    SPRITE->mapid=g.map->rid;
    SPRITE->pathp=SPRITE->pathc=0;
  }
  
  if (SPRITE->pathp>=SPRITE->pathc) {
    princess_rebuild_path_if_needed(sprite);
    
  } else {
    int dstcol=SPRITE->pathv[SPRITE->pathp]%NS_sys_mapw;
    int dstrow=SPRITE->pathv[SPRITE->pathp]/NS_sys_mapw;
    double dstx=dstcol+0.5;
    double dsty=dstrow+0.5;
    int ready=1;
    if (sprite->x<dstx) {
      if ((sprite->x+=PRINCESS_SPEED*elapsed)>=dstx) sprite->x=dstx;
      else ready=0;
    } else if (sprite->x>dstx) {
      if ((sprite->x-=PRINCESS_SPEED*elapsed)<=dstx) sprite->x=dstx;
      else ready=0;
    }
    if (sprite->y<dsty) {
      if ((sprite->y+=PRINCESS_SPEED*elapsed)>=dsty) sprite->y=dsty;
      else ready=0;
    } else if (sprite->y>dsty) {
      if ((sprite->y-=PRINCESS_SPEED*elapsed)<=dsty) sprite->y=dsty;
      else ready=0;
    }
    if (ready) {
      SPRITE->pathp++;
      if (SPRITE->pathp<SPRITE->pathc) {
        int nextcol=SPRITE->pathv[SPRITE->pathp]%NS_sys_mapw;
        int nextrow=SPRITE->pathv[SPRITE->pathp]/NS_sys_mapw;
        if (nextrow>dstrow) {
          sprite->xform=0;
          SPRITE->tilecol=0;
        } else if (nextrow<dstrow) {
          sprite->xform=0;
          SPRITE->tilecol=1;
        } else if (nextcol<dstcol) {
          sprite->xform=0;
          SPRITE->tilecol=2;
        } else if (nextcol>dstcol) {
          sprite->xform=EGG_XFORM_XREV;
          SPRITE->tilecol=2;
        }
      }
    }
  }
  
  /* Prepare face.
   * If idle, we stick to the first frame and face toward the hero.
   * Otherwise we look where we're going, and animate.
   */
  if (SPRITE->idle) {
    if (g.hero) {
      double dx=g.hero->x-sprite->x; double adx=(dx<0.0)?-dx:dx;
      double dy=g.hero->y-sprite->y; double ady=(dy<0.0)?-dy:dy;
      if (adx>ady) {
        if (dx<0.0) {
          SPRITE->tilecol=2;
          sprite->xform=0;
        } else {
          SPRITE->tilecol=2;
          sprite->xform=EGG_XFORM_XREV;
        }
      } else {
        if (dy<0.0) {
          SPRITE->tilecol=1;
          sprite->xform=0;
        } else {
          SPRITE->tilecol=0;
          sprite->xform=0;
        }
      }
    }
    SPRITE->animclock=0.0;
    SPRITE->animframe=0;
    sprite->tileid=SPRITE->tileid0+SPRITE->tilecol;
  } else {
    switch (SPRITE->animframe) {
      case 0: sprite->tileid=SPRITE->tileid0+SPRITE->tilecol+0x00; break;
      case 1: sprite->tileid=SPRITE->tileid0+SPRITE->tilecol+0x10; break;
      case 2: sprite->tileid=SPRITE->tileid0+SPRITE->tilecol+0x00; break;
      case 3: sprite->tileid=SPRITE->tileid0+SPRITE->tilecol+0x20; break;
    }
  }
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_princess={
  .name="princess",
  .objlen=sizeof(struct sprite_princess),
  .del=_princess_del,
  .init=_princess_init,
  .update=_princess_update,
};
