#include "game/game.h"

/* Store field metadata.
 */
 
static struct store_def {
  int p,c; // bits, little-endianly
} store_defv[FLD_COUNT];

static int store_defv_init() {
  struct store_def *def=store_defv;
  int i=FLD_COUNT;
  for (;i-->0;def++) def->c=1;
  
  // Call out any fields with a size other than 1:
  store_defv[NS_fld_qty_pepper].c=7;
  store_defv[NS_fld_qty_bomb  ].c=7;
  store_defv[NS_fld_qty_candy ].c=7;
  store_defv[NS_fld_equipped  ].c=4;
  
  int p=0;
  for (def=store_defv,i=FLD_COUNT;i-->0;def++) {
    def->p=p;
    p+=def->c;
  }
  int limit=STORE_SIZE<<3;
  if (p>limit) {
    fprintf(stderr,"!!! STORE_SIZE==%d (%d bits) but we need %d bits\n",STORE_SIZE,limit,p);
    return -1;
  }
  return 0;
}

/* Receive tilesheet, there should only be one.
 */
 
static int load_tilesheet(const void *src,int srcc) {
  struct rom_tilesheet_reader reader;
  struct rom_tilesheet_entry entry;
  if (rom_tilesheet_reader_init(&reader,src,srcc)<0) return -1;
  while (rom_tilesheet_reader_next(&entry,&reader)>0) {
    if (entry.tableid==NS_tilesheet_physics) {
      memcpy(g.physics+entry.tileid,entry.v,entry.c);
    }
  }
  return 0;
}

/* Init.
 */
 
int session_init() {

  if (store_defv_init()<0) return -1;
  
  /* Load all resources.
   */
  struct rom_reader reader;
  if (rom_reader_init(&reader,g.rom,g.romc)<0) return -1;
  struct rom_res *res;
  while (res=rom_reader_next(&reader)) {
    switch (res->tid) {
      case EGG_TID_tilesheet: if (res->rid==RID_tilesheet_tiles) if (load_tilesheet(res->v,res->c)<0) return -1; break;
      case EGG_TID_map: if (load_map(res->rid,res->v,res->c)<0) return -1; break;
      case EGG_TID_sprite: if (load_sprite(res->rid,res->v,res->c)<0) return -1; break;
    }
  }
  
  if (0) { // XXX TEMP Dump the world map.
    fprintf(stderr,"----- world map -----\n");
    struct map **p=g.maps_by_position;
    int y=0; for (;y<WORLDH;y++) {
      char dst[256];
      int dstc=0;
      int x=0; for (;x<WORLDW;x++,p++) {
        if (*p) {
          dstc+=snprintf(dst+dstc,sizeof(dst)-dstc," %3d",(*p)->rid);
        } else {
          memset(dst+dstc,' ',4);
          dstc+=4;
        }
      }
      fprintf(stderr,"%.*s\n",dstc,dst);
    }
    fprintf(stderr,"----- end of world map -----\n");
  }
  
  return 0;
}

/* Reset.
 */

int session_reset() {

  // Restore all map content to their defaults.
  struct map *map=g.mapv;
  int i=g.mapc;
  for (;i-->0;map++) memcpy(map->cellv,map->rocellv,sizeof(map->cellv));

  // Wipe store.
  memset(g.store,0,sizeof(g.store));
  g.store[0]=2; // NS_fld_one=1
  
  // Delete all sprites.
  while (g.spritec>0) {
    g.spritec--;
    sprite_del(g.spritev[g.spritec]);
  }
  
  if (1) {
    fprintf(stderr,"*** %s:%d: Enabling treasures. ***\n",__FILE__,__LINE__);
    store_set(NS_fld_got_broom,1);
    store_set(NS_fld_got_pepper,1);
    store_set(NS_fld_got_compass,1);
    store_set(NS_fld_got_stopwatch,1);
    store_set(NS_fld_got_camera,1);
    store_set(NS_fld_got_snowglobe,1);
    store_set(NS_fld_got_wand,1);
    store_set(NS_fld_got_bomb,1);
    store_set(NS_fld_got_candy,1);
    store_set(NS_fld_qty_pepper,99);
    store_set(NS_fld_qty_bomb,22);
    store_set(NS_fld_qty_candy,9);
  }
  
  // Load the home map.
  if (enter_map(RID_map_home,TRANSITION_NONE)<0) {
    fprintf(stderr,"Loading map:%d failed.\n",RID_map_home);
    return -1;
  }
  
  return 0;
}

