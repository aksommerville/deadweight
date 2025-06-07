/* sprite_pepperfire.c
 * Little flame that locks to the hero and kills things nearby.
 * This is a replacement for "ssflame", it's a new NES-plausible implementation of the pepper.
 * Initialized at the desired position, and we'll maintain that position relative to the hero.
 */
 
#include "game/game.h"

#define PEPPERFIRE_TTL 1.000
#define PEPPERFIRE_RADIUS 0.999 /* square */

struct sprite_pepperfire {
  struct sprite hdr;
  double ttl;
  double dx,dy;
};

#define SPRITE ((struct sprite_pepperfire*)sprite)

static int _pepperfire_init(struct sprite *sprite) {
  if (!g.hero) {
    sprite->defunct=1;
    return 0;
  }
  sprite->decorative=1;
  sprite->airborne=1;
  sprite->layer=129;
  sprite->tileid=0x0e;
  SPRITE->ttl=PEPPERFIRE_TTL;
  SPRITE->dx=sprite->x-g.hero->x;
  SPRITE->dy=sprite->y-g.hero->y;
  return 0;
}

static void pepperfire_check_damage(struct sprite *sprite) {
  int i=g.spritec;
  while (i-->0) {
    struct sprite *victim=g.spritev[i];
    if (victim->defunct) continue;
    if (!victim->type->hurt) continue;
    if (victim==g.hero) continue;
    
    double dx=victim->x-sprite->x;
    if ((dx<-PEPPERFIRE_RADIUS)||(dx>PEPPERFIRE_RADIUS)) continue;
    double dy=victim->y-sprite->y;
    if ((dy<-PEPPERFIRE_RADIUS)||(dy>PEPPERFIRE_RADIUS)) continue;
    
    victim->type->hurt(victim,sprite);
  }
}

static void pepperfire_update_face(struct sprite *sprite) {
  // 0x0d,0x0e,0x0f,0x0e,0x0d. Each with and without XREV. So 10 frames total.
  int frame=(int)((SPRITE->ttl*10.0)/PEPPERFIRE_TTL);
  if (frame<0) frame=0; else if (frame>=10) frame=9;
  sprite->xform=(frame&1)?0:EGG_XFORM_XREV;
  switch (frame>>1) {
    case 0: sprite->tileid=0x0d; break;
    case 1: sprite->tileid=0x0e; break;
    case 2: sprite->tileid=0x0f; break;
    case 3: sprite->tileid=0x0e; break;
    case 4: sprite->tileid=0x0d; break;
  }
}

static void _pepperfire_update(struct sprite *sprite,double elapsed) {
  if (!g.hero) {
    sprite->defunct=1;
    return;
  }
  if ((SPRITE->ttl-=elapsed)<0.0) {
    sprite->defunct=1;
    return;
  }
  sprite->x=g.hero->x+SPRITE->dx;
  sprite->y=g.hero->y+SPRITE->dy;
  pepperfire_check_damage(sprite);
  pepperfire_update_face(sprite);
}

const struct sprite_type sprite_type_pepperfire={
  .name="pepperfire",
  .objlen=sizeof(struct sprite_pepperfire),
  .init=_pepperfire_init,
  .update=_pepperfire_update,
};
