/* sprite_explode.c
 * Manages a bomb explosion: The entire visual effect, and also triggers damage to nearby sprites.
 */
 
#include "game/game.h"

#define EXPLODE_SMOKEC 16
#define EXPLODE_RADIUS_MIN 0.125
#define EXPLODE_RADIUS_MAX 0.500
#define EXPLODE_DAMAGE_RADIUS 2.000

struct sprite_explode {
  struct sprite hdr;
  double ttl;
};

#define SPRITE ((struct sprite_explode*)sprite)

static int _explode_init(struct sprite *sprite) {
  sprite->layer=190;
  sprite->airborne=1;
  sprite->decorative=1;
  SPRITE->ttl=1.500;
  
  egg_play_sound(RID_sound_explode);
  
  int i=EXPLODE_SMOKEC;
  while (i-->0) {
    double t=((rand()&0xffff)*M_PI)/32768.0;
    double r=EXPLODE_RADIUS_MIN+((rand()&0xffff)*(EXPLODE_RADIUS_MAX-EXPLODE_RADIUS_MIN))/65535.0;
    double dx=cos(t)*r;
    double dy=-sin(t)*r;
    struct sprite *smoke=sprite_spawn(sprite->x+dx,sprite->y+dy,0,&sprite_type_smoke,0);
    if (!smoke) continue;
    const double faster=3.0;
    sprite_smoke_setup(smoke,dx*faster,dy*faster);
  }
  
  struct sprite **p=g.spritev;
  for (i=g.spritec;i-->0;p++) {
    struct sprite *victim=*p;
    if (victim->defunct) continue;
    if (!victim->type->hurt) continue;
    
    double dx=victim->x-sprite->x;
    double dy=victim->y-sprite->y;
    double d2=dx*dx+dy*dy;
    if (d2>EXPLODE_DAMAGE_RADIUS*EXPLODE_DAMAGE_RADIUS) continue;
    
    victim->type->hurt(victim,sprite);
  }
  
  return 0;
}

static void _explode_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->ttl-=elapsed)<=0.0) {
    sprite->defunct=1;
  }
  sprite->y-=0.500*elapsed;
}

static void _explode_render(struct sprite *sprite,int x,int y) {
  int srccol=3;
  if ((int)(SPRITE->ttl*10.0)&1) srccol+=2;
  graf_draw_decal(&g.graf,g.texid_sprites,x-NS_sys_tilesize,y-NS_sys_tilesize,srccol*NS_sys_tilesize,4*NS_sys_tilesize,NS_sys_tilesize*2,NS_sys_tilesize*2,0);
}

const struct sprite_type sprite_type_explode={
  .name="explode",
  .objlen=sizeof(struct sprite_explode),
  .init=_explode_init,
  .update=_explode_update,
  .render=_explode_render,
};
