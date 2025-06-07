/* sprite_boss.c
 * If NS_fld_boss_dead is set, we self-delete at init.
 * We start the rescue sequence when killed, including spawing the Princess.
 */
 
#include "game/game.h"

static void boss_begin_CENTIPEDE(struct sprite *sprite);

/* Form progresses in the order listed here.
 * Its count of legs is 2**n (treating a snake as a single "leg", not sure what zoologists would say about that).
 * It is important that we maintain mathematical rigor re leg count, because I said so.
 */
#define BOSS_FORM_CENTIPEDE 4
#define BOSS_FORM_SPIDER 3
#define BOSS_FORM_CENTAUR 2
#define BOSS_FORM_OSTRICH 1
#define BOSS_FORM_SNAKE 0

#define BOSS_HP 1 /* Per form. Kind of has to be just 1, otherwise you'd need more than 8 peppers to kill me. */
#define BOSS_HURT_TIME 1.500 /* Pepperfire lasts about 1.25 s */
#define BOSS_COOLDOWN_TIME 0.750 /* Important that we not start moving until the scrolling transition finishes. */

#define BOSS_CENTIBUFFER_LEN 40

struct sprite_boss {
  struct sprite hdr;
  int form;
  int hp; // counts down to the next form change
  uint8_t tileid0;
  double hurtclock; // counts down while hurt
  double cooldown; // counts down
  double animclock;
  int animframe; // 0,1
  struct centipos { int x,y; uint8_t xform; } centibufferv[BOSS_CENTIBUFFER_LEN];
  int centibufferp;
  double dx,dy;
  double dstx,dsty;
  double clock; // For form's use. Resets to 0 on form changes.
  int wait; // CENTAUR
  int charge; // CENTAUR
  double fireclock; // OSTRICH
  int listenerid;
};

#define SPRITE ((struct sprite_boss*)sprite)

/* Delete.
 */
 
static void _boss_del(struct sprite *sprite) {
  store_unlisten(SPRITE->listenerid);
}

/* Notify hero dead.
 */
 
static void boss_on_death(int k,int v,void *userdata) {
  struct sprite *sprite=userdata;
  if (SPRITE->form!=BOSS_FORM_CENTIPEDE) {
    SPRITE->form=BOSS_FORM_CENTIPEDE;
    SPRITE->hp=BOSS_HP;
    SPRITE->cooldown=BOSS_COOLDOWN_TIME;
    SPRITE->clock=0.0;
    boss_begin_CENTIPEDE(sprite);
  }
}

/* Setup.
 */

static int _boss_init(struct sprite *sprite) {

  if (store_get(NS_fld_boss_dead)) {
    fprintf(stderr,"Boss is dead.\n");
    sprite->defunct=1;
    return 0;
  }

  SPRITE->tileid0=sprite->tileid;
  SPRITE->form=BOSS_FORM_CENTIPEDE;
  SPRITE->hp=BOSS_HP;
  SPRITE->cooldown=BOSS_COOLDOWN_TIME;
  SPRITE->listenerid=store_listen(NS_fld_death_count,boss_on_death,sprite);
  return 0;
}

/* How far can we travel in a given direction, based only on the map.
 * We never allow travel onto a one-meter edge.
 */
 
static int boss_freedom(int x,int y,int dx,int dy) {
  if (!g.map) return 0;
  int freedom=0;
  for (;;) {
    x+=dx;
    y+=dy;
    if ((x<1)||(y<1)||(x>=NS_sys_mapw-1)||(y>=NS_sys_maph-1)) return freedom;
    uint8_t physics=g.physics[g.map->cellv[y*NS_sys_mapw+x]];
    if (physics!=NS_physics_vacant) return freedom;
    freedom++;
  }
}

/* CENTIPEDE.
 */
 
static void boss_begin_CENTIPEDE(struct sprite *sprite) {
  sprite->phl=-0.5;
  sprite->phr=0.5;
  sprite->pht=-0.5;
  sprite->phb=0.5;
  SPRITE->centibufferp=0;
  SPRITE->centibufferv[0].x=0;
  SPRITE->centibufferv[0].y=0;
}
 
