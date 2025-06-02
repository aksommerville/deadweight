#include "game/game.h"

#define PAUSE_PHASE_BEGIN 1
#define PAUSE_PHASE_RUN 2
#define PAUSE_PHASE_END 3

#define PAUSE_BEGIN_TIME 0.400
#define PAUSE_END_TIME   0.200

#define PAUSE_MARGIN 4
#define PAUSE_LONGEST_TEXT_LABEL 9
#define PAUSE_TOTAL_W (PAUSE_MARGIN*5+NS_sys_tilesize*3+PAUSE_LONGEST_TEXT_LABEL*8)
#define PAUSE_TOTAL_H  (PAUSE_MARGIN*4+NS_sys_tilesize*3)
#define PAUSE_TOTAL_X ((FBW>>1)-(PAUSE_TOTAL_W>>1))
#define PAUSE_TOTAL_Y ((FBH>>1)-(PAUSE_TOTAL_H>>1))

#define PAUSE_LABEL_LIMIT 13 /* 9 items + resume + quit + new game + tattle */

struct modal_pause {
  struct modal hdr;
  int phase;
  double phaseclock;
  int pocketx,pockety; // where we live when not displayed
  struct label {
    int x,y,w,h;
    uint8_t tileid,xform;
    const char *text;
    int textc;
    int qty; // <0 to suppress
    int selectable;
  } labelv[PAUSE_LABEL_LIMIT];
  int labelc;
  int labelp;
  double animclock;
  int animframe;
};

#define MODAL ((struct modal_pause*)modal)

/* Cleanup.
 */
 
static void _pause_del(struct modal *modal) {
}

/* Replace text of the item name tattle.
 */
 
static void pause_update_item_label(struct modal *modal) {
  if (MODAL->labelc<13) return;
  struct label *label=MODAL->labelv+12;
  if (label->tileid||label->selectable) return; // oops we got the wrong one somehow
  int itemid=MODAL->labelp; // First 9 labels are the items, in canonical order.
  if ((itemid<0)||(itemid>=9)||!store_get(NS_fld_got_broom+itemid)) {
    label->textc=0;
  } else {
    label->textc=strings_get(&label->text,1,8+itemid);
  }
}

/* Digested input events.
 */
 
static void pause_move(struct modal *modal,int dx,int dy) {
  if (MODAL->labelc<9) return; // There's supposed to be a label for each item, whether possessed or not. We assume so.
  
  egg_play_sound(RID_sound_uimotion);
  
  // If there was no initial selection, pick one of the edge middles, opposite the pressed key.
  if ((MODAL->labelp<0)||(MODAL->labelp>=MODAL->labelc)) {
    if (dx<0) MODAL->labelp=5;
    else if (dx>0) MODAL->labelp=3;
    else if (dy<0) MODAL->labelp=7;
    else MODAL->labelp=1;
    pause_update_item_label(modal);
    return;
  }
  
  /* With one previously selected, move generically.
   * They're laid out in a grid so it's fair to assume that some aligned neighbor will exist.
   */
  struct label *prev=MODAL->labelv+MODAL->labelp;
  struct label *next=0;
  int nextd=9999;
  int i=MODAL->labelc;
  while (i-->0) {
    struct label *q=MODAL->labelv+i;
    if (q==prev) continue;
    if (!q->selectable) continue;
    int d;
    if (dx) {
      if (q->y+q->h<=prev->y) continue;
      if (q->y>=prev->y+prev->h) continue;
      if (dx<0) d=prev->x-q->x;
      else d=q->x-prev->x;
      if (d<0) d+=FBW;
    } else {
      if (q->x+q->w<=prev->x) continue;
      if (q->x>=prev->x+prev->w) continue;
      if (dy<0) d=prev->y-q->y;
      else d=q->y-prev->y;
      if (d<0) d+=FBH;
    }
    if (!next||(d<nextd)||((d==nextd)&&((q->x<next->x)||(q->y<next->y)))) {
      next=q;
      nextd=d;
    }
  }
  if (!next) return;
  MODAL->labelp=next-MODAL->labelv;
  
  pause_update_item_label(modal);
}

static void pause_dismiss(struct modal *modal) {
  if (MODAL->phase==PAUSE_PHASE_END) return; // alright already, i'm working on it!
  egg_play_sound(RID_sound_dismiss);
  if (MODAL->phase==PAUSE_PHASE_BEGIN) {
    // Set clock to the appropriate moment of the animation.
    double t=MODAL->phaseclock/PAUSE_BEGIN_TIME;
    t=1.0-t;
    if (t<0.0) t=0.0; else if (t>1.0) t=1.0;
    MODAL->phaseclock=t*PAUSE_END_TIME;
    MODAL->phase=PAUSE_PHASE_END;
  } else {
    MODAL->phase=PAUSE_PHASE_END;
    MODAL->phaseclock=0.0;
  }
}

