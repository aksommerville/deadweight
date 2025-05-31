#include "game/game.h"

struct modal_play {
  struct modal hdr;
};

#define MODAL ((struct modal_play*)modal)

/* Cleanup.
 */
 
static void _play_del(struct modal *modal) {
}

/* Input.
 */
 
static void _play_input(struct modal *modal) {
}

/* Navigate if a neighbor map exists.
 */
 
static int play_navigate(struct modal *modal,int dx,int dy) {
  int nlng=g.map->longitude+dx;
  int nlat=g.map->latitude+dy;
  if ((nlng<0)||(nlng>=WORLDW)||(nlat<0)||(nlat>=WORLDH)) return 0;
  int np=nlat*WORLDW+nlng;
  struct map *nmap=g.maps_by_position[np];
  if (!nmap) return 0;
  int transition;
       if (dx<0) transition=TRANSITION_LEFT;
  else if (dx>0) transition=TRANSITION_RIGHT;
  else if (dy<0) transition=TRANSITION_UP;
  else if (dy>0) transition=TRANSITION_DOWN;
  else return -1;
  
  // Draw the outgoing scene into (g.transition_texid).
  graf_set_output(&g.graf,g.transition_texid);
  graf_draw_tile_buffer(&g.graf,g.texid_tiles,(NS_sys_tilesize>>1),(NS_sys_tilesize>>1),g.map->cellv,NS_sys_mapw,NS_sys_maph,NS_sys_mapw);
  sprites_render_volatile();
  graf_set_output(&g.graf,1);
  graf_flush(&g.graf);
  
  return enter_map(nmap->rid,transition);
}

/* Update.
 */
 
static void _play_update(struct modal *modal,double elapsed) {
  sprites_update(elapsed);
  
  // Check for navigation.
  if (g.hero&&g.map) {
    if (g.hero->x<0.0) play_navigate(modal,-1,0);
    else if (g.hero->x>NS_sys_mapw) play_navigate(modal,1,0);
    else if (g.hero->y<0.0) play_navigate(modal,0,-1);
    else if (g.hero->y>NS_sys_maph) play_navigate(modal,0,1);
  }
  
  if (g.transition) {
    if ((g.transition_clock+=elapsed)>=TRANSITION_TIME) {
      g.transition=0;
    }
  }
}

/* Render.
 */
 
static void _play_render(struct modal *modal) {
  int dstx=0,dsty=0;
  
  // If a transition is in progress, bump (dstx,dsty) and draw the previous-scene snapshot.
  if (g.transition&&(g.transition_clock<TRANSITION_TIME)) {
    double t=g.transition_clock/TRANSITION_TIME;
    int pvx=0,pvy=0;
    switch (g.transition) {
      case TRANSITION_LEFT: dstx-=FBW; dstx+=(int)(FBW*t); pvx=dstx+FBW; break;
      case TRANSITION_RIGHT: dstx+=FBW; dstx-=(int)(FBW*t); pvx=dstx-FBW; break;
      case TRANSITION_UP: dsty-=FBH; dsty+=(int)(FBH*t); pvy=dsty+FBH; break;
      case TRANSITION_DOWN: dsty+=FBH; dsty-=(int)(FBH*t); pvy=dsty-FBH; break;
    }
    graf_draw_decal(&g.graf,g.transition_texid,pvx,pvy,0,0,FBW,FBH,0);
  }

  // Draw the new scene.
  graf_draw_tile_buffer(&g.graf,g.texid_tiles,dstx+(NS_sys_tilesize>>1),dsty+(NS_sys_tilesize>>1),g.map->cellv,NS_sys_mapw,NS_sys_maph,NS_sys_mapw);
  sprites_render(dstx,dsty);
}

/* Init.
 */
 
static int _play_init(struct modal *modal) {
  modal->name="play";
  modal->del=_play_del;
  modal->input=_play_input;
  modal->update=_play_update;
  modal->render=_play_render;
  modal->opaque=1;
  modal->passive=0;
  return 0;
}

/* New.
 */
 
struct modal *modal_new_play() {
  struct modal *modal=modal_new(sizeof(struct modal_play));
  if (!modal) return 0;
  if (_play_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
