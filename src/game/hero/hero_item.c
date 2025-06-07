#include "hero_internal.h"

/* Spawn one ring of single-serving flames for the pepper.
 * This is definitely more sprites than an NES would be comfortable with; not going to worry about it.
 * (also fwiw, I don't think the trigonometric functions would have been available to NES, would be an interesting* problem to solve).
 * [*] Not interesting enough to solve here, of course.
 *
//XXX for the old "ssflame" regime
 
static void hero_spawn_pepper_ring(struct sprite *sprite,double radius,int count,uint8_t delay_u44) {
  double t=0.0;
  double dt=(M_PI*2.0)/count;
  for (;count-->0;t+=dt) {
    double x=sprite->x+cos(t)*radius;
    double y=sprite->y-sin(t)*radius;
    struct sprite *ssflame=sprite_spawn(x,y,0,&sprite_type_ssflame,(delay_u44<<24));
  }
}
/**/

/* Pepper, bomb, candy: Check quantity, then create another sprite that does the real work.
 * Play "reject" if quantity zero, just as if nothing were equipped.
 */
 
static int hero_too_many_items_placed(struct sprite *sprite) {
  // Bomb and candy need to regulate their population somewhat, or the player could drop dozens of things at once.
  // That would be no problem for us, but it does kind of break the NES illusion.
  int c=0,i=g.spritec;
  struct sprite **p=g.spritev;
  for (;i-->0;p++) {
    struct sprite *other=*p;
    if (other->defunct) continue;
    if ((other->type==&sprite_type_bomb)||(other->type==&sprite_type_candy)) c++;
  }
  return (c>=8);
}
 
static void hero_pepper_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_pepper);
  if (qty<1) {
    egg_play_sound(RID_sound_reject);
    return;
  }
  store_set(NS_fld_qty_pepper,qty-1);
  egg_play_sound(RID_sound_pepper);
  /*XXX old "ssflame" regime
  hero_spawn_pepper_ring(sprite,1.0, 9,0x00);
  hero_spawn_pepper_ring(sprite,1.5,12,0x03);
  hero_spawn_pepper_ring(sprite,2.0,16,0x06);
  hero_spawn_pepper_ring(sprite,2.5,24,0x09);
  /**/
  /* new "pepperfire" regime. */
  sprite_spawn(sprite->x-0.5,sprite->y-1.0,0,&sprite_type_pepperfire,0);
  sprite_spawn(sprite->x+0.5,sprite->y-1.0,0,&sprite_type_pepperfire,0);
  sprite_spawn(sprite->x+1.0,sprite->y-0.5,0,&sprite_type_pepperfire,0);
  sprite_spawn(sprite->x+1.0,sprite->y+0.5,0,&sprite_type_pepperfire,0);
  sprite_spawn(sprite->x+0.5,sprite->y+1.0,0,&sprite_type_pepperfire,0);
  sprite_spawn(sprite->x-0.5,sprite->y+1.0,0,&sprite_type_pepperfire,0);
  sprite_spawn(sprite->x-1.0,sprite->y+0.5,0,&sprite_type_pepperfire,0);
  sprite_spawn(sprite->x-1.0,sprite->y-0.5,0,&sprite_type_pepperfire,0);
  SPRITE->item_cooldown=0.500;
}
 
static void hero_bomb_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_bomb);
  if ((qty<1)||hero_too_many_items_placed(sprite)) {
    egg_play_sound(RID_sound_reject);
    return;
  }
  double x=sprite->x+SPRITE->facedx;
  double y=sprite->y+SPRITE->facedy;
  struct sprite *bomb=sprite_spawn(x,y,0,&sprite_type_bomb,0);
  if (!bomb) return;
  store_set(NS_fld_qty_bomb,qty-1);
  egg_play_sound(RID_sound_bomb);
}
 
static void hero_candy_begin(struct sprite *sprite) {
  int qty=store_get(NS_fld_qty_candy);
  if ((qty<1)||hero_too_many_items_placed(sprite)) {
    egg_play_sound(RID_sound_reject);
    return;
  }
  double x=sprite->x+SPRITE->facedx;
  double y=sprite->y+SPRITE->facedy;
  struct sprite *candy=sprite_spawn(x,y,0,&sprite_type_candy,0);
  if (!candy) return;
  store_set(NS_fld_qty_candy,qty-1);
  egg_play_sound(RID_sound_candy);
  store_set(NS_fld_qty_candy,qty-1);
}

/* Broom.
 */
 
static void hero_broom_begin(struct sprite *sprite) {
  SPRITE->using_item=NS_fld_got_broom;
  sprite->airborne=1;
  // When flying, we can move in any direction but face dir is constrained to horizontal.
  if (!SPRITE->facedx) {
    if (SPRITE->indx) SPRITE->facedx=SPRITE->indx;
    else SPRITE->facedx=-1; // got to be one of them, whatever.
    SPRITE->facedy=0;
  }
}
 
