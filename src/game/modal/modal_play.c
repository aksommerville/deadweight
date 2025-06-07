#include "game/game.h"

// There's a fixed set of chronflakes, initially random.
// When one expires, we create a new one in its place.
#define PLAY_CHRONFLAKE_COUNT 40
#define PLAY_CHRONFLAKE_TTL 60 /* video frames */

/* Maximum count of sprites at one time that can actuate quantized triggers (treadle, stompbox, maybe other things...).
 * That means basically every sprite that conceptually has its feet on the ground.
 */
#define PLAY_QSTEP_LIMIT 32

struct modal_play {
  struct modal hdr;
  struct chronflake {
    int x,y; // fb coords
    int ttl; // Video frames, so we can update and render at the same time. Counts down.
  } chronflakev[PLAY_CHRONFLAKE_COUNT];
  
  struct qstep {
    struct sprite *sprite; // WEAK, for indexing.
    int x,y;
    int scratch;
  } qstepv[PLAY_QSTEP_LIMIT];
  int qstepc;
  int qstep_mapid;
  uint8_t qonv[PLAY_QSTEP_LIMIT];
  int qonc;
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

/* Move things around due to earthquake.
 */
 
static void play_update_earthquake(struct modal *modal,double dx,double dy) {
  int i=g.spritec;
  while (i-->0) {
    struct sprite *sprite=g.spritev[i];
    if (sprite->defunct) continue;
    if (sprite->airborne) continue;
    
    /* If it's more than one meter offscreen, don't move at all.
     * This is mostly for the Princess, when you leave her behind.
     */
    if ((sprite->x<-1.0)||(sprite->y<-1.0)||(sprite->x>NS_sys_mapw+1.0)||(sprite->y>NS_sys_maph+1.0)) continue;
    
    /* If it's just a little bit offscreen, permit motion in the direction that brings it back.
     */
    if ((sprite->x<0.0)&&(dx<=0.0)) continue;
    if ((sprite->y<0.0)&&(dy<=0.0)) continue;
    if ((sprite->x>NS_sys_mapw)&&(dx>=0.0)) continue;
    if ((sprite->y>NS_sys_maph)&&(dy>=0.0)) continue;
    
    if (sprite->decorative) continue;
    if (sprite->type==&sprite_type_selfie) continue;
    //TODO there surely are other things that need to be earthquake-proof
    
    int pvsolid=sprite->solid;
    sprite->solid=1;
    sprite_move(sprite,dx,dy);
    sprite->solid=pvsolid;
  }
}

/* Final answer from qstep+qon: Turn POI features on and off.
 */
 
static void play_poi_activate(struct modal *modal,int x,int y,int v) {
  struct poi *poi=g.poiv;
  int i=g.poic;
  for (;i-->0;poi++) {
    if ((poi->x!=x)||(poi->y!=y)) continue;
    int cellp=poi->y*NS_sys_mapw+poi->x;
    switch (poi->opcode) {
      
      case CMD_map_treadle: {
          egg_play_sound(RID_sound_treadle);
          int k=(poi->argv[2]<<8)|poi->argv[3];
          g.map->cellv[cellp]=g.map->rocellv[cellp]+v;
          store_set(k,v);
        } break;
        
      case CMD_map_stompbox: {
          egg_play_sound(RID_sound_treadle);
          int k=(poi->argv[2]<<8)|poi->argv[3];
          if (v) {
            g.map->cellv[cellp]=g.map->rocellv[cellp]+2;
            if (store_get(k)) {
              store_set(k,0);
            } else {
              store_set(k,1);
            }
          } else {
            g.map->cellv[cellp]=g.map->rocellv[cellp]+store_get(k);
          }
        } break;
        
    }
  }
}

/* qon list primitives.
 */
 
static int play_qonv_search(const struct modal *modal,uint8_t v) {
  int lo=0,hi=MODAL->qonc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
         if (v<MODAL->qonv[ck]) hi=ck;
    else if (v>MODAL->qonv[ck]) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}

static int play_qonv_insert(struct modal *modal,int p,uint8_t v) {
  if ((p<0)||(p>MODAL->qonc)) return -1;
  if (MODAL->qonc>=PLAY_QSTEP_LIMIT) return -1;
  memmove(MODAL->qonv+p+1,MODAL->qonv+p,MODAL->qonc-p);
  MODAL->qonc++;
  MODAL->qonv[p]=v;
  return 0;
}

/* Notify that a sprite has entered or exitted a cell.
 * This doesn't necessarily change the cell's state, we then have to check whether anybody else is on it.
 * Cells outside the map are never acknowledged.
 */
 
static void play_quantized_step(struct modal *modal,int x,int y) {
  if ((x<0)||(y<0)||(x>=NS_sys_mapw)||(y>=NS_sys_maph)) return;
  int id=y*NS_sys_mapw+x;
  int p=play_qonv_search(modal,id);
  if (p>=0) return; // already on
  p=-p-1;
  if (play_qonv_insert(modal,p,id)<0) return; // failed to add
  play_poi_activate(modal,x,y,1);
}

// Caller must modify the qstep in question before calling.
static void play_quantized_unstep(struct modal *modal,int x,int y) {
  if ((x<0)||(y<0)||(x>=NS_sys_mapw)||(y>=NS_sys_maph)) return;
  int id=y*NS_sys_mapw+x;
  int p=play_qonv_search(modal,id);
  if (p<0) return; // already off
  // Now we have to iterate all qsteps to confirm that nobody else is standing on it, before removing.
  int i=MODAL->qstepc;
  const struct qstep *qstep=MODAL->qstepv;
  for (;i-->0;qstep++) {
    if ((qstep->x==x)&&(qstep->y==y)) return; // Somebody else is holding it.
  }
  // Release!
  MODAL->qonc--;
  memmove(MODAL->qonv+p,MODAL->qonv+p+1,MODAL->qonc-p);
  play_poi_activate(modal,x,y,0);
}

/* qstep list primitives.
 */
 
static int play_qstepv_search(const struct modal *modal,const struct sprite *sprite) {
  int lo=0,hi=MODAL->qstepc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct qstep *q=MODAL->qstepv+ck;
         if (sprite<q->sprite) hi=ck;
    else if (sprite>q->sprite) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}

static struct qstep *play_qstepv_insert(struct modal *modal,int p,struct sprite *sprite) {
  if ((p<0)||(p>MODAL->qstepc)) return 0;
  if (MODAL->qstepc>=PLAY_QSTEP_LIMIT) return 0;
  struct qstep *qstep=MODAL->qstepv+p;
  memmove(qstep+1,qstep,sizeof(struct qstep)*(MODAL->qstepc-p));
  MODAL->qstepc++;
  qstep->sprite=sprite;
  return qstep;
}

/* Does this sprite participate in qstep?
 */
 
static int sprite_qsteppable(const struct sprite *sprite) {
  if (sprite->defunct) return 0; // obviously
  if (sprite->decorative) return 0;

  // eg hero on broom is temporarily non-qsteppable
  if (sprite->airborne) return 0;
  
  // Call out specific types which might not be solid but must be qsteppable.
  if (sprite->type==&sprite_type_hero) return 1;
  if (sprite->type==&sprite_type_princess) return 1;
  if (sprite->type==&sprite_type_treasure) return 1;
  
  // Anything else, it's qsteppable if it's solid.
  return sprite->solid;
}

/* Track hero, princess, and anything else that is able to trigger treadles and such.
 * Sprite controllers are not expected to do this manually, let us handle it all together.
 */
 
static void play_update_quantized_motion(struct modal *modal) {
  struct qstep *qstep;
  int i;
  
  // If mapid changed, drop all steps. State changes due to the dropped steps are not our problem.
  if (!g.map) return;
  if (g.map->rid!=MODAL->qstep_mapid) {
    MODAL->qonc=0;
    MODAL->qstepc=0;
    MODAL->qstep_mapid=g.map->rid;
  }
  
  // It's perfectly reasonable for a map to have no POI. Don't bother tracking quantized motion in those cases.
  if (!g.poic) return;
  
  // Zero all (scratch). If it stays zero, the sprite is no longer relevant and its qstep should be removed.
  for (i=MODAL->qstepc,qstep=MODAL->qstepv;i-->0;qstep++) qstep->scratch=0;
  
  /* Walk the global sprite list, filter to qsteppable sprites, and compare to their qstep record.
   * Doesn't exist yet? Add it and mark that cell stepped.
   * Changed? Mark the old cell unstepped, update, mark the new one stepped.
   */
  struct sprite **p=g.spritev;
  for (i=g.spritec;i-->0;p++) {
    struct sprite *sprite=*p;
    if (!sprite_qsteppable(sprite)) continue;
    int qstepp=play_qstepv_search(modal,sprite);
    if (qstepp<0) {
      qstepp=-qstepp-1;
      if (!(qstep=play_qstepv_insert(modal,qstepp,sprite))) continue;
      qstep->scratch=1;
      qstep->x=(int)sprite->x; if (sprite->x<0.0) qstep->x--;
      qstep->y=(int)sprite->y; if (sprite->y<0.0) qstep->y--;
      play_quantized_step(modal,qstep->x,qstep->y);
    } else {
      qstep=MODAL->qstepv+qstepp;
      qstep->scratch=1;
      int nx=(int)sprite->x; if (sprite->x<0.0) nx--;
      int ny=(int)sprite->y; if (sprite->y<0.0) ny--;
      if ((nx==qstep->x)&&(ny==qstep->y)) continue;
      int ox=qstep->x;
      int oy=qstep->y;
      qstep->x=nx;
      qstep->y=ny;
      play_quantized_unstep(modal,ox,oy);
      play_quantized_step(modal,nx,ny);
    }
  }
  
  // Any qsteps that still have zero scratch are defunct, remove them.
  for (i=MODAL->qstepc,qstep=MODAL->qstepv+MODAL->qstepc-1;i-->0;qstep--) {
    if (qstep->scratch) continue;
    int ox=qstep->x;
    int oy=qstep->y;
    qstep->x=qstep->y=-1;
    play_quantized_unstep(modal,ox,oy);
    MODAL->qstepc--;
    memmove(qstep,qstep+1,sizeof(struct qstep)*(MODAL->qstepc-i));
  }
}

/* Update.
 */
 
static void _play_update(struct modal *modal,double elapsed) {

  // Apply earthquake if that's going on.
  if (g.earthquake_cooldown>0.0) {
    g.earthquake_cooldown-=elapsed;
    const double speed=2.0;
    play_update_earthquake(modal,g.eqdx*speed*elapsed,g.eqdy*speed*elapsed);
  }

  // Most of the action is driven by sprite controllers:
  sprites_update(elapsed);
  
  // Update our tracking of treadles and such.
  play_update_quantized_motion(modal);
  
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
  
  // Jostle a bit during earthquakes.
  if (g.earthquake_cooldown>0.0) {
    int t=(int)(g.earthquake_cooldown*20.0);
    dsty+=t%3-1;
  }

  // Draw the new scene.
  graf_draw_tile_buffer(&g.graf,g.texid_tiles,dstx+(NS_sys_tilesize>>1),dsty+(NS_sys_tilesize>>1),g.map->cellv,NS_sys_mapw,NS_sys_maph,NS_sys_mapw);
  sprites_render(dstx,dsty);
  
  /* Weather and such, above the scene.
   */
  if (g.time_stopped) play_render_chronflakes(modal);
  
  /* Item quantities, even abover.
   */
  dstx=FBW-3;
  dsty=4;
  #define SHOWQUANTITY(thingname,tileid) { \
    if (store_get(NS_fld_got_##thingname)) { \
      int qty=store_get(NS_fld_qty_##thingname); \
      graf_draw_tile(&g.graf,g.texid_sprites,dstx,dsty,0x90+qty%10,0); dstx-=4; \
      graf_draw_tile(&g.graf,g.texid_sprites,dstx,dsty,0x90+qty/10,0); dstx-=6; \
      graf_draw_tile(&g.graf,g.texid_sprites,dstx,dsty,tileid,0); dstx-=8; \
    } \
  }
  SHOWQUANTITY(pepper,0x59)
  SHOWQUANTITY(candy,0x58)
  SHOWQUANTITY(bomb,0x57)
  #undef SHOWQUANTITY
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
  
  if (store_get(NS_fld_boss_dead)) egg_play_song(RID_song_we_need_norris,0,1);
  else egg_play_song(RID_song_embark_at_sunrise,0,1);
  
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
