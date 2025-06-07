/* sprite_tank.c
 * Roams the map, staying aligned to the grid, and fires baseballs occasionally.
 * Can't be tempted by candy.
 */
 
#include "game/game.h"

#define TANK_PHASE_HOLD 0 /* After a step or shot, stay here for a sec. */
#define TANK_PHASE_WAIT 1 /* Facing the new direction, wait just a little. */
#define TANK_PHASE_MOVE 2 /* Move in progress. */

#define TANK_HOLD_TIME_MIN 0.125
#define TANK_HOLD_TIME_MAX 0.250
#define TANK_WAIT_TIME_MIN 0.125
#define TANK_WAIT_TIME_MAX 0.250
#define TANK_MOVE_SPEED 6.0
#define TANK_MISSILE_SPEED 10.0

struct sprite_tank {
  struct sprite hdr;
  uint8_t tileid0;
  double animclock;
  int animframe;
  int phase;
  double phaseclock;
  int col,row;
  double dx,dy; // Speed baked in.
  double dstx,dsty; // In WAIT or MOVE, where we're going.
  int dstcol,dstrow; // ''
  int fired;
};

#define SPRITE ((struct sprite_tank*)sprite)

static void tank_fire(struct sprite *sprite) {
  double dx=0.0,dy=0.0;
  switch (sprite->xform&(EGG_XFORM_SWAP|EGG_XFORM_XREV)) { // Ignore all else, the direction we fire is the direction we're displaying.
    case 0: dx=-1.0; break;
    case EGG_XFORM_SWAP: dy=-1.0; break;
    case EGG_XFORM_XREV: dx=1.0; break;
    case EGG_XFORM_SWAP|EGG_XFORM_XREV: dy=1.0; break;
  }
  double x=sprite->x+dx*0.750;
  double y=sprite->y+dy*0.750;
  egg_play_sound(RID_sound_tank);
  struct sprite *smoke=sprite_spawn(x,y,0,&sprite_type_smoke,0);
  if (smoke) sprite_smoke_quickie(smoke);
  struct sprite *baseball=sprite_spawn(x,y,0,&sprite_type_missile,0);
  if (baseball) {
    baseball->tileid=SPRITE->tileid0+4;
    sprite_missile_setup(baseball,dx*TANK_MISSILE_SPEED,dy*TANK_MISSILE_SPEED);
  }
}

static int tank_passable(int x,int y) {
  if (x<=1) return 0;
  if (y<=1) return 0;
  if (x>=NS_sys_mapw-1) return 0;
  if (y>=NS_sys_maph-1) return 0;
  uint8_t physics=g.physics[g.map->cellv[y*NS_sys_mapw+x]];
  return (physics==NS_physics_vacant);
}

// Pick a new direction and enter WAIT state.
static void tank_begin_WAIT(struct sprite *sprite) {
  SPRITE->col=(int)sprite->x;
  SPRITE->row=(int)sprite->y;
  uint8_t candidatev[5]; // direction (0x40,0x10,0x08,0x02) or 0 for fire.
  int candidatec=0;
  if (SPRITE->fired) SPRITE->fired=0;
  else candidatev[candidatec++]=0;
  if (g.map&&(SPRITE->col>=0)&&(SPRITE->row>=0)&&(SPRITE->col<NS_sys_mapw)&&(SPRITE->row<NS_sys_maph)) {
    if (tank_passable(SPRITE->col-1,SPRITE->row)) candidatev[candidatec++]=0x10;
    if (tank_passable(SPRITE->col+1,SPRITE->row)) candidatev[candidatec++]=0x08;
    if (tank_passable(SPRITE->col,SPRITE->row-1)) candidatev[candidatec++]=0x40;
    if (tank_passable(SPRITE->col,SPRITE->row+1)) candidatev[candidatec++]=0x02;
  }
  SPRITE->dstx=sprite->x;
  SPRITE->dsty=sprite->y;
  SPRITE->dstcol=SPRITE->col;
  SPRITE->dstrow=SPRITE->row;
  SPRITE->dx=0.0;
  SPRITE->dy=0.0;
  if (candidatec<1) {
    sprite->defunct=1;
  } else {
    int choice=rand()%candidatec;
    switch (candidatev[choice]) {
      case 0x00: {
          SPRITE->fired=1;
          SPRITE->phase=TANK_PHASE_HOLD;
          SPRITE->phaseclock=TANK_HOLD_TIME_MIN+((rand()&0xffff)*(TANK_HOLD_TIME_MAX-TANK_HOLD_TIME_MIN))/65535.0;
          tank_fire(sprite);
        } return;
      case 0x40: {
          SPRITE->dy=-1.0;
          SPRITE->dsty-=1.0;
          SPRITE->dstrow-=1;
          sprite->xform=EGG_XFORM_SWAP;
        } break;
      case 0x10: {
          SPRITE->dx=-1.0;
          SPRITE->dstx-=1.0;
          SPRITE->dstcol-=1;
          sprite->xform=0;
        } break;
      case 0x08: {
          SPRITE->dx=1.0;
          SPRITE->dstx+=1.0;
          SPRITE->dstcol+=1;
          sprite->xform=EGG_XFORM_XREV;
        } break;
      case 0x02: {
          SPRITE->dy=1.0;
          SPRITE->dsty+=1.0;
          SPRITE->dstrow+=1;
          sprite->xform=EGG_XFORM_SWAP|EGG_XFORM_XREV;
        } break;
    }
    SPRITE->phase=TANK_PHASE_WAIT;
    SPRITE->phaseclock=TANK_WAIT_TIME_MIN+((rand()&0xffff)*(TANK_WAIT_TIME_MAX-TANK_WAIT_TIME_MIN))/65535.0;
  }
}