static void boss_choose_centipede_direction(struct sprite *sprite) {
  int col=(int)sprite->x,row=(int)sprite->y;
  if (g.map&&(col>=0)&&(row>=0)&&(col<NS_sys_mapw)&&(row<NS_sys_maph)) {
    uint8_t candidatev[4];
    int candidatec=0;
    if (boss_freedom(col,row,0,-1)>=2) candidatev[candidatec++]=0x40;
    if (boss_freedom(col,row,-1,0)>=2) candidatev[candidatec++]=0x10;
    if (boss_freedom(col,row,1,0)>=2) candidatev[candidatec++]=0x08;
    if (boss_freedom(col,row,0,1)>=2) candidatev[candidatec++]=0x02;
    if (candidatec>=1) {
      SPRITE->dx=SPRITE->dy=0.0;
      switch (candidatev[rand()%candidatec]) {
        case 0x40: SPRITE->dy=-1.0; break;
        case 0x10: SPRITE->dx=-1.0; break;
        case 0x08: SPRITE->dx=1.0; break;
        case 0x02: SPRITE->dy=1.0; break;
      }
    }
  }
  SPRITE->clock=0.250+((rand()&0xffff)*0.500)/65535.0;
}
 
static void boss_update_CENTIPEDE(struct sprite *sprite,double elapsed) {
  if ((SPRITE->clock-=elapsed)<0.0) {
    boss_choose_centipede_direction(sprite);
  }
  const double speed=5.0;
  if (sprite_move(sprite,SPRITE->dx*elapsed*speed,SPRITE->dy*elapsed*speed)<2) {
    boss_choose_centipede_direction(sprite);
  }
}

/* SPIDER.
 */
 
static void boss_begin_SPIDER(struct sprite *sprite) {
  if (sprite->x<NS_sys_mapw*0.5) SPRITE->dx=1.0;
  else SPRITE->dx=-1.0;
  const double horzprotect=3.0;
  if (sprite->x<horzprotect) sprite->x=horzprotect;
  else if (sprite->x>NS_sys_mapw-horzprotect) sprite->x=NS_sys_mapw-horzprotect;
  sprite->phl=-1.0;
  sprite->phr=1.0;
  SPRITE->fireclock=0.200+((rand()&0xffff)*1.000)/65535.0;
}
 
static void boss_update_SPIDER(struct sprite *sprite,double elapsed) {
  sprite_move(sprite,0.0,-3.0*elapsed); // Back up to the top.
  if (sprite_move(sprite,SPRITE->dx*elapsed*5.0,0.0)<2) {
    SPRITE->dx*=-1.0;
  }
  if ((SPRITE->fireclock-=elapsed)<0.0) {
    SPRITE->fireclock=0.200+((rand()&0xffff)*1.000)/65535.0;
    struct sprite *missile=sprite_spawn(sprite->x,sprite->y+1.0,0,&sprite_type_missile,0);
    if (missile) {
      sprite_missile_setup(missile,0.0,9.0);
      missile->tileid=SPRITE->tileid0+0x10;
    }
  }
}

/* CENTAUR.
 */
 
static void boss_begin_CENTAUR(struct sprite *sprite) {
  // Whichever horizontal edge we're closer to, set up to approach that one.
  if (sprite->x<NS_sys_mapw*0.5) {
    sprite->xform=EGG_XFORM_XREV;
    SPRITE->dx=-1.0;
  } else {
    sprite->xform=0;
    SPRITE->dx=1.0;
  }
  if (sprite->y<NS_sys_maph*0.5) {
    SPRITE->dy=1.0;
  } else {
    SPRITE->dy=-1.0;
  }
  sprite->phl=-0.750;
  sprite->pht=-0.750;
  sprite->phr=0.750;
  sprite->phb=0.750;
  sprite->y+=0.500; // We were spider before, and he is narrower vertically. Get my head out of the wall.
  SPRITE->wait=1; // Forbid charging until we've backed into the wall.
}
 
static void boss_update_CENTAUR(struct sprite *sprite,double elapsed) {
  if (SPRITE->charge) {
    if (sprite_move(sprite,SPRITE->dx*13.0*elapsed,0.0)<2) {
      if (
        ((SPRITE->dx>0.0)&&(sprite->x<NS_sys_mapw-3.0))||
        ((SPRITE->dx<0.0)&&(sprite->x>3.0))
      ) { // interrupted. retreat back to the previous wall
        SPRITE->wait=1;
      } else {
        sprite->xform^=EGG_XFORM_XREV;
      }
      SPRITE->charge=0;
      SPRITE->dx*=-1.0;
    }
  } else {
    if (sprite_move(sprite,0.0,SPRITE->dy*4.0*elapsed)<2) {
      SPRITE->dy*=-1.0;
    }
    if (SPRITE->wait) {
      if (sprite_move(sprite,SPRITE->dx*3.0*elapsed,0.0)<2) {
        SPRITE->wait=0;
      }
    } else if (g.hero) {
      const double radius=0.750;
      double dy=g.hero->y-sprite->y;
      if ((dy>=-radius)&&(dy<=radius)) {
        if (sprite->xform&&(g.hero->x>sprite->x)) { // charge right
          SPRITE->charge=1;
          SPRITE->dx=1.0;
        } else if (!sprite->xform&&(g.hero->x<sprite->x)) { // charge left
          SPRITE->charge=1;
          SPRITE->dx=-1.0;
        }
      }
    }
  }
}

