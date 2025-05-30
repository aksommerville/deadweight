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
 
static void load_tilesheet(const void *src,int srcc) {
  struct rom_tilesheet_reader reader;
  struct rom_tilesheet_entry entry;
  if (rom_tilesheet_reader_init(&reader,src,srcc)<0) return;
  while (rom_tilesheet_reader_next(&entry,&reader)>0) {
    if (entry.tableid==NS_tilesheet_physics) {
      memcpy(g.physics+entry.tileid,entry.v,entry.c);
    }
  }
}

/* Receive map resource.
 */
 
static void load_map(int rid,const void *src,int srcc) {
  struct rom_map rmap;
  if (rom_map_decode(&rmap,src,srcc)<0) return;
  if (g.mapc>=MAP_LIMIT) {
    fprintf(stderr,"Too many maps.\n");
    return;
  }
  if ((rmap.w!=NS_sys_mapw)||(rmap.h!=NS_sys_maph)) {
    fprintf(stderr,"map:%d incorrect size %dx%d, expected %dx%d\n",rid,rmap.w,rmap.h,NS_sys_mapw,NS_sys_maph);
    return;
  }
  struct map *map=g.mapv+g.mapc++;
  map->rid=rid;
  map->rocellv=rmap.v;
  map->cmdv=rmap.cmdv;
  map->cmdc=rmap.cmdc;
}

/* Receive sprite resource.
 */
 
static void load_sprite(int rid,const void *src,int srcc) {
  struct rom_sprite rspr;
  if (rom_sprite_decode(&rspr,src,srcc)<0) return;
  if (g.sprdefc>=SPRDEF_LIMIT) {
    fprintf(stderr,"Too many sprite resources.\n");
    return;
  }
  const struct sprite_type *type=0;
  struct rom_command_reader reader={.v=rspr.cmdv,.c=rspr.cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_sprite_sprtype: type=sprite_type_by_id((cmd.argv[0]<<8)|cmd.argv[1]); break;
    }
  }
  if (!type) {
    fprintf(stderr,"sprite:%d: Type unknown.\n",rid);
    return;
  }
  struct sprdef *sprdef=g.sprdefv+g.sprdefc++;
  sprdef->rid=rid;
  sprdef->cmdv=rspr.cmdv;
  sprdef->cmdc=rspr.cmdc;
  sprdef->type=type;
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
      case EGG_TID_tilesheet: if (res->rid==RID_tilesheet_tiles) load_tilesheet(res->v,res->c); break;
      case EGG_TID_map: load_map(res->rid,res->v,res->c); break;
      case EGG_TID_sprite: load_sprite(res->rid,res->v,res->c); break;
    }
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
  
  // Load the home map.
  if (enter_map(RID_map_home,TRANSITION_NONE)<0) {
    fprintf(stderr,"Loading map:%d failed.\n",RID_map_home);
    return -1;
  }
  
  return 0;
}

/* Enter map.
 */
 
int enter_map(int rid,int transition) {
  struct map *map=map_by_id(rid);
  if (!map) return -1;
  g.map=map;
  
  sprites_kill_all();
  //TODO How are we managing the hero and princess sprites? They either persist across maps or get recreated.
  
  struct rom_command_reader reader={.v=g.map->cmdv,.c=g.map->cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_sprite: {
          uint8_t x=cmd.argv[0];
          uint8_t y=cmd.argv[1];
          uint16_t rid=(cmd.argv[2]<<8)|cmd.argv[3];
          uint32_t arg=(cmd.argv[4]<<24)|(cmd.argv[5]<<16)|(cmd.argv[6]<<8)|cmd.argv[7];
          sprite_spawn(x+0.5,y+0.5,rid,0,arg);
        } break;
      case CMD_map_field: {
          uint16_t k=(cmd.argv[0]<<8)|cmd.argv[1];
          uint16_t v=(cmd.argv[2]<<8)|cmd.argv[3];
          store_set(k,v);
        } break;
    }
  }
  
  //TODO Prepare transition.
  
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
    if (v&(i<<1)) (*dst)|=mask; else (*dst)&=~mask;
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
  struct listener *listener=g.listenerv;
  int i=g.listenerc;
  for (;i-->0;listener++) {
    if (listener->k!=k) continue;
    listener->cb(k,v,listener->userdata);
  }
  return v;
}
