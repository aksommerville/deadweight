/* sprite_caterpillar.c
 * Similar to caterpillar, but slow heterogenous movement normally, and charges when you enter its line of sight.
 * Touching is hazardous.
 */
 
#include "game/game.h"

#define CAT_PHASE_CONTRACT 0
#define CAT_PHASE_EXPAND 1
#define CAT_PHASE_BREATHE 2
#define CAT_PHASE_CHARGE 3

#define CAT_CONTRACT_TIME 0.500
#define CAT_CONTRACT_SPEED 0.750
#define CAT_EXPAND_TIME 0.500
#define CAT_EXPAND_SPEED 1.250
#define CAT_BREATHE_TIME_MIN 0.125
#define CAT_BREATHE_TIME_MAX 0.500
#define CAT_CHARGE_SPEED 8.0

struct sprite_caterpillar {
  struct sprite hdr;
  uint8_t tileid0;
  int phase;
  double phaseclock;
  double dx,dy;
};

#define SPRITE ((struct sprite_caterpillar*)sprite)

// Pick a new direction and start travelling.
static void caterpillar_start_cycle(struct sprite *sprite) {
  SPRITE->dx=SPRITE->dy=0.0;

  // If there's candy, move in that direction or chill.
  int candyp=find_candy(sprite);
  if (candyp>=0) {
    struct sprite *candy=g.candyv[candyp];
    double dx=candy->x-sprite->x;
    double dy=candy->y-sprite->y;
         if (dx<-1.000) SPRITE->dx=-1.0;
    else if (dx>1.000) SPRITE->dx=1.0;
    else if (dy<-1.000) SPRITE->dy=-1.0;
    else if (dy>1.000) SPRITE->dy=1.0;
    else {
      SPRITE->phase=CAT_PHASE_BREATHE;
      SPRITE->phaseclock=3.0;
      return;
    }
    
  // Normally, just random.
  } else {
    switch (rand()&3) {
      case 0: SPRITE->dx=-1.0; break;
      case 1: SPRITE->dx=1.0; break;
      case 2: SPRITE->dy=-1.0; break;
      case 3: SPRITE->dy=1.0; break;
    }
  }
  SPRITE->phase=CAT_PHASE_CONTRACT;
  SPRITE->phaseclock=CAT_CONTRACT_TIME;
}

static int _caterpillar_init(struct sprite *sprite) {
  sprite->phl=-0.333;
  sprite->pht=-0.333;
  sprite->phr=0.333;
  sprite->phb=0.333;
  SPRITE->tileid0=sprite->tileid;
  caterpillar_start_cycle(sprite);
  return 0;
}

static int caterpillar_damagable(struct sprite *sprite,struct sprite *victim) {
  if (!victim) return 0;
  if (!victim->type->hurt) return 0;
  const double radius=0.750;
  double dx=victim->x-sprite->x;
  if ((dx<-radius)||(dx>radius)) return 0;
  double dy=victim->y-sprite->y;
  if ((dy<-radius)||(dy>radius)) return 0;
  return 1;
}

static void caterpillar_next_phase(struct sprite *sprite) {
  switch (SPRITE->phase) {
    case CAT_PHASE_CONTRACT: {
        SPRITE->phase=CAT_PHASE_EXPAND;
        SPRITE->phaseclock=CAT_EXPAND_TIME;
      } break;
    case CAT_PHASE_EXPAND: {
        SPRITE->phase=CAT_PHASE_BREATHE;
        SPRITE->phaseclock=CAT_BREATHE_TIME_MIN+((rand()&0xffff)*(CAT_BREATHE_TIME_MAX-CAT_BREATHE_TIME_MIN))/65535.0;
      } break;
    case CAT_PHASE_BREATHE:
    default: {
        caterpillar_start_cycle(sprite);
      } break;
  }
}

static void caterpillar_check_charge(struct sprite *sprite) {
  const double radius=0.750;
  double l=sprite->x-radius;
  double r=sprite->x+radius;
  double t=sprite->y-radius;
  double b=sprite->y+radius;
  if (SPRITE->dx<0.0) l=0.0;
  else if (SPRITE->dx>0.0) r=NS_sys_mapw;
  else if (SPRITE->dy<0.0) t=0.0;
  else if (SPRITE->dy>0.0) b=NS_sys_maph;
  else return;
  if (
    (g.hero&&(g.hero->x>=l)&&(g.hero->x<=r)&&(g.hero->y>=t)&&(g.hero->y<=b))||
    (g.princess&&(g.princess->x>=l)&&(g.princess->x<=r)&&(g.princess->y>=t)&&(g.princess->y<=b))
  ) {
    SPRITE->phase=CAT_PHASE_CHARGE;
    SPRITE->phaseclock=5.0;
  }
}

