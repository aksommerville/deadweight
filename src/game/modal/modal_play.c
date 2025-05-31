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
  fprintf(stderr,"%s: Navigate from map:%d (%+d,%+d) to map:%d\n",__func__,g.map->rid,dx,dy,nmap->rid);
  int transition=TRANSITION_NONE;
       if (dx<0) transition=TRANSITION_LEFT;
  else if (dx>0) transition=TRANSITION_RIGHT;
  else if (dy<0) transition=TRANSITION_UP;
  else if (dy>0) transition=TRANSITION_DOWN;
  else return -1;
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
}

/* Render.
 */
 
static void _play_render(struct modal *modal) {
  //TODO transition
  graf_draw_tile_buffer(&g.graf,g.texid_tiles,NS_sys_tilesize>>1,NS_sys_tilesize>>1,g.map->cellv,NS_sys_mapw,NS_sys_maph,NS_sys_mapw);
  sprites_render(0,0);
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
