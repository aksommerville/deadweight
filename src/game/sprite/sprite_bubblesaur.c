#include "game/game.h"

#define BUBBLESAUR_BLOW_TIME 2.500
#define BUBBLESAUR_RESET_TIME 3.000
#define BUBBLESAUR_WALK_SPEED 2.0

struct sprite_bubblesaur {
  struct sprite hdr;
  uint8_t tileid0;
  double clock; // counts up, resets each blow cycle
  int blown;
  int candyp; // 0..g.candyc-1 if we're distracted; refreshes each update.
  double animclock; // Only when candystracted
  int animframe;
};

#define SPRITE ((struct sprite_bubblesaur*)sprite)

static void _bubblesaur_del(struct sprite *sprite) {
}

static int _bubblesaur_init(struct sprite *sprite) {
  sprite->solid=1;
  SPRITE->tileid0=sprite->tileid;
  SPRITE->candyp=-1;
  return 0;
}

static void bubblesaur_approach_candy(struct sprite *sprite,const struct sprite *candy,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.250;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
  if (sprite->summoning) return;
  double dx=candy->x-sprite->x;
  double dy=candy->y-sprite->y;
  double distance=sqrt(dx*dx+dy*dy);
  if (distance<=1.0) return;
  sprite_move(sprite,
    (dx*BUBBLESAUR_WALK_SPEED*elapsed)/distance,
    (dy*BUBBLESAUR_WALK_SPEED*elapsed)/distance
  );
  if (dx<0) sprite->xform=0;
  else sprite->xform=EGG_XFORM_XREV;
}

static void _bubblesaur_update(struct sprite *sprite,double elapsed) {
  SPRITE->candyp=-1;

  // Time stopped, do nothing.
  if (g.time_stopped) return;
  
  // Candy present, walk toward the nearest.
  if ((SPRITE->candyp=find_candy(sprite))>=0) {
    SPRITE->clock=0.0;
    bubblesaur_approach_candy(sprite,g.candyv[SPRITE->candyp],elapsed);
    return;
  }
  
  // Normal operation: Blow bubbles.
  SPRITE->clock+=elapsed;
  if (SPRITE->clock<BUBBLESAUR_BLOW_TIME) {
    sprite->tileid=SPRITE->tileid0;
  } else if (SPRITE->clock>BUBBLESAUR_RESET_TIME) {
    SPRITE->blown=0;
    SPRITE->clock=0.0;
    sprite->tileid=SPRITE->tileid0;
  } else {
    sprite->tileid=SPRITE->tileid0+1;
    if (!SPRITE->blown) {
      SPRITE->blown=1;
      double x=sprite->x;
      if (sprite->xform&EGG_XFORM_XREV) x+=1.0; else x-=1.0;
      struct sprite *bubble=sprite_spawn(x,sprite->y,0,&sprite_type_bubble,0);
    }
  }
}

static void _bubblesaur_render(struct sprite *sprite,int x,int y) {
  int eyedx=0,eyedy=0;
  struct sprite *target=0;
  if (SPRITE->candyp>=0) target=g.candyv[SPRITE->candyp];
  else get_preferred_monster_target();
  if (target) {
    double dx=target->x-sprite->x;
    double dy=target->y-sprite->y;
    if (dx<-0.5) {
      eyedx=-1;
      if (!SPRITE->blown) sprite->xform=0;
    } else if (dx>0.5) {
      eyedx=1;
      if (!SPRITE->blown) sprite->xform=EGG_XFORM_XREV;
    }
    if (dy<-0.5) eyedy=-1; else if (dy>0.5) eyedy=1;
  }
  uint8_t tileid=sprite->tileid;
  if (SPRITE->candyp>=0) {
    tileid=SPRITE->tileid0+(SPRITE->animframe?0x12:0x13); // drew bubblesaur before adding candystraction, so they aren't contiguous
  }
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,tileid,sprite->xform);
  graf_draw_tile(&g.graf,g.texid_sprites,x+eyedx,y+eyedy,0x80,EGG_XFORM_SWAP);
}

static void _bubblesaur_hurt(struct sprite *sprite,struct sprite *assailant) {
  sprite->defunct=1;
  sprite_spawn(sprite->x,sprite->y,0,&sprite_type_soulballs,0x05000000);
}

const struct sprite_type sprite_type_bubblesaur={
  .name="bubblesaur",
  .objlen=sizeof(struct sprite_bubblesaur),
  .del=_bubblesaur_del,
  .init=_bubblesaur_init,
  .update=_bubblesaur_update,
  .render=_bubblesaur_render,
  .hurt=_bubblesaur_hurt,
};