void sprite_hero_force_broom(struct sprite *sprite) {
  hero_broom_begin(sprite);
}

static void hero_broom_end(struct sprite *sprite) {
  int x=(int)sprite->x;
  int y=(int)sprite->y;
  if ((x>=0)&&(y>=0)&&(x<NS_sys_mapw)&&(y<NS_sys_maph)) {
    uint8_t physics=g.physics[g.map->cellv[y*NS_sys_mapw+x]];
    if (physics==NS_physics_hole) {
      // Stay on the broom. We'll be repeatedly polled until it works.
      return;
    }
  }
  sprite->airborne=0;
  SPRITE->using_item=0;
}

/* Stopwatch.
 */
 
static void hero_stopwatch_begin(struct sprite *sprite) {
  SPRITE->using_item=NS_fld_got_stopwatch;
  SPRITE->stopwatch_clock=0.0;
  g.time_stopped=1;
}

static void hero_stopwatch_end(struct sprite *sprite) {
  SPRITE->using_item=0;
  g.time_stopped=0;
}

static void hero_stopwatch_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->stopwatch_clock-=elapsed)<0.0) {
    SPRITE->stopwatch_clock+=0.500;
    egg_play_sound(RID_sound_stopwatch);
  }
}

/* Camera: Entirely defer to modal_camera.
 */
 
static void hero_camera_begin(struct sprite *sprite) {
  modal_new_camera();
  SPRITE->indx=SPRITE->indy=0;
  SPRITE->walking=0;
}

/* Snowglobe.
 */
 
static void hero_snowglobe_begin(struct sprite *sprite) {
  SPRITE->using_item=NS_fld_got_snowglobe;
  SPRITE->walking=0;
}

static void hero_snowglobe_end(struct sprite *sprite) {
  SPRITE->using_item=0;
}

static void hero_snowglobe_update(struct sprite *sprite,double elapsed) {
  // Might make more sense to do this at hero_sprite.c:hero_update_ind(), but I'm doing here to keep snowglobe stuff together.
  uint8_t nv=0;
  if (SPRITE->indx&&SPRITE->indy) ; // single direction only
  else if (SPRITE->indy<0) nv=0x40;
  else if (SPRITE->indx<0) nv=0x10;
  else if (SPRITE->indx>0) nv=0x08;
  else if (SPRITE->indy>0) nv=0x02;
  if (SPRITE->snowglobe!=nv) {
    if (SPRITE->snowglobe=nv) dw_earthquake(-SPRITE->indx,-SPRITE->indy);
  }
}

/* Wand.
 */
 
static void hero_unsummon(struct sprite *sprite,struct sprite *pumpkin) {
  pumpkin->summoning=0;
  
  // If the pumpkin landed in a hole, kill it and create a splash. Doesn't apply to airbone pumpkins like bubble.
  if (!pumpkin->airborne&&(pumpkin->x>0.0)&&(pumpkin->y>0.0)) {
    int col=(int)pumpkin->x,row=(int)pumpkin->y;
    if ((col<NS_sys_mapw)&&(row<NS_sys_maph)) {
      uint8_t physics=g.physics[g.map->cellv[row*NS_sys_mapw+col]];
      if (physics==NS_physics_hole) {
        pumpkin->defunct=1;
        sprite_spawn(pumpkin->x,pumpkin->y,0,&sprite_type_splash,0);
        egg_play_sound(RID_sound_splash);
        return;
      }
    }
  }
}
 
static void hero_wand_begin(struct sprite *sprite) {
  SPRITE->using_item=NS_fld_got_wand;
  SPRITE->walking=0;
  SPRITE->indx=SPRITE->indy=0;
}

static void hero_wand_end(struct sprite *sprite) {
  SPRITE->using_item=0;
  int i=g.spritec;
  while (i-->0) {
    if (g.spritev[i]->summoning) hero_unsummon(sprite,g.spritev[i]);
  }
  hero_update_ind(sprite);
}

