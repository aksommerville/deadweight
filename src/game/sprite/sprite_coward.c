/* sprite_coward.c
 * If monsters are present, I hide behind the tile below me. That should be solid with a flat upper row.
 * When all the monsters are dead, I stand still and produce a dialogue box when the hero steps on me.
 */
 
#include "game/game.h"

struct sprite_coward {
  struct sprite hdr;
  uint8_t tileid0;
  int strix; // rid 1
  double y0;
  int triggered;
};

#define SPRITE ((struct sprite_coward*)sprite)

static int _coward_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  SPRITE->strix=sprite->arg>>24;
  SPRITE->y0=sprite->y;
  return 0;
}

static int all_monsters_dead() {
  int i=g.spritec;
  while (i-->0) {
    struct sprite *monster=g.spritev[i];
    if (monster->defunct) continue;
    if (!monster->monster) continue;
    return 0;
  }
  return 1;
}

// Does not check (triggered); caller must.
static int coward_check_dialogue(struct sprite *sprite) {
  if (!g.hero) return 0;
  if (!SPRITE->strix) return 0;
  const double radius=0.750;
  double dx=g.hero->x-sprite->x;
  if ((dx<-radius)||(dx>radius)) return 0;
  double dy=g.hero->y-sprite->y;
  if ((dy<-radius)||(dy>radius)) return 0;
  return 1;
}

static void _coward_update(struct sprite *sprite,double elapsed) {
  if (all_monsters_dead()) {
    sprite->tileid=SPRITE->tileid0;
    sprite->y=SPRITE->y0;
    if (coward_check_dialogue(sprite)) {
      if (!SPRITE->triggered) {
        SPRITE->triggered=1;
        int x=(int)(sprite->x*NS_sys_tilesize);
        int y=(int)(sprite->y*NS_sys_tilesize);
        modal_new_dialogue(x,y,1,SPRITE->strix,0,0);
      }
    } else {
      SPRITE->triggered=0;
    }
  } else {
    sprite->tileid=SPRITE->tileid0+1;
    sprite->y=SPRITE->y0+0.5;
    SPRITE->triggered=0;
  }
}

const struct sprite_type sprite_type_coward={
  .name="coward",
  .objlen=sizeof(struct sprite_coward),
  .init=_coward_init,
  .update=_coward_update,
};
