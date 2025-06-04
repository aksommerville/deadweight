/* sprite_smoke.c
 * Decorative, created by sprite_explode and sprite_tank.
 */
 
#include "game/game.h"

#define SMOKE_TTL 1.500

struct sprite_smoke {
  struct sprite hdr;
  double dx,dy;
  double ttl;
};

#define SPRITE ((struct sprite_smoke*)sprite)

static int _smoke_init(struct sprite *sprite) {
  sprite->layer=189;
  sprite->airborne=1;
  sprite->decorative=1;
  SPRITE->ttl=SMOKE_TTL;
  sprite->tileid=0x63;
  return 0;
}

static void _smoke_update(struct sprite *sprite,double elapsed) {
  sprite->x+=SPRITE->dx*elapsed;
  sprite->y+=SPRITE->dy*elapsed;
  if ((SPRITE->ttl-=elapsed)<=0.0) {
    sprite->defunct=1;
  } else if (SPRITE->ttl<0.250) {
    sprite->tileid=0x66;
  } else if (SPRITE->ttl<0.500) {
    sprite->tileid=0x65;
  } else if (SPRITE->ttl<0.750) {
    sprite->tileid=0x64;
  } else {
    sprite->tileid=0x63;
  }
}

const struct sprite_type sprite_type_smoke={
  .name="smoke",
  .objlen=sizeof(struct sprite_smoke),
  .init=_smoke_init,
  .update=_smoke_update,
};

void sprite_smoke_setup(struct sprite *sprite,double dx,double dy) {
  if (!sprite||(sprite->type!=&sprite_type_smoke)) return;
  SPRITE->dx=dx;
  SPRITE->dy=dy;
}

void sprite_smoke_quickie(struct sprite *sprite) {
  if (!sprite||(sprite->type!=&sprite_type_smoke)) return;
  SPRITE->ttl=1.000;
}
