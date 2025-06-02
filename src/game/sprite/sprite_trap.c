#include "game/game.h"

#define TRAP_PHASE_IDLE 0
#define TRAP_PHASE_ATTACK 1
#define TRAP_PHASE_RETREAT 2

#define TRAP_ATTACK_SPEED 12.0
#define TRAP_RETREAT_SPEED 6.0
#define TRAP_TRIGGER_RADIUS 1.250
#define TRAP_TOO_CLOSE 1.000 /* If target is so close on the off-axis, don't trigger. */

struct sprite_trap {
  struct sprite hdr;
  double restx,resty;
  double dx,dy; // direction of attack, even when in RETREAT phase.
  int phase;
};

#define SPRITE ((struct sprite_trap*)sprite)

static void _trap_del(struct sprite *sprite) {
}

static int _trap_init(struct sprite *sprite) {
  SPRITE->restx=sprite->x;
  SPRITE->resty=sprite->y;
  return 0;
}

static int trap_check_target(struct sprite *sprite,struct sprite *target) {
  if (!target) return 0;
  if ((target->x<0.0)||(target->y<0.0)||(target->x>NS_sys_mapw)||(target->y>NS_sys_maph)) return 0;
  double dx=target->x-sprite->x;
  double dy=target->y-sprite->y;
  if ((dx>-TRAP_TRIGGER_RADIUS)&&(dx<TRAP_TRIGGER_RADIUS)) {
    if (dy<-TRAP_TOO_CLOSE) {
      SPRITE->phase=TRAP_PHASE_ATTACK;
      SPRITE->dx=0.0;
      SPRITE->dy=-1.0;
      return 1;
    }
    if (dy>TRAP_TOO_CLOSE) {
      SPRITE->phase=TRAP_PHASE_ATTACK;
      SPRITE->dx=0.0;
      SPRITE->dy=1.0;
      return 1;
    }
  }
  if ((dy>=-TRAP_TRIGGER_RADIUS)&&(dy<=TRAP_TRIGGER_RADIUS)) {
    if (dx<-TRAP_TOO_CLOSE) {
      SPRITE->phase=TRAP_PHASE_ATTACK;
      SPRITE->dx=-1.0;
      SPRITE->dy=0.0;
      return 1;
    }
    if (dx>TRAP_TOO_CLOSE) {
      SPRITE->phase=TRAP_PHASE_ATTACK;
      SPRITE->dx=1.0;
      SPRITE->dy=0.0;
      return 1;
    }
  }
  return 0;
}

static void trap_check_targets(struct sprite *sprite) {
  if (trap_check_target(sprite,g.hero)) return;
  trap_check_target(sprite,g.princess);
}

static int trap_closed(struct sprite *sprite) {
  if ((sprite->x<0.0)||(sprite->y<0.0)||(sprite->x>NS_sys_mapw)||(sprite->y>NS_sys_maph)) return 1;
  int col=(int)sprite->x;
  int row=(int)sprite->y;
  if ((col>=0)&&(row>=0)&&(col<NS_sys_mapw)&&(row<NS_sys_maph)) {
    uint8_t physics=g.physics[g.map->cellv[row*NS_sys_mapw+col]];
    if ((physics==NS_physics_solid)||(physics==NS_physics_hole)) return 1;
  }
  struct sprite **p=g.spritev;
  int i=g.spritec;
  for (;i-->0;p++) {
    struct sprite *other=*p;
    if (other==sprite) continue;
    if (other->defunct) continue;
    if (other->type!=&sprite_type_trap) continue;
    double dx=other->x-sprite->x;
    if ((dx<-1.0)||(dx>1.0)) continue;
    double dy=other->y-sprite->y;
    if ((dy<-1.0)||(dy>1.0)) continue;
    return 1;
  }
  return 0;
}

static void _trap_update(struct sprite *sprite,double elapsed) {
  if (g.time_stopped) return;
  switch (SPRITE->phase) {
    case TRAP_PHASE_IDLE: trap_check_targets(sprite); break;
    case TRAP_PHASE_ATTACK: {
        sprite->x+=SPRITE->dx*TRAP_ATTACK_SPEED*elapsed;
        sprite->y+=SPRITE->dy*TRAP_ATTACK_SPEED*elapsed;
        if (trap_closed(sprite)) {
          SPRITE->phase=TRAP_PHASE_RETREAT;
        }
      } break;
    case TRAP_PHASE_RETREAT: {
        sprite->x-=SPRITE->dx*TRAP_RETREAT_SPEED*elapsed;
        sprite->y-=SPRITE->dy*TRAP_RETREAT_SPEED*elapsed;
        if (
          ((SPRITE->dx<0.0)&&(sprite->x>=SPRITE->restx))||
          ((SPRITE->dx>0.0)&&(sprite->x<=SPRITE->restx))||
          ((SPRITE->dy<0.0)&&(sprite->y>=SPRITE->resty))||
          ((SPRITE->dy>0.0)&&(sprite->y<=SPRITE->resty))
        ) {
          SPRITE->phase=TRAP_PHASE_IDLE;
          sprite->x=SPRITE->restx;
          sprite->y=SPRITE->resty;
        }
      } break;
  }
}

const struct sprite_type sprite_type_trap={
  .name="trap",
  .objlen=sizeof(struct sprite_trap),
  .del=_trap_del,
  .init=_trap_init,
  .update=_trap_update,
};