/* OSTRICH.
 */
 
static void boss_random_direction(struct sprite *sprite) {
  double t=((rand()&0xffff)*M_PI*2.0)/65535.0;
  SPRITE->dx=cos(t);
  SPRITE->dy=-sin(t);
}
 
static void boss_begin_OSTRICH(struct sprite *sprite) {
  boss_random_direction(sprite);
  sprite->phl=-0.333;
  sprite->phr=0.333;
  sprite->pht=-1.0;
  sprite->phb=1.0;
  SPRITE->clock=0.250;
}
 
static void boss_update_OSTRICH(struct sprite *sprite,double elapsed) {
  if (SPRITE->fireclock>0.0) {
    SPRITE->fireclock-=elapsed;
  } else if ((SPRITE->clock-=elapsed)<0.0) {
    SPRITE->clock=0.200+((rand()&0xffff)*1.000)/65535.0;
    SPRITE->fireclock=0.333;
    double x=sprite->x;
    double y=sprite->y-0.5;
    if (sprite->xform) x+=1.0; else x-=1.0;
    struct sprite *bubble=sprite_spawn(x,y,0,&sprite_type_bubble,1); // args 1 for no warmup
    boss_random_direction(sprite);
  } else {
    double speed=8.0;
    if (sprite_move(sprite,SPRITE->dx*elapsed*speed,0.0)<2) SPRITE->dx*=-1.0;
    if (sprite_move(sprite,0.0,SPRITE->dy*elapsed*speed)<2) SPRITE->dy*=-1.0;
    if ((sprite->x<1.0)&&(SPRITE->dx<0.0)) SPRITE->dx*=-1.0;
    if ((sprite->x>NS_sys_mapw-1.0)&&(SPRITE->dx>0.0)) SPRITE->dx*=-1.0;
    if ((sprite->y<1.0)&&(SPRITE->dy<0.0)) SPRITE->dy*=-1.0;
    if ((sprite->y>NS_sys_maph-1.0)&&(SPRITE->dy>0.0)) SPRITE->dy*=-1.0;
    if (g.hero) {
      if (g.hero->x>sprite->x) sprite->xform=EGG_XFORM_XREV;
      else sprite->xform=0;
    }
  }
}

/* SNAKE.
 */
 
static void boss_begin_SNAKE(struct sprite *sprite) {
  sprite->phl=-0.750;
  sprite->phr=0.750;
  sprite->pht=-0.333;
  sprite->phb=0.333;
  if (sprite->xform&EGG_XFORM_XREV) SPRITE->dx=1.0;
  else SPRITE->dx=-1.0;
  if (rand()&1) SPRITE->dy=-1.0;
  else SPRITE->dy=1.0;
}
 
static void boss_update_SNAKE(struct sprite *sprite,double elapsed) {
  if (sprite_move(sprite,0.0,SPRITE->dy*3.0*elapsed)<2) SPRITE->dy=-SPRITE->dy;
  if (sprite_move(sprite,SPRITE->dx*9.0*elapsed,0.0)<2) {
    SPRITE->dx=-SPRITE->dx;
    if (SPRITE->dx<0.0) sprite->xform=0;
    else sprite->xform=EGG_XFORM_XREV;
  }
  if (sprite->y<1.0) SPRITE->dy=1.0;
  else if (sprite->y>NS_sys_maph-1.0) SPRITE->dy=-1.0;
}

/* Nonzero if we are close enough to hurt the hero, and in a state to do so.
 */
 
static int boss_check_damage(struct sprite *sprite) {
  if (SPRITE->hurtclock>0.0) return 0; // nope, fair's fair.
  if (!g.hero) return 0;
  double affordance=0.010; // We're both solid. It's a hit if our hitboxes are this close.
  double dx=g.hero->x-sprite->x;
  if (dx>sprite->phr-g.hero->phl+affordance) return 0;
  if (dx<sprite->phl-g.hero->phr-affordance) return 0;
  double dy=g.hero->y-sprite->y;
  if (dy>sprite->phb-g.hero->pht+affordance) return 0;
  if (dy<sprite->pht-g.hero->phb-affordance) return 0;
  return 1;
}

/* Update.
 */
 
