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
  while (SPRITE->pathc<PRINCESS_PATH_LIMIT) {
    int dx=dotcol-sprcol;
    int dy=dotrow-sprrow;
    if ((dx>=-1)&&(dx<=1)&&(dy>=-1)&&(dy<=1)) break;
    //TODO Don't walk thru walls etc.
    int adx=(dx<0)?-dx:dx;
    int ady=(dy<0)?-dy:dy;
    if (adx>=ady) {
      if (dx<0) sprcol--;
      else sprcol++;
    } else {
      if (dy<0) sprrow--;
      else sprrow++;
    }
    if ((sprcol>=0)&&(sprcol<NS_sys_mapw)&&(sprrow>=0)&&(sprrow<NS_sys_maph)) {
      SPRITE->pathv[SPRITE->pathc++]=sprrow*NS_sys_mapw+sprcol;
    }
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
