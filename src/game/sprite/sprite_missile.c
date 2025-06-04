/* sprite_missile.c
 * Generic fixed-course projectile.
 */
 
#include "game/game.h"

struct sprite_missile {
  struct sprite hdr;
  double dx,dy;
};

#define SPRITE ((struct sprite_missile*)sprite)

static int _missile_init(struct sprite *sprite) {
  sprite->airborne=1;
  sprite->tileid=0x72; // caller should replace; there isn't a sensible default.
  SPRITE->dx=0.0;
  SPRITE->dy=1.0; // don't let it be zero
  return 0;
}

static int missile_check_damage(struct sprite *sprite,struct sprite *victim) {
  if (!victim) return 0;
  if (!victim->type->hurt) return 0;
  double dx=victim->x-sprite->x;
  double dy=victim->y-sprite->y;
  const double radius=0.250;
  if ((dx<-radius)||(dx>radius)||(dy<-radius)||(dy>radius)) return 0;
  if (victim->type->hurt(victim,sprite)) {
    sprite->defunct=1;
    return 1;
  }
  return 0;
}

static void _missile_update(struct sprite *sprite,double elapsed) {
  sprite->x+=SPRITE->dx*elapsed;
  sprite->y+=SPRITE->dy*elapsed;
  if ((sprite->x<-1.0)||(sprite->y<-1.0)||(sprite->x>NS_sys_mapw+1.0)||(sprite->y>NS_sys_maph+1.0)) {
    sprite->defunct=1;
  } else {
    if (missile_check_damage(sprite,g.hero)) return;
    if (missile_check_damage(sprite,g.princess)) return;
    if (g.map) {
      int x=(int)sprite->x;
      int y=(int)sprite->y;
      if ((x>=0)&&(x<NS_sys_mapw)&&(y>=0)&&(y<NS_sys_maph)&&(g.physics[g.map->cellv[y*NS_sys_mapw+x]]==NS_physics_solid)) {
        sprite->defunct=1;
      }
    }
  }
}

const struct sprite_type sprite_type_missile={
  .name="missile",
  .objlen=sizeof(struct sprite_missile),
  .init=_missile_init,
  .update=_missile_update,
};

void sprite_missile_setup(struct sprite *sprite,double dx,double dy) {
  if (!sprite||(sprite->type!=&sprite_type_missile)) return;
  SPRITE->dx=dx;
  SPRITE->dy=dy;
}
