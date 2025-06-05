#include "game/game.h"

/* Receive sprite resource.
 */
 
int load_sprite(int rid,const void *src,int srcc) {
  struct rom_sprite rspr;
  if (rom_sprite_decode(&rspr,src,srcc)<0) return -1;
  if (g.sprdefc>=SPRDEF_LIMIT) {
    fprintf(stderr,"Too many sprite resources.\n");
    return -1;
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
    return -1;
  }
  struct sprdef *sprdef=g.sprdefv+g.sprdefc++;
  sprdef->rid=rid;
  sprdef->cmdv=rspr.cmdv;
  sprdef->cmdc=rspr.cmdc;
  sprdef->type=type;
  return 0;
}

/* Delete.
 */
 
void sprite_del(struct sprite *sprite) {
  if (!sprite) return;
  
  if (sprite->type==&sprite_type_candy) {
    int i=g.candyc;
    while (i-->0) {
      if (g.candyv[i]==sprite) {
        g.candyc--;
        memmove(g.candyv+i,g.candyv+i+1,sizeof(void*)*(g.candyc-i));
      }
    }
  }
  
  if (sprite->type->del) sprite->type->del(sprite);
  free(sprite);
  
  if (g.hero==sprite) g.hero=0;
  if (g.princess==sprite) g.princess=0;
}

/* New.
 */
 
struct sprite *sprite_new(const struct sprite_type *type) {
  if (!type) return 0;
  if (type->objlen<(int)sizeof(struct sprite)) return 0;
  struct sprite *sprite=calloc(1,type->objlen);
  if (!sprite) return 0;
  sprite->type=type;
  return sprite;
}

/* Spawn.
 */

struct sprite *sprite_spawn(double x,double y,uint16_t rid,const struct sprite_type *type,uint32_t arg) {
  const void *cmdv=0;
  int cmdc=0;
  if (rid) {
    const struct sprdef *sprdef=sprdef_by_id(rid);
    if (!sprdef) return 0;
    cmdv=sprdef->cmdv;
    cmdc=sprdef->cmdc;
    type=sprdef->type;
  } else if (!type) return 0;
  struct sprite *sprite=sprite_new(type);
  if (!sprite) return 0;
  sprite->x=x;
  sprite->y=y;
  sprite->arg=arg;
  sprite->layer=128;
  sprite->phl=-0.5;
  sprite->pht=-0.5;
  sprite->phr=0.5;
  sprite->phb=0.5;
  
  if (cmdc) {
    struct rom_command_reader reader={.v=cmdv,.c=cmdc};
    struct rom_command cmd;
    while (rom_command_reader_next(&cmd,&reader)>0) {
      switch (cmd.opcode) {
        case CMD_sprite_solid: sprite->solid=1; break;
        case CMD_sprite_airborne: sprite->airborne=1; break;
        case CMD_sprite_monster: sprite->monster=1; break;
        case CMD_sprite_tile: {
            sprite->tileid=cmd.argv[0];
            sprite->xform=cmd.argv[1];
          } break;
        case CMD_sprite_layer: {
            sprite->layer=cmd.argv[0];
          } break;
      }
    }
    sprite->cmdv=cmdv;
    sprite->cmdc=cmdc;
  }
  
  if (type->init&&(type->init(sprite)<0)) {
    sprite_del(sprite);
    return 0;
  }
  
  if (g.spritec>=g.spritea) {
    int na=g.spritea+128;
    if (na>INT_MAX/sizeof(void*)) { sprite_del(sprite); return 0; }
    void *nv=realloc(g.spritev,sizeof(void*)*na);
    if (!nv) { sprite_del(sprite); return 0; }
    g.spritev=nv;
    g.spritea=na;
  }
  // TODO Insert by approximate rendering order.
  g.spritev[g.spritec++]=sprite;
  
