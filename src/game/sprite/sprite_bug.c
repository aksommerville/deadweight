/* sprite_bug.c
 * Walks in a straight line until it decides to walk in some other straight line.
 * Touching is hazardous.
 */
 
#include "game/game.h"

#define BUG_SPEED 5.0
#define BUG_WAIT_TIME_MIN 0.250
#define BUG_WAIT_TIME_MAX 1.000

struct sprite_bug {
  struct sprite hdr;
  double animclock;
  int animframe;
  double dx,dy; // direction and speed
  double waitclock;
  uint8_t tileid0;
};

#define SPRITE ((struct sprite_bug*)sprite)

// Pick a new direction and start waiting, before travel.
static void bug_start_cycle(struct sprite *sprite) {
  switch (rand()&3) { // TODO Prefer directions with more freedom?
    case 0: SPRITE->dx=BUG_SPEED; SPRITE->dy=0.0; break;
    case 1: SPRITE->dx=-BUG_SPEED; SPRITE->dy=0.0; break;
    case 2: SPRITE->dx=0.0; SPRITE->dy=-BUG_SPEED; break;
    case 3: SPRITE->dx=0.0; SPRITE->dy=BUG_SPEED; break;
  }
  SPRITE->waitclock=BUG_WAIT_TIME_MIN+((rand()&0xffff)*(BUG_WAIT_TIME_MAX-BUG_WAIT_TIME_MIN))/65535.0;
}

static int _bug_init(struct sprite *sprite) {
  sprite->phl=-0.333;
  sprite->pht=-0.333;
  sprite->phr=0.333;
  sprite->phb=0.333;
  SPRITE->tileid0=sprite->tileid;
  bug_start_cycle(sprite);
  return 0;
}

static int bug_damagable(struct sprite *sprite,struct sprite *victim) {
  if (!victim) return 0;
  if (!victim->type->hurt) return 0;
  const double radius=0.750;
  double dx=victim->x-sprite->x;
  if ((dx<-radius)||(dx>radius)) return 0;
  double dy=victim->y-sprite->y;
  if ((dy<-radius)||(dy>radius)) return 0;
  return 1;
}

static void _bug_update(struct sprite *sprite,double elapsed) {
  if (g.time_stopped) return;
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
  }
  if (SPRITE->waitclock>0.0) {
    SPRITE->waitclock-=elapsed;
  } else {
  
    int candyp=find_candy(sprite);
    if (candyp>=0) {
      struct sprite *candy=g.candyv[candyp];
      if ((SPRITE->dx<0.0)&&(candy->x<sprite->x)) ;
      else if ((SPRITE->dx>0.0)&&(candy->x>sprite->x)) ;
      else if ((SPRITE->dy<0.0)&&(candy->y<sprite->y)) ;
      else if ((SPRITE->dy>0.0)&&(candy->y>sprite->y)) ;
      else {
        double dx=candy->x-sprite->x; double adx=(dx<0.0)?-dx:dx;
        double dy=candy->y-sprite->y; double ady=(dy<0.0)?-dy:dy;
        if ((adx<1.0)&&(ady<1.0)) {
          SPRITE->dx=SPRITE->dy=0.0;
        } else if (adx>=ady) {
          SPRITE->dx=(candy->x<sprite->x)?-BUG_SPEED:BUG_SPEED;
          SPRITE->dy=0.0;
        } else {
          SPRITE->dx=0.0;
          SPRITE->dy=(candy->y<sprite->y)?-BUG_SPEED:BUG_SPEED;
        }
      }
    } else if ((SPRITE->dx==0.0)&&(SPRITE->dy==0.0)) {
      bug_start_cycle(sprite);
    }
  
    if ( // Stay off the edge column and row (that's where the hero typically respawns if we kill her).
      ((sprite->x<1.5)&&(SPRITE->dx<0.0))||
      ((sprite->x>NS_sys_mapw-1.5)&&(SPRITE->dx>0.0))||
      ((sprite->y<1.5)&&(SPRITE->dy<0.0))||
      ((sprite->y>NS_sys_maph-1.5)&&(SPRITE->dy>0.0))||
      (sprite_move(sprite,SPRITE->dx*elapsed,SPRITE->dy*elapsed)<2)
    ) {
      bug_start_cycle(sprite);
    }
  }
  if (SPRITE->dx<0.0) sprite->xform=0;
  else if (SPRITE->dx>0.0) sprite->xform=EGG_XFORM_XREV|EGG_XFORM_YREV;
  else if (SPRITE->dy<0.0) sprite->xform=EGG_XFORM_SWAP|EGG_XFORM_YREV;
  else if (SPRITE->dy>0.0) sprite->xform=EGG_XFORM_XREV|EGG_XFORM_SWAP;
  sprite->tileid=SPRITE->tileid0+SPRITE->animframe;
  
  /* Deal damage to hero and princess only.
   * Mind that me and hero are both solid -- radius must be wide enough to account for that.
   */
  if (bug_damagable(sprite,g.hero)) g.hero->type->hurt(g.hero,sprite);
  if (bug_damagable(sprite,g.princess)) g.princess->type->hurt(g.princess,sprite);
}

static void _bug_hurt(struct sprite *sprite,struct sprite *assailant) {
  sprite->defunct=1;
  sprite_spawn(sprite->x,sprite->y,0,&sprite_type_soulballs,0x05000000);
}

const struct sprite_type sprite_type_bug={
  .name="bug",
  .objlen=sizeof(struct sprite_bug),
  .init=_bug_init,
  .update=_bug_update,
  .hurt=_bug_hurt,
};
