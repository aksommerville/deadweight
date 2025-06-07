#include "game/game.h"

#define BUBBLE_RADIUS 0.625

struct sprite_bubble {
  struct sprite hdr;
  double dx,dy;
  double animclock;
  int animframe;
  double warmup;
};

#define SPRITE ((struct sprite_bubble*)sprite)

static void _bubble_del(struct sprite *sprite) {
}

static int _bubble_init(struct sprite *sprite) {
  sprite->airborne=1;
  sprite->tileid=0x8a;
  if (sprite->arg==1) SPRITE->warmup=0.001;
  else SPRITE->warmup=0.500;
  
  /* Delete self if there's no target.
   * Otherwise ignore the target; we'll establish the travel vector after the warmup expires.
   */
  struct sprite *target=get_preferred_monster_target();
  if (!target) return -1;
  return 0;
}

static void bubble_set_travel_vector(struct sprite *sprite) {
  struct sprite *target=get_preferred_monster_target();
  if (!target) {
    SPRITE->dx=0.0;
    SPRITE->dy=5.0;
    return;
  }
  SPRITE->dx=target->x-sprite->x;
  SPRITE->dy=target->y-sprite->y;
  if ((SPRITE->dx>=-0.010)&&(SPRITE->dx<=0.010)&&(SPRITE->dy>=-0.010)&&(SPRITE->dy<=0.010)) {
    // If the target is too close, pick a random direction.
    switch (rand()&7) {
      case 0: SPRITE->dx=-1.0; SPRITE->dy=-1.0; break;
      case 1: SPRITE->dx=-1.0; SPRITE->dy= 0.0; break;
      case 2: SPRITE->dx=-1.0; SPRITE->dy= 1.0; break;
      case 3: SPRITE->dx= 0.0; SPRITE->dy=-1.0; break;
      case 4: SPRITE->dx= 0.0; SPRITE->dy= 1.0; break;
      case 5: SPRITE->dx= 1.0; SPRITE->dy=-1.0; break;
      case 6: SPRITE->dx= 1.0; SPRITE->dy= 0.0; break;
      case 7: SPRITE->dx= 1.0; SPRITE->dy= 1.0; break;
    }
  }
  double distance=sqrt(SPRITE->dx*SPRITE->dx+SPRITE->dy*SPRITE->dy);
  SPRITE->dx/=distance;
  SPRITE->dy/=distance;
  SPRITE->dx*=5.0; // travel speed, m/s
  SPRITE->dy*=5.0;
}

static void bubble_check_damage(struct sprite *sprite) {
  int i=g.spritec;
  while (i-->0) {
    struct sprite *victim=g.spritev[i];
    if (victim->defunct) continue;
    if (!victim->type->hurt) continue;
    
    if (victim->type==&sprite_type_bubblesaur) continue;
    
    double dx=victim->x-sprite->x;
    if ((dx<-BUBBLE_RADIUS)||(dx>BUBBLE_RADIUS)) continue;
    double dy=victim->y-sprite->y;
    if ((dy<-BUBBLE_RADIUS)||(dy>BUBBLE_RADIUS)) continue;
    
    if (victim->type->hurt(victim,sprite)) {
      sprite->defunct=1;
      return;
    }
  }
}

static void _bubble_update(struct sprite *sprite,double elapsed) {
  if (g.time_stopped||sprite->summoning) {
    // When stopped, we're still deadly.
    // Also stop moving when summoned. Don't imagine it will come up often, but it's pretty cool to catch bubbles midair.
    bubble_check_damage(sprite);
    return;
  }
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.250;
    if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
    sprite->tileid=0x8a+SPRITE->animframe;
  }
  if (SPRITE->warmup>0.0) {
    if ((SPRITE->warmup-=elapsed)<=0.0) {
      bubble_set_travel_vector(sprite);
    }
  } else {
    sprite->x+=SPRITE->dx*elapsed;
    sprite->y+=SPRITE->dy*elapsed;
    bubble_check_damage(sprite);
    if ((sprite->x<-1.0)||(sprite->y<-1.0)||(sprite->x>NS_sys_mapw+1.0)||(sprite->y>NS_sys_maph+1.0)) {
      sprite->defunct=1;
    }
  }
}

const struct sprite_type sprite_type_bubble={
  .name="bubble",
  .objlen=sizeof(struct sprite_bubble),
  .del=_bubble_del,
  .init=_bubble_init,
  .update=_bubble_update,
};