static void pause_pick_item(struct modal *modal,int itemid) {
  store_set(NS_fld_equipped,itemid);
  pause_dismiss(modal);
}

static void pause_main_menu(struct modal *modal) {
  modal->defunct=1;
  modal_new_hello();
}

static void pause_new_game(struct modal *modal) {
  modal->defunct=1;
  if (session_reset()<0) return;
  int i=g.modalc; while (i-->0) {
    if (g.modalv[i]->ctor==modal_new_play) g.modalv[i]->defunct=1;
  }
  modal_new_play();
}

static void pause_activate(struct modal *modal) {
  switch (MODAL->labelp) {
    case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: pause_pick_item(modal,NS_fld_got_broom+MODAL->labelp); break;
    case 9: modal->defunct=1; break;
    case 10: pause_main_menu(modal); break;
    case 11: pause_new_game(modal); break;
  }
}

/* Input state change.
 */
 
static void _pause_input(struct modal *modal) {
  if (MODAL->phase==PAUSE_PHASE_RUN) {
    if ((g.input&EGG_BTN_LEFT)&&!(g.pvinput&EGG_BTN_LEFT)) pause_move(modal,-1,0);
    if ((g.input&EGG_BTN_RIGHT)&&!(g.pvinput&EGG_BTN_RIGHT)) pause_move(modal,1,0);
    if ((g.input&EGG_BTN_UP)&&!(g.pvinput&EGG_BTN_UP)) pause_move(modal,0,-1);
    if ((g.input&EGG_BTN_DOWN)&&!(g.pvinput&EGG_BTN_DOWN)) pause_move(modal,0,1);
    if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
      g.input_blackout|=EGG_BTN_SOUTH;
      pause_activate(modal);
    }
  }
  if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) {
    g.input_blackout|=EGG_BTN_WEST;
    if ((MODAL->labelp>=0)&&(MODAL->labelp<9)) { // WEST with an item focussed, accept it.
      pause_activate(modal);
    } else { // WEST anywhere else is a straight "cancel"
      pause_dismiss(modal);
    }
  }
}

/* Update.
 */
 
static void _pause_update(struct modal *modal,double elapsed) {
  if ((MODAL->animclock-=elapsed)<0.0) {
    MODAL->animclock+=0.150;
    if (++(MODAL->animframe)>=6) MODAL->animframe=0;
  }
  switch (MODAL->phase) {
    
    case PAUSE_PHASE_BEGIN: {
        if ((MODAL->phaseclock+=elapsed)>=PAUSE_BEGIN_TIME) {
          MODAL->phase=PAUSE_PHASE_RUN;
          MODAL->phaseclock=0.0;
        }
      } break;
      
    case PAUSE_PHASE_END: {
        if ((MODAL->phaseclock+=elapsed)>=PAUSE_END_TIME) {
          modal->defunct=1;
        }
      } break;
      
    case PAUSE_PHASE_RUN: {
      } break;
  }
}

/* Render.
 */
 
static void _pause_render(struct modal *modal) {
  switch (MODAL->phase) {
  
    case PAUSE_PHASE_BEGIN: {
        double t=MODAL->phaseclock/PAUSE_BEGIN_TIME;
        if (t<0.0) t=0.0; else if (t>1.0) t=1.0;
        double r=1.0-t;
        int x=(int)(MODAL->pocketx*r+PAUSE_TOTAL_X*t);
        int y=(int)(MODAL->pockety*r+PAUSE_TOTAL_Y*t);
        int w=(int)(PAUSE_TOTAL_W*t);
        int h=(int)(PAUSE_TOTAL_H*t);
        graf_draw_rect(&g.graf,x,y,w,h,nes_colors[0]);
      } break;
      
    case PAUSE_PHASE_END: {
        double t=MODAL->phaseclock/PAUSE_END_TIME;
        if (t<0.0) t=0.0; else if (t>1.0) t=1.0;
        double r=1.0-t;
        int x=(int)(MODAL->pocketx*t+PAUSE_TOTAL_X*r);
        int y=(int)(MODAL->pockety*t+PAUSE_TOTAL_Y*r);
        int w=(int)(PAUSE_TOTAL_W*r);
        int h=(int)(PAUSE_TOTAL_H*r);
        graf_draw_rect(&g.graf,x,y,w,h,nes_colors[0]);
      } break;
      
    case PAUSE_PHASE_RUN: {
        graf_draw_rect(&g.graf,PAUSE_TOTAL_X,PAUSE_TOTAL_Y,PAUSE_TOTAL_W,PAUSE_TOTAL_H,nes_colors[0]);
        
        if ((MODAL->labelp>=0)&&(MODAL->labelp<MODAL->labelc)) { // cursor
          struct label *label=MODAL->labelv+MODAL->labelp;
          if (label->textc) {
            graf_draw_rect(&g.graf,label->x,label->y,label->w,label->h,nes_colors[7]);
          } else {
            graf_draw_tile(&g.graf,g.texid_sprites,label->x+(label->w>>1),label->y+(label->h>>1),0x39+MODAL->animframe,0);
          }
        }
        
        struct label *label=MODAL->labelv;
        int i=MODAL->labelc;
        for (;i-->0;label++) {
          if (label->tileid) {
            graf_draw_tile(&g.graf,g.texid_sprites,label->x+(label->w>>1),label->y+(label->h>>1),label->tileid,label->xform);
          } else if (label->textc) {
            dw_draw_string(label->x+4,label->y+(label->h>>1),label->text,label->textc,1);
          }
        }
        for (label=MODAL->labelv,i=MODAL->labelc;i-->0;label++) {
          if (label->qty<0) continue;
          graf_draw_tile(&g.graf,g.texid_sprites,label->x+label->w,label->y+label->h,0x90+(label->qty%10),0);
          if (label->qty>=10) {
            graf_draw_tile(&g.graf,g.texid_sprites,label->x+label->w-4,label->y+label->h,0x90+(label->qty/10),0);
            // Quantities >99 will never be permitted.
          }
        }
      } break;
  }
}