/* Check all POI for any that react to a given flag change.
 */
 
static void check_changed_poi(int k,int v) {
  struct poi *poi=g.poiv;
  int i=g.poic;
  for (;i-->0;poi++) {
    switch (poi->opcode) {
      case CMD_map_switchable: {
          int qk=(poi->argv[2]<<8)|poi->argv[3];
          if (qk==k) {
            int cellp=poi->y*NS_sys_mapw+poi->x;
            g.map->cellv[cellp]=g.map->rocellv[cellp]+(v?1:0);
          }
        } break;
    }
  }
}

/* For all the POI we've gathered at entering a new map, do whatever they need.
 */
 
static void prerun_poi() {
  struct poi *poi=g.poiv;
  int i=g.poic;
  for (;i-->0;poi++) {
    switch (poi->opcode) {
      case CMD_map_treadle: {
          int cellp=poi->y*NS_sys_mapw+poi->x;
          g.map->cellv[cellp]=g.map->rocellv[cellp];
          uint16_t k=(poi->argv[2]<<8)|poi->argv[3];
          store_set(k,0);
        } break;
      case CMD_map_stompbox: {
          int cellp=poi->y*NS_sys_mapw+poi->x;
          g.map->cellv[cellp]=g.map->rocellv[cellp];
          uint16_t k=(poi->argv[2]<<8)|poi->argv[3];
          if (store_get(k)) {
            g.map->cellv[cellp]=g.map->rocellv[cellp]+2;
          } else {
            g.map->cellv[cellp]=g.map->rocellv[cellp];
          }
        } break;
      case CMD_map_switchable: {
          int cellp=poi->y*NS_sys_mapw+poi->x;
          g.map->cellv[cellp]=g.map->rocellv[cellp];
          uint16_t k=(poi->argv[2]<<8)|poi->argv[3];
          if (store_get(k)) {
            g.map->cellv[cellp]=g.map->rocellv[cellp]+1;
          } else {
            g.map->cellv[cellp]=g.map->rocellv[cellp];
          }
        } break;
    }
  }
}

/* Enter map.
 */
 
int enter_map(int rid,int transition) {
  struct map *map=map_by_id(rid);
  if (!map) return -1;
  g.map=map;
  g.poic=0;
  
  sprites_delete_volatile();
  
  struct rom_command_reader reader={.v=g.map->cmdv,.c=g.map->cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_treadle:
      case CMD_map_stompbox:
      case CMD_map_switchable: {
          if (g.poic<POI_LIMIT) {
            struct poi *poi=g.poiv+g.poic++;
            poi->x=cmd.argv[0];
            poi->y=cmd.argv[1];
            if ((poi->x>=NS_sys_mapw)||(poi->y>=NS_sys_maph)) {
              g.poic--;
              break;
            }
            poi->opcode=cmd.opcode;
            poi->argv=cmd.argv;
            poi->argc=cmd.argc;
          }
        } break;
      case CMD_map_sprite: {
          uint8_t x=cmd.argv[0];
          uint8_t y=cmd.argv[1];
          uint16_t rid=(cmd.argv[2]<<8)|cmd.argv[3];
          uint32_t arg=(cmd.argv[4]<<24)|(cmd.argv[5]<<16)|(cmd.argv[6]<<8)|cmd.argv[7];
          if ((rid==RID_sprite_hero)&&g.hero) break; // already got it
          if ((rid==RID_sprite_princess)&&g.princess) break; // already got it
          sprite_spawn(x+0.5,y+0.5,rid,0,arg);
        } break;
      case CMD_map_field: {
          uint16_t k=(cmd.argv[0]<<8)|cmd.argv[1];
          uint16_t v=(cmd.argv[2]<<8)|cmd.argv[3];
          store_set(k,v);
        } break;
    }
  }
  
  prerun_poi();
  
  // Apply transition to the persistent sprites.
  switch (transition) {
    case TRANSITION_LEFT: {
        if (g.hero) g.hero->x+=NS_sys_mapw;
        if (g.princess) g.princess->x+=NS_sys_mapw;
      } break;
    case TRANSITION_RIGHT: {
        if (g.hero) g.hero->x-=NS_sys_mapw;
        if (g.princess) g.princess->x-=NS_sys_mapw;
      } break;
    case TRANSITION_UP: {
        if (g.hero) g.hero->y+=NS_sys_maph;
        if (g.princess) g.princess->y+=NS_sys_maph;
      } break;
    case TRANSITION_DOWN: {
        if (g.hero) g.hero->y-=NS_sys_maph;
        if (g.princess) g.princess->y-=NS_sys_maph;
      } break;
  }
  
  if (transition) {
    // The previous-scene snapshot (g.transition_texid) must already have been prepared by modal_play.
    g.transition=transition;
    g.transition_clock=0.0;
  } else {
    g.transition=0;
  }
  
  return 0;
}

