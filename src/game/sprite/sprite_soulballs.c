/* sprite_soulballs.c
 * Args: u8:count u8:mode(0=generic,1=hero,2=princess) u16:reserved
 */
 
#include "game/game.h"

#define SOULBALLS_MODE_GENERIC 0
#define SOULBALLS_MODE_HERO 1
#define SOULBALLS_MODE_PRINCESS 2

#define SOULBALLS_PHASE_EXPAND 1
#define SOULBALLS_PHASE_TRAVEL 2
#define SOULBALLS_PHASE_CONTRACT 3

#define SOULBALLS_EXPAND_TIME   0.333 /* s */
#define SOULBALLS_CONTRACT_TIME 0.333 /* s */
#define SOULBALLS_TRAVEL_SPEED 20.000 /* m/s */
#define SOULBALLS_ROTATE_SPEED  7.000 /* rad/s */
#define SOULBALLS_RADIUS        2.000 /* m */
#define SOULBALLS_RADIUS_MAX    6.000 /* GENERIC mode terminates here. */

struct sprite_soulballs {
  struct sprite hdr;
  uint8_t count;
  uint8_t mode;
  int phase;
  double clock;
  double t;
  double animclock;
  int animframe;
  double radius;
};

#define SPRITE ((struct sprite_soulballs*)sprite)

static int _soulballs_init(struct sprite *sprite) {
  sprite->layer=200;
  sprite->decorative=1;
  sprite->airborne=1;
  sprite->tileid=0x70; // 3 tiles horizontally
  SPRITE->phase=SOULBALLS_PHASE_EXPAND;
  SPRITE->count=sprite->arg>>24; if (SPRITE->count<1) SPRITE->count=1;
  SPRITE->mode=sprite->arg>>16;
  
  // This is hacky, but the best way to count dead princesses and monsters is actually by tracking instantiation of soulballs.
  // Dead heroes are tracked elsewhere, more respectably.
  switch (SPRITE->mode) {
    case SOULBALLS_MODE_GENERIC: g.deadmonsterc++; break;
    case SOULBALLS_MODE_PRINCESS: g.deadprincessc++; break;
  }
  
  return 0;
}

static void soulballs_get_destination(double *dstx,double *dsty,struct sprite *sprite) {
  *dstx=sprite->x;
  *dsty=sprite->y;
  switch (SPRITE->mode) {
    case SOULBALLS_MODE_GENERIC: {
        // Collapse into wherever we started; this soul persisteth not.
      } break;
    case SOULBALLS_MODE_HERO: {
        if (g.hero) {
          *dstx=g.hero->x;
          *dsty=g.hero->y;
        }
      } break;
    case SOULBALLS_MODE_PRINCESS: {
        if (g.princess) {
          *dstx=g.princess->x;
          *dsty=g.princess->y;
        } else { // the usual case, she's dead and back at the dungeon.
          int lng=g.princess_map_index%WORLDW-g.map->longitude;
          int lat=g.princess_map_index/WORLDW-g.map->latitude;
          *dstx=(lng*NS_sys_mapw)+NS_sys_mapw*0.5;
          *dsty=(lat*NS_sys_maph)+NS_sys_maph*0.5;
        }
      } break;
  }
}

static void _soulballs_update(struct sprite *sprite,double elapsed) {

  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.125;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
  
  SPRITE->t+=SOULBALLS_ROTATE_SPEED*elapsed;
  if (SPRITE->t>M_PI) SPRITE->t-=M_PI*2.0;
  
  SPRITE->clock+=elapsed;
  switch (SPRITE->phase) {
  
    case SOULBALLS_PHASE_EXPAND: {
        // In GENERIC mode, EXPAND is the only phase, just defunct the sprite when it's sufficiently wide.
        if (SPRITE->mode==SOULBALLS_MODE_GENERIC) {
          SPRITE->radius=(SPRITE->clock*SOULBALLS_RADIUS)/SOULBALLS_EXPAND_TIME;
          if (SPRITE->radius>=SOULBALLS_RADIUS_MAX) sprite->defunct=1;
        } else if (SPRITE->clock>=SOULBALLS_EXPAND_TIME) {
          SPRITE->clock=0.0;
          SPRITE->phase=SOULBALLS_PHASE_TRAVEL;
        } else {
          SPRITE->radius=(SPRITE->clock*SOULBALLS_RADIUS)/SOULBALLS_EXPAND_TIME;
        }
      } break;
      
    // Continue travelling during the CONTRACT phase.
    case SOULBALLS_PHASE_TRAVEL:
    case SOULBALLS_PHASE_CONTRACT: {
        SPRITE->radius=SOULBALLS_RADIUS;
        double dstx,dsty;
        soulballs_get_destination(&dstx,&dsty,sprite);
        double dx=dstx-sprite->x,dy=dsty-sprite->y;
        double distance=sqrt(dx*dx+dy*dy);
        double speed=SOULBALLS_TRAVEL_SPEED*elapsed;
        if (distance<=speed) {
          if (SPRITE->phase==SOULBALLS_PHASE_TRAVEL) {
            SPRITE->clock=0.0;
            SPRITE->phase=SOULBALLS_PHASE_CONTRACT;
          }
        } else {
          sprite->x+=(speed*dx)/distance;
          sprite->y+=(speed*dy)/distance;
          if ((sprite->x<-3.0)||(sprite->y<-3.0)||(sprite->x>NS_sys_mapw+3.0)||(sprite->y>NS_sys_maph+3.0)) {
            sprite->defunct=1;
          }
        }
        if (SPRITE->phase==SOULBALLS_PHASE_CONTRACT) {
          if (SPRITE->clock>=SOULBALLS_CONTRACT_TIME) {
            sprite->defunct=1;
          } else {
            SPRITE->radius=SOULBALLS_RADIUS-(SPRITE->clock*SOULBALLS_RADIUS)/SOULBALLS_EXPAND_TIME;
          }
        }
      } break;
  }
}

static void _soulballs_render(struct sprite *sprite,int x,int y) {
  uint8_t tileid=sprite->tileid;
  switch (SPRITE->animframe) {
    case 0: break;
    case 1: tileid+=1; break;
    case 2: tileid+=2; break;
    case 3: tileid+=1; break;
  }
  double dt=(M_PI*2.0)/SPRITE->count;
  double t=SPRITE->t;
  int i=SPRITE->count;
  for (;i-->0;t+=dt) {
    double dx=cos(t)*SPRITE->radius;
    double dy=-sin(t)*SPRITE->radius;
    int dstx=x+(int)(dx*NS_sys_tilesize);
    int dsty=y+(int)(dy*NS_sys_tilesize);
    graf_draw_tile(&g.graf,g.texid_sprites,dstx,dsty,tileid,0);
  }
}

const struct sprite_type sprite_type_soulballs={
  .name="soulballs",
  .objlen=sizeof(struct sprite_soulballs),
  .init=_soulballs_init,
  .update=_soulballs_update,
  .render=_soulballs_render,
};