static void hero_wand_update(struct sprite *sprite,double elapsed) {
  const double radius=0.500;
  double l=sprite->x-radius,r=sprite->x+radius,t=sprite->y-radius,b=sprite->y+radius;
  if (SPRITE->facedx<0) l=0.0;
  else if (SPRITE->facedx>0) r=NS_sys_mapw;
  else if (SPRITE->facedy<0) t=0.0;
  else b=NS_sys_maph;
  
  // First determine what we are already summoning; we bias toward keeping it.
  struct sprite *pvpumpkin=0;
  int i=g.spritec;
  while (i-->0) {
    struct sprite *q=g.spritev[i];
    if (q->defunct) continue;
    if (q->summoning) {
      pvpumpkin=q;
      break; // There can't be more than one (and if there is, there's no guidance on which to prefer so whatever).
    }
  }
  
  // Determine which pumpkin we ought to be summoning.
  struct sprite *pumpkin=0;
  double bestdistance=999.0;
  for (i=g.spritec;i-->0;) {
    struct sprite *q=g.spritev[i];
    if (q->defunct) continue;
    if (q==sprite) continue;
    
    if (q->decorative) continue;
    if (q->type==&sprite_type_selfie) continue;
    //TODO Which sprites are summonable? Surely not all of them.
    
    // It must be in the box to be eligible, even if we were already holding it. (determined pumpkins can escape our grasp).
    if (q->x<l) continue;
    if (q->y<t) continue;
    if (q->x>r) continue;
    if (q->y>b) continue;
    
    // If this is the one we're already carrying, it's eligible so that's the answer.
    if (pvpumpkin==q) {
      pumpkin=q;
      break;
    }
    
    // Prefer the one nearest me. If something else steps in the way, it won't interrupt.
    double distance=999.0;
         if (SPRITE->facedx<0) distance=sprite->x-q->x;
    else if (SPRITE->facedx>0) distance=q->x-sprite->x;
    else if (SPRITE->facedy<0) distance=sprite->y-q->y;
    else if (SPRITE->facedy>0) distance=q->y-sprite->y;
    if (distance<bestdistance) {
      bestdistance=distance;
      pumpkin=q;
    }
  }
  
  /* Set (summoning) and if there's a current pumpkin, draw it near.
   * Temporarily make it solid and airborne, to let physics work sensibly.
   */
  if (pvpumpkin&&(pvpumpkin!=pumpkin)) hero_unsummon(sprite,pvpumpkin);
  if (pumpkin) {
    pumpkin->summoning=1;
    double dx=pumpkin->x-sprite->x;
    double dy=pumpkin->y-sprite->y;
    const double min=1.0;
    const double speed=1.500;
    int pvsolid=pumpkin->solid; pumpkin->solid=1;
    int pvairborne=pumpkin->airborne; pumpkin->airborne=1;
    if (SPRITE->facedx<0) {
      if (dx<-min) sprite_move(pumpkin,speed*elapsed,0.0);
    } else if (SPRITE->facedx>0) {
      if (dx>min) sprite_move(pumpkin,-speed*elapsed,0.0);
    } else if (SPRITE->facedy<0) {
      if (dy<-min) sprite_move(pumpkin,0.0,speed*elapsed);
    } else {
      if (dy>min) sprite_move(pumpkin,0.0,-speed*elapsed);
    }
    pumpkin->airborne=pvairborne;
    pumpkin->solid=pvsolid;
  }
}

/* Begin item.
 */
 
void hero_item_begin(struct sprite *sprite) {

  if (SPRITE->item_cooldown>0.0) return;

  /* Don't begin using an item if one is already in use.
   * This is unusual, because normally releasing A ends the item usage (plus most items are not sustained).
   * But it can happen for the Broom, if the player released A while over a hole.
   * So it's a very passive noop.
   */
  if (SPRITE->using_item) return;
  
  /* Normally if you press A and there's nothing equipped, a rejection sound is in order.
   */
  int equipped=store_get(NS_fld_equipped);
  if ((equipped<NS_fld_got_broom)||(equipped>NS_fld_got_candy)||!store_get(equipped)) {
    egg_play_sound(RID_sound_reject);
    return;
  }

  switch (equipped) {
    case NS_fld_got_broom: hero_broom_begin(sprite); break;
    case NS_fld_got_pepper: hero_pepper_begin(sprite); break;
    case NS_fld_got_compass: break; // nothing, it's entirely passive
    case NS_fld_got_stopwatch: hero_stopwatch_begin(sprite); break;
    case NS_fld_got_camera: hero_camera_begin(sprite); break;
    case NS_fld_got_snowglobe: hero_snowglobe_begin(sprite); break;
    case NS_fld_got_wand: hero_wand_begin(sprite); break;
    case NS_fld_got_bomb: hero_bomb_begin(sprite); break;
    case NS_fld_got_candy: hero_candy_begin(sprite); break;
  }
}

/* End item.
 */
 
void hero_item_end(struct sprite *sprite) {
  switch (SPRITE->using_item) {
    case NS_fld_got_broom: hero_broom_end(sprite); break;
    case NS_fld_got_stopwatch: hero_stopwatch_end(sprite); break;
    case NS_fld_got_snowglobe: hero_snowglobe_end(sprite); break;
    case NS_fld_got_wand: hero_wand_end(sprite); break;
    default: SPRITE->using_item=0;
  }
}

/* Continue item use.
 */
 
void hero_item_update(struct sprite *sprite,double elapsed) {
  switch (SPRITE->using_item) {
    case NS_fld_got_stopwatch: hero_stopwatch_update(sprite,elapsed); break;
    case NS_fld_got_snowglobe: hero_snowglobe_update(sprite,elapsed); break;
    case NS_fld_got_wand: hero_wand_update(sprite,elapsed); break;
  }
}
