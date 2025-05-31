#include "hero_internal.h"

/* Delete.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  sprite->phl=-0.400;
  sprite->phr=0.400;
  sprite->pht=-0.250; // extra headroom
  sprite->phb=0.500; // bottom of tiles tends to be exact
  SPRITE->facedy=1; // Default facing south.
  return 0;
}

/* Launch the pause modal.
 */
 
static void hero_choose(struct sprite *sprite) {
  int x=(int)(sprite->x*NS_sys_tilesize);
  int y=(int)(sprite->y*NS_sys_tilesize);
  modal_new_pause(x,y);
  g.input_blackout|=EGG_BTN_WEST;
}

/* Moved to a new cell. Look for switches etc.
 */
 
static void hero_change_quantized_position(struct sprite *sprite,int mapid,int x,int y) {
  const struct poi *poi=g.poiv;
  int i=g.poic;
  for (;i-->0;poi++) {
    if ((poi->x==x)&&(poi->y==y)) {
      switch (poi->opcode) {
        case CMD_map_treadle: {
            int cellp=poi->y*NS_sys_mapw+poi->x;
            g.map->cellv[cellp]=g.map->rocellv[cellp]+1;
            store_set((poi->argv[2]<<8)|poi->argv[3],1);
          } break;
        case CMD_map_stompbox: {
            int cellp=poi->y*NS_sys_mapw+poi->x;
            g.map->cellv[cellp]=g.map->rocellv[cellp]+1;
            int k=(poi->argv[2]<<8)|poi->argv[3];
            if (store_get(k)) store_set(k,0);
            else store_set(k,1);
          } break;
      }
    } else if ((SPRITE->mapid==mapid)&&(poi->x==SPRITE->cellx)&&(poi->y==SPRITE->celly)) {
      switch (poi->opcode) {
        case CMD_map_treadle: {
            int cellp=poi->y*NS_sys_mapw+poi->x;
            g.map->cellv[cellp]=g.map->rocellv[cellp];
            store_set((poi->argv[2]<<8)|poi->argv[3],0);
          } break;
        case CMD_map_stompbox: {
            int cellp=poi->y*NS_sys_mapw+poi->x;
            int k=(poi->argv[2]<<8)|poi->argv[3];
            if (store_get(k)) g.map->cellv[cellp]=g.map->rocellv[cellp]+2;
            else g.map->cellv[cellp]=g.map->rocellv[cellp];
          } break;
      }
    }
  }
  SPRITE->mapid=mapid;
  SPRITE->cellx=x;
  SPRITE->celly=y;
}

/* Walking.
 */
 
static void hero_walk_begin(struct sprite *sprite) {
  if (SPRITE->walking) return;
  //TODO Some items should inhibit walking.
  SPRITE->walking=1;
  SPRITE->animclock=0.0;
  SPRITE->animframe=0;
}

static void hero_walk_end(struct sprite *sprite) {
  if (!SPRITE->walking) return;
  SPRITE->walking=0;
}

static void hero_walk_update(struct sprite *sprite,double elapsed) {
  if (!SPRITE->walking) return;
  sprite_move(sprite,HERO_WALK_SPEED*SPRITE->indx*elapsed,HERO_WALK_SPEED*SPRITE->indy*elapsed);
  
  // Check for changes to quantized position.
  int mapid=g.map?g.map->rid:0;
  int cellx=(int)sprite->x;
  int celly=(int)sprite->y;
  if ((mapid!=SPRITE->mapid)||(cellx!=SPRITE->cellx)||(celly!=SPRITE->celly)) {
    hero_change_quantized_position(sprite,mapid,cellx,celly);
  }
}

/* Update (indx,indy), motion state, and (faced) in response to an input change.
 */
 
static void hero_update_ind(struct sprite *sprite) {
  int nindx=0,nindy=0;
  switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: nindx=-1; break;
    case EGG_BTN_RIGHT: nindx=1; break;
  }
  switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
    case EGG_BTN_UP: nindy=-1; break;
    case EGG_BTN_DOWN: nindy=1; break;
  }
  if ((nindx==SPRITE->indx)&&(nindy==SPRITE->indy)) return; // No change.
  
  /* If an axis changed to nonzero, face that way.
   * Otherwise if one existing axis is nonzero, face that way.
   * Otherwise don't turn.
   * Ties break toward horizontal, but ties should be rare.
   */
  if (nindx&&(nindx!=SPRITE->indx)) {
    SPRITE->facedx=nindx;
    SPRITE->facedy=0;
  } else if (nindy&&(nindy!=SPRITE->indy)) {
    SPRITE->facedx=0;
    SPRITE->facedy=nindy;
  } else if (nindx) {
    SPRITE->facedx=nindx;
    SPRITE->facedy=0;
  } else if (nindy) {
    SPRITE->facedx=0;
    SPRITE->facedy=nindy;
  }
  
  SPRITE->indx=nindx;
  SPRITE->indy=nindy;
  
  /* Start or stop walking.
   */
  if (nindx||nindy) {
    hero_walk_begin(sprite);
  } else {
    hero_walk_end(sprite);
  }
}

/* Update animation.
 * For now we tick 4 frames at a fixed rate regardless of state.
 * What those frames mean exactly, is render's problem.
 * Easy to imagine that will need to change eventually.
 */
 
static void hero_update_animation(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  if (g.input!=g.pvinput) {
    if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) {
      hero_choose(sprite);
      SPRITE->indx=SPRITE->indy=0;
      SPRITE->walking=0;
      return;
    }
    if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) hero_item_begin(sprite);
    else if (!(g.input&EGG_BTN_SOUTH)&&(g.pvinput&EGG_BTN_SOUTH)) hero_item_end(sprite);
    hero_update_ind(sprite);
  }
  if (g.input&EGG_BTN_SOUTH) hero_item_update(sprite,elapsed);
  if (SPRITE->walking) hero_walk_update(sprite,elapsed);
  hero_update_animation(sprite,elapsed);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
  .render=hero_render,
};