static void _boss_update(struct sprite *sprite,double elapsed) {
  if (SPRITE->hurtclock>0.0) {
    SPRITE->hurtclock-=elapsed;
  }
  if (SPRITE->cooldown>0.0) {
    SPRITE->cooldown-=elapsed;
    return;
  }
  if ((SPRITE->animclock-=elapsed)<0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
  }
  if (boss_check_damage(sprite)) {
    g.hero->type->hurt(g.hero,sprite);
  }
  switch (SPRITE->form) {
    case BOSS_FORM_CENTIPEDE: boss_update_CENTIPEDE(sprite,elapsed); break;
    case BOSS_FORM_SPIDER: boss_update_SPIDER(sprite,elapsed); break;
    case BOSS_FORM_CENTAUR: boss_update_CENTAUR(sprite,elapsed); break;
    case BOSS_FORM_OSTRICH: boss_update_OSTRICH(sprite,elapsed); break;
    case BOSS_FORM_SNAKE: boss_update_SNAKE(sprite,elapsed); break;
  }
}

/* Render per form.
 */
 
static void boss_render_CENTIPEDE(struct sprite *sprite,int x,int y) {

  /* During the cooldown, draw only my head.
   * This coincides roughly with the entry transition animation.
   * And we're tracking position by render coords, which are adjusted during that transition.
   */
  if (SPRITE->cooldown>0.0) {
    graf_draw_tile(&g.graf,g.texid_sprites,x,y,sprite->tileid,sprite->xform);
    return;
  }
  
  /* If the first space in our ring buffer is zeroes, overwrite it all with current position.
   * This should happen exactly once, the first time we're ready to render.
   */
  if (!SPRITE->centibufferp&&!SPRITE->centibufferv[0].x&&!SPRITE->centibufferv[0].y) {
    struct centipos *centipos=SPRITE->centibufferv;
    int i=BOSS_CENTIBUFFER_LEN;
    for (;i-->0;centipos++) {
      centipos->x=x;
      centipos->y=y;
    }
  }
  
  /* Draw 4 body segments at uniform intervals backward in our step history.
   */
  int offset=-BOSS_CENTIBUFFER_LEN,d=BOSS_CENTIBUFFER_LEN/4,i=4;
  for (;i-->0;offset+=d) {
    int bufp=SPRITE->centibufferp+offset;
    if (bufp<0) bufp+=BOSS_CENTIBUFFER_LEN;
    struct centipos *centipos=SPRITE->centibufferv+bufp;
    uint8_t tileid=SPRITE->tileid0+1+SPRITE->animframe;
    graf_draw_tile(&g.graf,g.texid_sprites,centipos->x,centipos->y,tileid,centipos->xform);
  }
  
  /* Draw the head.
   * Important that we do body first, then head; they do overlap.
   */
  uint8_t xform=0;
  if (SPRITE->dx<0.0) ;
  else if (SPRITE->dx>0.0) xform=EGG_XFORM_XREV;
  else if (SPRITE->dy<0.0) xform=EGG_XFORM_SWAP;
  else if (SPRITE->dy>0.0) xform=EGG_XFORM_SWAP|EGG_XFORM_XREV;
  graf_draw_tile(&g.graf,g.texid_sprites,x,y,SPRITE->tileid0,xform);
  
  /* Drop new position into (centibuffer) and advance it.
   */
  SPRITE->centibufferv[SPRITE->centibufferp++]=(struct centipos){.x=x,.y=y,.xform=xform};
  if (SPRITE->centibufferp>=BOSS_CENTIBUFFER_LEN) SPRITE->centibufferp=0;
}
 
static void boss_render_SPIDER(struct sprite *sprite,int x,int y) {
  uint8_t tileid=SPRITE->tileid0+3+SPRITE->animframe;
  graf_draw_tile(&g.graf,g.texid_sprites,x-(NS_sys_tilesize>>1),y,tileid,0);
  graf_draw_tile(&g.graf,g.texid_sprites,x+(NS_sys_tilesize>>1),y,tileid,EGG_XFORM_XREV);
}
 
static void boss_render_CENTAUR(struct sprite *sprite,int x,int y) {
  uint8_t tileid=SPRITE->tileid0+5;
  if (SPRITE->animframe) tileid+=2;
  int srcx=(tileid&0x0f)*NS_sys_tilesize;
  int srcy=(tileid>>4)*NS_sys_tilesize;
  graf_draw_decal(&g.graf,g.texid_sprites,x-NS_sys_tilesize,y-NS_sys_tilesize,srcx,srcy,NS_sys_tilesize<<1,NS_sys_tilesize<<1,sprite->xform);
}
 
