#include "game/game.h"

// There's a fixed set of chronflakes, initially random.
// When one expires, we create a new one in its place.
#define PLAY_CHRONFLAKE_COUNT 40
#define PLAY_CHRONFLAKE_TTL 60 /* video frames */

struct modal_play {
  struct modal hdr;
  struct chronflake {
    int x,y; // fb coords
    int ttl; // Video frames, so we can update and render at the same time. Counts down.
  } chronflakev[PLAY_CHRONFLAKE_COUNT];
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

/* Chronflakes: Some crystally kind of effect above the scene, when stopwatch in use.
 * Not sure how possible this would have been for a real NES, but Mega Man 2 does something similarish.
 */
 
static void play_render_chronflakes(struct modal *modal) {
  struct chronflake *chronflake=MODAL->chronflakev;
  int i=PLAY_CHRONFLAKE_COUNT;
  for (;i-->0;chronflake++) {
    if (--(chronflake->ttl)<=0) {
      chronflake->x=rand()%FBW;
      chronflake->y=rand()%FBH;
      chronflake->ttl=PLAY_CHRONFLAKE_TTL;
    }
    uint8_t tileid;
    switch ((chronflake->ttl*5)/PLAY_CHRONFLAKE_TTL) {
      case 0: tileid=0x2f; break;
      case 1: tileid=0x2e; break;
      case 2: tileid=0x2d; break;
      case 3: tileid=0x2e; break;
      default: tileid=0x2f;
    }
    graf_draw_tile(&g.graf,g.texid_sprites,chronflake->x,chronflake->y,tileid,0);
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
  
  /* Weather and such, above the scene.
   */
  if (g.time_stopped) play_render_chronflakes(modal);
}

/* Init.
 */
 
static int _play_init(struct modal *modal) {
  modal->name="play";
  modal->ctor=modal_new_play;
  modal->del=_play_del;
  modal->input=_play_input;
  modal->update=_play_update;
  modal->render=_play_render;
  modal->opaque=1;
  modal->passive=0;
  
  egg_play_song(RID_song_we_need_norris,0,1);
  
  // Chronflakes are initially randomized, so we can start displaying whenever.
  // They don't update when not in use.
  struct chronflake *chronflake=MODAL->chronflakev;
  int i=PLAY_CHRONFLAKE_COUNT;
  for (;i-->0;chronflake++) {
    chronflake->x=rand()%FBW;
    chronflake->y=rand()%FBH;
    chronflake->ttl=rand()%PLAY_CHRONFLAKE_TTL+1;
  }
  
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