  // The hero and princess sprites are special; they get tracked globally.
  if (type==&sprite_type_hero) g.hero=sprite;
  else if (type==&sprite_type_princess) g.princess=sprite;
  else if (type==&sprite_type_candy) {
    if (g.candyc<CANDY_LIMIT) g.candyv[g.candyc++]=sprite;
  }
  
  return sprite;
}

/* Kill all.
 */

void sprites_kill_all() {
  int i=g.spritec;
  while (i-->0) {
    struct sprite *sprite=g.spritev[i];
    sprite->defunct=1;
  }
}

/* Delete all except the globally-tracked hero and princess.
 */
 
void sprites_delete_volatile() {
  int i=g.spritec;
  while (i-->0) {
    struct sprite *sprite=g.spritev[i];
    if (sprite==g.hero) continue;
    if (sprite==g.princess) continue;
    g.spritec--;
    memmove(g.spritev+i,g.spritev+i+1,sizeof(void*)*(g.spritec-i));
    sprite_del(sprite);
  }
}

/* Update.
 */
 
void sprites_update(double elapsed) {
  int i=g.spritec,defunct=0;
  while (i-->0) {
    struct sprite *sprite=g.spritev[i];
    if (sprite->defunct) { defunct=1; continue; }
    if (sprite->type->update) sprite->type->update(sprite,elapsed);
    if (sprite->defunct) defunct=1;
  }
  int reprincess=0;
  if (defunct) {
    for (i=g.spritec;i-->0;) {
      struct sprite *sprite=g.spritev[i];
      if (!sprite->defunct) continue;
      g.spritec--;
      memmove(g.spritev+i,g.spritev+i+1,sizeof(void*)*(g.spritec-i));
      
      if (sprite->type==&sprite_type_princess) {
        sprite_spawn(sprite->x,sprite->y,0,&sprite_type_soulballs,0x06020000); // Human souls have six circles.
        reprincess=1;
      } else if (sprite->type==&sprite_type_hero) {
        sprite_spawn(sprite->x,sprite->y,0,&sprite_type_soulballs,0x07010000); // Witch souls have seven circles.
      }
      
      sprite_del(sprite);
    }
  }
  if (reprincess) session_respawn_princess();
  if (!g.hero) session_respawn_hero();
}

/* Render order.
 */
 
static int sprites_rendercmp(const struct sprite *a,const struct sprite *b) {
  if (a->layer<b->layer) return -1;
  if (a->layer>b->layer) return 1;
  if (a->y<b->y) return -1;
  if (a->y>b->y) return 1;
  return 0;
}

/* Sort to render order, single pass.
 */
 
static void sprites_sort_partial() {
  if (g.spritec<2) return;
  int first,last,i,d;
  if (g.sprites_sort_dir==1) {
    g.sprites_sort_dir=-1;
    first=0;
    last=g.spritec-1;
    d=1;
  } else {
    g.sprites_sort_dir=1;
    first=g.spritec-1;
    last=0;
    d=-1;
  }
  for (i=first;i!=last;i+=d) {
    int cmp=sprites_rendercmp(g.spritev[i],g.spritev[i+d]);
    if (cmp==d) {
      struct sprite *tmp=g.spritev[i];
      g.spritev[i]=g.spritev[i+d];
      g.spritev[i+d]=tmp;
    }
  }
}

/* Render.
 */
 
void sprites_render(int offx,int offy) {
  sprites_sort_partial();
  int i=0;
  struct sprite **p=g.spritev;
  for (;i<g.spritec;i++,p++) {
    struct sprite *sprite=*p;
    if (sprite->defunct) continue;
    int x=(int)(sprite->x*NS_sys_tilesize+0.5)+offx;
    int y=(int)(sprite->y*NS_sys_tilesize+0.5)+offy;
    if (sprite->type->render) {
      sprite->type->render(sprite,x,y);
    } else {
      graf_draw_tile(&g.graf,g.texid_sprites,x,y,sprite->tileid,sprite->xform);
    }
    if (sprite->summoning) {
      graf_draw_tile(&g.graf,g.texid_sprites,x,y,(g.framec&8)?0x27:0x28,0);
    }
  }
}