static void boss_render_OSTRICH(struct sprite *sprite,int x,int y) {
  uint8_t headtile=SPRITE->tileid0+9;
  uint8_t foottile=headtile+0x10;
  if (SPRITE->fireclock>0.0) {
    headtile+=1;
  } else {
    foottile+=SPRITE->animframe;
  }
  graf_draw_tile(&g.graf,g.texid_sprites,x,y-(NS_sys_tilesize>>1),headtile,sprite->xform);
  graf_draw_tile(&g.graf,g.texid_sprites,x,y+(NS_sys_tilesize>>1),foottile,sprite->xform);
}
 
static void boss_render_SNAKE(struct sprite *sprite,int x,int y) {
  uint8_t tileid=SPRITE->tileid0+0x11+SPRITE->animframe*2;
  if (sprite->xform) {
    graf_draw_tile(&g.graf,g.texid_sprites,x-(NS_sys_tilesize>>1),y,tileid+1,EGG_XFORM_XREV);
    graf_draw_tile(&g.graf,g.texid_sprites,x+(NS_sys_tilesize>>1),y,tileid,EGG_XFORM_XREV);
  } else {
    graf_draw_tile(&g.graf,g.texid_sprites,x-(NS_sys_tilesize>>1),y,tileid,0);
    graf_draw_tile(&g.graf,g.texid_sprites,x+(NS_sys_tilesize>>1),y,tileid+1,0);
  }
}

/* Render, generic.
 */
 
static void _boss_render(struct sprite *sprite,int x,int y) {

  // Hurt highlight is effected generically, regardless of form.
  if (SPRITE->hurtclock>0.0) {
    uint32_t color=nes_colors[24];
    if (((int)(SPRITE->hurtclock*10.0))&1) color=nes_colors[1];
    graf_set_tint(&g.graf,color);
  }

  switch (SPRITE->form) {
    case BOSS_FORM_CENTIPEDE: boss_render_CENTIPEDE(sprite,x,y); break;
    case BOSS_FORM_SPIDER: boss_render_SPIDER(sprite,x,y); break;
    case BOSS_FORM_CENTAUR: boss_render_CENTAUR(sprite,x,y); break;
    case BOSS_FORM_OSTRICH: boss_render_OSTRICH(sprite,x,y); break;
    case BOSS_FORM_SNAKE: boss_render_SNAKE(sprite,x,y); break;
  }
  
  if (SPRITE->hurtclock>0.0) {
    graf_set_tint(&g.graf,0);
  }
}

/* Find the Princess's spawn command and create her.
 */
 
static void boss_spawn_princess() {
  if (!g.map) return;
  struct rom_command_reader reader={.v=g.map->cmdv,.c=g.map->cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    if (cmd.opcode!=CMD_map_sprite) continue;
    int rid=(cmd.argv[2]<<8)|cmd.argv[3];
    if (rid!=RID_sprite_princess) continue;
    sprite_spawn(cmd.argv[0]+0.5,cmd.argv[1]+0.5,rid,0,0);
    return;
  }
}

/* Hurt.
 */
 
static int _boss_hurt(struct sprite *sprite,struct sprite *assailant) {
  if (assailant->type==&sprite_type_bubble) return 0;
  if (SPRITE->hurtclock>0.0) return 0;
  SPRITE->hurtclock=BOSS_HURT_TIME;
  if (--(SPRITE->hp)<=0) {
    if (SPRITE->form==BOSS_FORM_SNAKE) {
      sprite->defunct=1;
      sprite_spawn(sprite->x,sprite->y,0,&sprite_type_soulballs,0x05000000);
      store_set(NS_fld_boss_dead,1);
      boss_spawn_princess();
      // TODO Cutscene?
    } else {
      SPRITE->form--;
      SPRITE->hp=BOSS_HP;
      SPRITE->cooldown=BOSS_COOLDOWN_TIME;
      SPRITE->clock=0.0;
      switch (SPRITE->form) {
        case BOSS_FORM_CENTIPEDE: boss_begin_CENTIPEDE(sprite); break;
        case BOSS_FORM_SPIDER: boss_begin_SPIDER(sprite); break;
        case BOSS_FORM_CENTAUR: boss_begin_CENTAUR(sprite); break;
        case BOSS_FORM_OSTRICH: boss_begin_OSTRICH(sprite); break;
        case BOSS_FORM_SNAKE: boss_begin_SNAKE(sprite); break;
      }
    }
  }
  return 1;
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_boss={
  .name="boss",
  .objlen=sizeof(struct sprite_boss),
  .del=_boss_del,
  .init=_boss_init,
  .update=_boss_update,
  .render=_boss_render,
  .hurt=_boss_hurt,
};