static int _tank_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  SPRITE->col=(int)sprite->x;
  SPRITE->row=(int)sprite->y;
  SPRITE->fired=1; // Don't let it fire as the first play, give the player a chance!
  tank_begin_WAIT(sprite);
  return 0;
}

static int tank_damagable(struct sprite *sprite,struct sprite *victim) {
  if (!victim) return 0;
  if (!victim->type->hurt) return 0;
  const double radius=0.750;
  double dx=victim->x-sprite->x;
  if ((dx<-radius)||(dx>radius)) return 0;
  double dy=victim->y-sprite->y;
  if ((dy<-radius)||(dy>radius)) return 0;
  return 1;
}

static void _tank_update(struct sprite *sprite,double elapsed) {

  if (tank_damagable(sprite,g.hero)) g.hero->type->hurt(g.hero,sprite);
  if (tank_damagable(sprite,g.princess)) g.princess->type->hurt(g.princess,sprite);

  if (g.time_stopped) return;
  
  if ((SPRITE->phaseclock-=elapsed)<=0.0) {
    switch (SPRITE->phase) {
      case TANK_PHASE_WAIT: {
          SPRITE->phase=TANK_PHASE_MOVE;
          SPRITE->phaseclock=5.0;
        } break;
      case TANK_PHASE_HOLD: {
          tank_begin_WAIT(sprite);
        } break;
      case TANK_PHASE_MOVE: { // Shouldn't happen, but can if we got magic-wanded or something.
          SPRITE->col=(int)sprite->x;
          SPRITE->row=(int)sprite->y;
          sprite->x=SPRITE->col+0.5;
          sprite->y=SPRITE->row+0.5;
          tank_begin_WAIT(sprite);
        } break;
    }
  }
  
  if (SPRITE->phase==TANK_PHASE_MOVE) {
    if ((SPRITE->animclock-=elapsed)<0.0) {
      SPRITE->animclock+=0.080;
      if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
      sprite->tileid=SPRITE->tileid0+SPRITE->animframe;
    }
    sprite->x+=elapsed*SPRITE->dx;
    sprite->y+=elapsed*SPRITE->dy;
    if (
      ((SPRITE->dx<0.0)&&(sprite->x<=SPRITE->dstx))||
      ((SPRITE->dx>0.0)&&(sprite->x>=SPRITE->dstx))||
      ((SPRITE->dy<0.0)&&(sprite->y<=SPRITE->dsty))||
      ((SPRITE->dy>0.0)&&(sprite->y>=SPRITE->dsty))
    ) {
      SPRITE->col=(int)sprite->x;
      SPRITE->row=(int)sprite->y;
      sprite->x=SPRITE->col+0.5;
      sprite->y=SPRITE->row+0.5;
      SPRITE->phase=TANK_PHASE_HOLD;
      SPRITE->phaseclock=TANK_HOLD_TIME_MIN+((rand()&0xffff)*(TANK_HOLD_TIME_MAX-TANK_HOLD_TIME_MIN))/65535.0;
    }
  }
}

static int _tank_hurt(struct sprite *sprite,struct sprite *assailant) {
  if ((assailant->type==&sprite_type_explode)||(assailant->type==&sprite_type_pepperfire)) {
    egg_play_sound(RID_sound_hurt_monster);
    sprite->defunct=1;
    sprite_spawn(sprite->x,sprite->y,0,&sprite_type_soulballs,0x03000000);
    spawn_prize(sprite->x,sprite->y);
    return 1;
  }
  return 0;
}

const struct sprite_type sprite_type_tank={
  .name="tank",
  .objlen=sizeof(struct sprite_tank),
  .init=_tank_init,
  .update=_tank_update,
  .hurt=_tank_hurt,
};
