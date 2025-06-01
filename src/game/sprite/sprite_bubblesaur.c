#include "game/game.h"

#define BUBBLESAUR_BLOW_TIME 2.500
#define BUBBLESAUR_RESET_TIME 3.000

struct sprite_bubblesaur {
  struct sprite hdr;
  uint8_t tileid0;
  double clock; // counts up, resets each blow cycle
  int blown;
};

#define SPRITE ((struct sprite_bubblesaur*)sprite)

static void _bubblesaur_del(struct sprite *sprite) {
}

static int _bubblesaur_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  return 0;
}

static void _bubblesaur_update(struct sprite *sprite,double elapsed) {
  if (g.time_stopped) return;
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
  struct sprite *target=get_preferred_monster_target();
  if (target) {
    double dx=target->x-sprite->x;
    double dy=target->y-sprite->y;
    if (dx<-1.0) {
      eyedx=-1;
      if (!SPRITE->blown) sprite->xform=0;
    } else if (dx>1.0) {
      eyedx=1;
      if (!SPRITE->blown) sprite->xform=EGG_XFORM_XREV;
    }
    if (dy<-1.0) eyedy=-1; else if (dy>1.0) eyedy=1;
  }
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,sprite->tileid,sprite->xform);
  graf_draw_tile(&g.graf,g.texid_sprites,x+eyedx,y+eyedy,0x80,EGG_XFORM_SWAP);
}

const struct sprite_type sprite_type_bubblesaur={
  .name="bubblesaur",
  .objlen=sizeof(struct sprite_bubblesaur),
  .del=_bubblesaur_del,
  .init=_bubblesaur_init,
  .update=_bubblesaur_update,
  .render=_bubblesaur_render,
};
