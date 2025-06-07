/* sprite_bossfire.c
 * Your average bonfire that spits a missile every so often.
 * When the boss is dead, we lose interest and stop with the missiles.
 */
 
#include "game/game.h"

#define BOSSFIRE_PERIOD_MIN 0.800
#define BOSSFIRE_PERIOD_MAX 2.500

struct sprite_bossfire {
  struct sprite hdr;
  int listenerid;
  uint8_t tileid0;
  double fireclock;
  double animclock;
  int animframe;
  int bossdead;
};

#define SPRITE ((struct sprite_bossfire*)sprite)

static void bossfire_cb_boss_dead(int k,int v,void *userdata) {
  struct sprite *sprite=userdata;
  SPRITE->bossdead=1;
}

static void _bossfire_del(struct sprite *sprite) {
  store_unlisten(SPRITE->listenerid);
}

static int _bossfire_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  if (store_get(NS_fld_boss_dead)) {
    SPRITE->bossdead=1;
  } else {
    SPRITE->listenerid=store_listen(NS_fld_boss_dead,bossfire_cb_boss_dead,sprite);
    SPRITE->fireclock=BOSSFIRE_PERIOD_MIN+((rand()&0xffff)*(BOSSFIRE_PERIOD_MAX-BOSSFIRE_PERIOD_MIN))/65535.0;
  }
  return 0;
}

static void bossfire_fire(struct sprite *sprite) {
  if (!g.hero) return;
  const double speed=5.0;
  double dx=g.hero->x-sprite->x;
  double dy=g.hero->y-sprite->y;
  if ((dx>-1.0)&&(dx<1.0)&&(dy>-1.0)&&(dy<1.0)) return; // dodge errors from small division, and also it's just rude at short distances.
  double distance=sqrt(dx*dx+dy*dy);
  dx/=distance;
  dy/=distance;
  struct sprite *missile=sprite_spawn(sprite->x,sprite->y,0,&sprite_type_missile,0);
  if (!missile) return;
  missile->tileid=0x0e;
  sprite_missile_setup(missile,dx*speed,dy*speed);
}

static void _bossfire_update(struct sprite *sprite,double elapsed) {
  if (g.time_stopped) return;
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.125;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
  switch (SPRITE->animframe) {
    case 0: sprite->tileid=SPRITE->tileid0+0; sprite->xform=0; break;
    case 1: sprite->tileid=SPRITE->tileid0+1; sprite->xform=0; break;
    case 2: sprite->tileid=SPRITE->tileid0+0; sprite->xform=EGG_XFORM_XREV; break;
    case 3: sprite->tileid=SPRITE->tileid0+1; sprite->xform=EGG_XFORM_XREV; break;
  }
  if (!SPRITE->bossdead) {
    if ((SPRITE->fireclock-=elapsed)<0.0) {
      SPRITE->fireclock=BOSSFIRE_PERIOD_MIN+((rand()&0xffff)*(BOSSFIRE_PERIOD_MAX-BOSSFIRE_PERIOD_MIN))/65535.0;
      bossfire_fire(sprite);
    }
  }
}

const struct sprite_type sprite_type_bossfire={
  .name="bossfire",
  .objlen=sizeof(struct sprite_bossfire),
  .del=_bossfire_del,
  .init=_bossfire_init,
  .update=_bossfire_update,
};