void sprites_render_volatile() {
  int i=0;
  struct sprite **p=g.spritev;
  for (;i<g.spritec;i++,p++) {
    struct sprite *sprite=*p;
    if (sprite->defunct) continue;
    if (sprite==g.hero) continue;
    if (sprite==g.princess) continue;
    int x=(int)(sprite->x*NS_sys_tilesize+0.5);
    int y=(int)(sprite->y*NS_sys_tilesize+0.5);
    if (sprite->type->render) {
      sprite->type->render(sprite,x,y);
    } else {
      graf_draw_tile(&g.graf,g.texid_sprites,x,y,sprite->tileid,sprite->xform);
    }
  }
}

/* Type by id.
 */

const struct sprite_type *sprite_type_by_id(uint16_t sprtype) {
  switch (sprtype) {
    #define _(tag) case NS_sprtype_##tag: return &sprite_type_##tag;
    SPRTYPE_FOR_EACH
    #undef _
  }
  return 0;
}

/* Resource by id.
 */
 
const struct sprdef *sprdef_by_id(int rid) {
  int lo=0,hi=g.sprdefc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct sprdef *q=g.sprdefv+ck;
         if (rid<q->rid) hi=ck;
    else if (rid>q->rid) lo=ck+1;
    else return q;
  }
  return 0;
}

/* Preferred monster target.
 */
 
struct sprite *get_preferred_monster_target() {
  if (g.princess) {
    if (
      (g.princess->x>=0.0)&&
      (g.princess->y>=0.0)&&
      (g.princess->x<NS_sys_mapw)&&
      (g.princess->y<NS_sys_maph)
    ) return g.princess;
  }
  return g.hero;
}

/* Preferred candy.
 */
 
int find_candy(const struct sprite *sprite) {
  // I reckon the overwhelming majority of the time there are zero or one candies, and that's an easy answer:
  if (g.candyc<1) return -1;
  if (g.candyc==1) return 0;
  // Otherwise consider the square distance to each candy, and approach the nearest:
  int bestp=0;
  double bestd=999.0;
  int i=g.candyc; while (i-->0) {
    struct sprite *candy=g.candyv[i];
    double dx=candy->x-sprite->x;
    double dy=candy->y-sprite->y;
    double d=dx*dx+dy*dy;
    if (d<bestd) {
      bestd=d;
      bestp=i;
    }
  }
  return bestp;
}

/* Prizes.
 */

struct sprite *spawn_prize(double x,double y) {
  int got_bomb=store_get(NS_fld_got_bomb);
  int got_pepper=store_get(NS_fld_got_pepper);
  int got_candy=store_get(NS_fld_got_candy);
  if (!got_bomb&&!got_pepper&&!got_candy) return 0; // Nothing eligible. How did you kill the monster?
  int qty_bomb=got_bomb?store_get(NS_fld_qty_bomb):0;
  int qty_pepper=got_pepper?store_get(NS_fld_qty_pepper):0;
  int qty_candy=got_candy?store_get(NS_fld_qty_candy):0;
  // If we have >=99 already, don't make more.
  if (qty_bomb>=99) got_bomb=0;
  if (qty_pepper>=99) got_pepper=0;
  if (qty_candy>=99) got_candy=0;
  if (!got_bomb&&!got_pepper&&!got_candy) return 0;
  // Pick randomly among the eligible items.
  int choice=rand()%(got_bomb+got_pepper+got_candy);
  if (got_bomb&&!choice--) choice=NS_fld_got_bomb;
  else if (got_pepper&&!choice--) choice=NS_fld_got_pepper;
  else choice=NS_fld_got_candy;
  return sprite_spawn(x,y,0,&sprite_type_prize,(choice<<24));
}