static void _caterpillar_update(struct sprite *sprite,double elapsed) {
  if (g.time_stopped) return;
  
  if ((SPRITE->phaseclock-=elapsed)<0.0) {
    caterpillar_next_phase(sprite);
  }
  
  double speed=0.0;
  switch (SPRITE->phase) {
    case CAT_PHASE_CONTRACT: if (SPRITE->phaseclock<CAT_CONTRACT_TIME*0.750) speed=CAT_CONTRACT_SPEED; caterpillar_check_charge(sprite); break;
    case CAT_PHASE_EXPAND: if (SPRITE->phaseclock<CAT_EXPAND_TIME*0.750) speed=CAT_EXPAND_SPEED; caterpillar_check_charge(sprite); break;
    case CAT_PHASE_BREATHE: caterpillar_check_charge(sprite); break;
    case CAT_PHASE_CHARGE: speed=CAT_CHARGE_SPEED; break;
  }
  
  if (speed>0.0) {
    if ( // Stay off the edge column and row (that's where the hero typically respawns if we kill her).
      ((sprite->x<1.5)&&(SPRITE->dx<0.0))||
      ((sprite->x>NS_sys_mapw-1.5)&&(SPRITE->dx>0.0))||
      ((sprite->y<1.5)&&(SPRITE->dy<0.0))||
      ((sprite->y>NS_sys_maph-1.5)&&(SPRITE->dy>0.0))||
      (sprite_move(sprite,SPRITE->dx*elapsed*speed,SPRITE->dy*elapsed*speed)<2)
    ) {
      caterpillar_start_cycle(sprite);
    }
  }
  
  if (SPRITE->dx<0.0) sprite->xform=0;
  else if (SPRITE->dx>0.0) sprite->xform=EGG_XFORM_XREV|EGG_XFORM_YREV;
  else if (SPRITE->dy<0.0) sprite->xform=EGG_XFORM_SWAP|EGG_XFORM_YREV;
  else if (SPRITE->dy>0.0) sprite->xform=EGG_XFORM_XREV|EGG_XFORM_SWAP;
  
  switch (SPRITE->phase) {
    case CAT_PHASE_CONTRACT: {
        int frame=2-(int)(SPRITE->phaseclock*3.0)/CAT_CONTRACT_TIME;
        if (frame<0) frame=0; else if (frame>2) frame=2;
        sprite->tileid=SPRITE->tileid0+frame;
      } break;
    case CAT_PHASE_EXPAND: {
        int frame=(int)(SPRITE->phaseclock*3.0)/CAT_EXPAND_TIME;
        if (frame<0) frame=0; else if (frame>2) frame=2;
        sprite->tileid=SPRITE->tileid0+frame;
      } break;
    case CAT_PHASE_BREATHE: {
        sprite->tileid=SPRITE->tileid0;
      } break;
    case CAT_PHASE_CHARGE: {
        sprite->tileid=SPRITE->tileid0+3;
      } break;
  }
  
  /* Deal damage to hero and princess only.
   * Mind that me and hero are both solid -- radius must be wide enough to account for that.
   */
  if (caterpillar_damagable(sprite,g.hero)) g.hero->type->hurt(g.hero,sprite);
  if (caterpillar_damagable(sprite,g.princess)) g.princess->type->hurt(g.princess,sprite);
}

static int _caterpillar_hurt(struct sprite *sprite,struct sprite *assailant) {
  if (assailant->type==&sprite_type_bubble) return 0;
  egg_play_sound(RID_sound_hurt_monster);
  sprite->defunct=1;
  sprite_spawn(sprite->x,sprite->y,0,&sprite_type_soulballs,0x05000000);
  spawn_prize(sprite->x,sprite->y);
  return 1;
}

const struct sprite_type sprite_type_caterpillar={
  .name="caterpillar",
  .objlen=sizeof(struct sprite_caterpillar),
  .init=_caterpillar_init,
  .update=_caterpillar_update,
  .hurt=_caterpillar_hurt,
};