/* Add listener.
 */
 
int store_listen(int k,void (*cb)(int k,int v,void *userdata),void *userdata) {
  if (!cb) return -1;
  if (g.listenerc>=LISTENER_LIMIT) {
    fprintf(stderr,"LISTENER_LIMIT breached (%d)\n",LISTENER_LIMIT);
    return -1;
  }
  if (g.listenerid_next<1) g.listenerid_next=1;
  struct listener *listener=g.listenerv+g.listenerc++;
  listener->listenerid=g.listenerid_next++;
  listener->k=k;
  listener->userdata=userdata;
  listener->cb=cb;
  return listener->listenerid;
}

/* Remove listener.
 */
 
void store_unlisten(int listenerid) {
  if (listenerid<1) return;
  int i=g.listenerc;
  struct listener *listener=g.listenerv+i-1;
  for (;i-->0;listener--) {
    if (listener->listenerid!=listenerid) continue;
    g.listenerc--;
    memmove(listener,listener+1,sizeof(struct listener)*(g.listenerc-i));
    return;
  }
}

/* Unregulated store access.
 * We'll do this one bit at a time because most fields are 1-bit, and the more efficient way is error-prone.
 */
 
static int store_read(const uint8_t *src,int p,int c) {
  src+=p>>3;
  uint8_t mask=1<<(p&7);
  int v=0,i=0;
  for (;i<c;i++) {
    if ((*src)&mask) v|=1<<i;
    if (!(mask<<=1)) { mask=1; src++; }
  }
  return v;
}

static void store_write(uint8_t *dst,int p,int c,int v) {
  dst+=p>>3;
  uint8_t mask=1<<(p&7);
  int i=0;
  for (;i<c;i++) {
    if (v&(1<<i)) (*dst)|=mask; else (*dst)&=~mask;
    if (!(mask<<=1)) { mask=1; dst++; }
  }
}

/* Get store field.
 */

int store_get(int k) {
  if ((k<0)||(k>=FLD_COUNT)) return 0;
  const struct store_def *def=store_defv+k;
  return store_read(g.store,def->p,def->c);
}

/* Set store field.
 */
 
int store_set(int k,int v) {
  if ((k<1)||(k>=FLD_COUNT)) return 0;
  if (k==1) return 1; // Fields 0 and 1 are immutable, with values 0 and 1.
  const struct store_def *def=store_defv+k;
  int limit=(1<<def->c)-1;
  if (v<0) v=0; else if (v>limit) v=limit;
  int pv=store_read(g.store,def->p,def->c);
  if (pv==v) return v;
  store_write(g.store,def->p,def->c,v);
  check_changed_poi(k,v);
  struct listener *listener=g.listenerv;
  int i=g.listenerc;
  for (;i-->0;listener++) {
    if (listener->k!=k) continue;
    listener->cb(k,v,listener->userdata);
  }
  return v;
}
