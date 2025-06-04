/* sprite_ssflame.c
 * Single Serving Flame, a component of the Pepper's fire ring.
 * arg: u4.4:delay s
 */
 
#include "game/game.h"

#define SSFLAME_TTL 0.750 /* Not counting initial delay. */
#define SSFLAME_RADIUS 0.500 /* Square. */

struct sprite_ssflame {
  struct sprite hdr;
  double delay;
  double clock; // Starts counting up after (delay) runs down.
};

#define SPRITE ((struct sprite_ssflame*)sprite)

static int _ssflame_init(struct sprite *sprite) {
  sprite->tileid=0xff;
  SPRITE->delay=((sprite->arg>>24)&0xff)/16.0;
  return 0;
}

static void _ssflame_update(struct sprite *sprite,double elapsed) {
  if (g.time_stopped) return;
  if (SPRITE->delay>0.0) { // We're just "not here yet" until (delay) runs down.
    SPRITE->delay-=elapsed;
    return;
  }
  SPRITE->clock+=elapsed;
  if (SPRITE->clock>=SSFLAME_TTL) {
    sprite->defunct=1;
  } else if (SPRITE->clock>=SSFLAME_TTL*0.800) {
    sprite->tileid=0x0d;
  } else if (SPRITE->clock>=SSFLAME_TTL*0.600) {
    sprite->tileid=0x0e;
  } else if (SPRITE->clock>=SSFLAME_TTL*0.400) {
    sprite->tileid=0x0f;
  } else if (SPRITE->clock>=SSFLAME_TTL*0.200) {
    sprite->tileid=0x0e;
  } else {
    sprite->tileid=0x0d;
  }
  sprite->xform=(((int)(SPRITE->clock*10.0))&1)?0:EGG_XFORM_XREV;
  
  int i=g.spritec;
  while (i-->0) {
    struct sprite *victim=g.spritev[i];
    if (victim->defunct) continue;
    if (!victim->type->hurt) continue;
    double dx=victim->x-sprite->x;
    if ((dx<-SSFLAME_RADIUS)||(dx>SSFLAME_RADIUS)) continue;
    double dy=victim->y-sprite->y;
    if ((dy<-SSFLAME_RADIUS)||(dy>SSFLAME_RADIUS)) continue;
    victim->type->hurt(victim,sprite);
  }
}

const struct sprite_type sprite_type_ssflame={
  .name="ssflame",
  .objlen=sizeof(struct sprite_ssflame),
  .init=_ssflame_init,
  .update=_ssflame_update,
};