/* Add label.
 */
 
static struct label *pause_add_item_label(struct modal *modal,int col,int row,int kgot,int kqty) {
  int colw=PAUSE_MARGIN+NS_sys_tilesize;
  if (MODAL->labelc>=PAUSE_LABEL_LIMIT) return 0;
  struct label *label=MODAL->labelv+MODAL->labelc++;
  memset(label,0,sizeof(struct label));
  label->selectable=1;
  label->x=PAUSE_TOTAL_X+PAUSE_MARGIN+col*colw;
  label->y=PAUSE_TOTAL_Y+PAUSE_MARGIN+row*colw;
  label->w=NS_sys_tilesize;
  label->h=NS_sys_tilesize;
  label->qty=-1;
  if (store_get(kgot)) {
    label->tileid=0x30+kgot-NS_fld_got_broom;
    if (kqty) {
      label->qty=store_get(kqty);
    }
  }
  return label;
}

static struct label *pause_add_string_label(struct modal *modal,int row,int rid,int ix) {
  if (MODAL->labelc>=PAUSE_LABEL_LIMIT) return 0;
  struct label *label=MODAL->labelv+MODAL->labelc++;
  memset(label,0,sizeof(struct label));
  label->selectable=1;
  label->textc=strings_get(&label->text,rid,ix);
  label->x=PAUSE_TOTAL_X+PAUSE_MARGIN*4+NS_sys_tilesize*3;
  label->y=PAUSE_TOTAL_Y+PAUSE_MARGIN+row*8;
  label->w=label->textc*8+1;
  label->h=8;
  label->qty=-1;
  return label;
}

/* Init.
 */
 
static int _pause_init(struct modal *modal,int x,int y) {
  modal->name="pause";
  modal->ctor=modal_new_pause;
  modal->del=_pause_del;
  modal->input=_pause_input;
  modal->update=_pause_update;
  modal->render=_pause_render;
  modal->opaque=0;
  modal->passive=0;
  
  egg_play_sound(RID_sound_pause);
  MODAL->phase=PAUSE_PHASE_BEGIN;
  MODAL->pocketx=x;
  MODAL->pockety=y;
  MODAL->labelp=-1;
  
  pause_add_item_label(modal,0,0,NS_fld_got_broom,0);
  pause_add_item_label(modal,1,0,NS_fld_got_pepper,NS_fld_qty_pepper);
  pause_add_item_label(modal,2,0,NS_fld_got_compass,0);
  pause_add_item_label(modal,0,1,NS_fld_got_stopwatch,0);
  pause_add_item_label(modal,1,1,NS_fld_got_camera,0);
  pause_add_item_label(modal,2,1,NS_fld_got_snowglobe,0);
  pause_add_item_label(modal,0,2,NS_fld_got_wand,0);
  pause_add_item_label(modal,1,2,NS_fld_got_bomb,NS_fld_qty_bomb);
  pause_add_item_label(modal,2,2,NS_fld_got_candy,NS_fld_qty_candy);
  pause_add_string_label(modal,0,1,5); // Resume
  pause_add_string_label(modal,1,1,6); // Main Menu
  pause_add_string_label(modal,2,1,7); // New Game
  struct label *label=pause_add_string_label(modal,6,1,0); // Placeholder for item name.
  if (!label) return -1;
  label->selectable=0;
  
  int equipped=store_get(NS_fld_equipped);
  MODAL->labelp=equipped-NS_fld_got_broom;
  pause_update_item_label(modal);
  
  return 0;
}

/* New.
 */
 
struct modal *modal_new_pause(int x,int y) {
  struct modal *modal=modal_new(sizeof(struct modal_pause));
  if (!modal) return 0;
  if (_pause_init(modal,x,y)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
