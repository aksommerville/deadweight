#include "game/game.h"

#define PAUSE_PHASE_BEGIN 1
#define PAUSE_PHASE_RUN 2
#define PAUSE_PHASE_END 3

#define PAUSE_BEGIN_TIME 0.400
#define PAUSE_END_TIME   0.200

#define PAUSE_TOTAL_W 100
#define PAUSE_TOTAL_H  60
#define PAUSE_TOTAL_X ((FBW>>1)-(PAUSE_TOTAL_W>>1))
#define PAUSE_TOTAL_Y ((FBH>>1)-(PAUSE_TOTAL_H>>1))

struct modal_pause {
  struct modal hdr;
  int phase;
  double phaseclock;
  int pocketx,pockety; // where we live when not displayed
  int pvinput;
  int input_blackout;
};

#define MODAL ((struct modal_pause*)modal)

/* Cleanup.
 */
 
static void _pause_del(struct modal *modal) {
}

/* Digested input events.
 */
 
static void pause_move(struct modal *modal,int dx,int dy) {
  fprintf(stderr,"%s %+d,%+d\n",__func__,dx,dy);
  egg_play_sound(RID_sound_uimotion);
}

static void pause_activate(struct modal *modal) {
  fprintf(stderr,"%s\n",__func__);
}

static void pause_cancel(struct modal *modal) {
  egg_play_sound(RID_sound_dismiss);
  MODAL->phase=PAUSE_PHASE_END;
  MODAL->phaseclock=0.0;
}

/* Update.
 */
 
static void _pause_update(struct modal *modal,double elapsed) {
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
        int input=egg_input_get_one(0);
        if (MODAL->input_blackout) {
          if (input&(EGG_BTN_LEFT|EGG_BTN_RIGHT|EGG_BTN_UP|EGG_BTN_DOWN|EGG_BTN_SOUTH|EGG_BTN_WEST)) {
            input=0;
          } else {
            MODAL->input_blackout=0;
          }
        }
        if (input!=MODAL->pvinput) {
          if ((input&EGG_BTN_LEFT)&&!(MODAL->pvinput&EGG_BTN_LEFT)) pause_move(modal,-1,0);
          if ((input&EGG_BTN_RIGHT)&&!(MODAL->pvinput&EGG_BTN_RIGHT)) pause_move(modal,1,0);
          if ((input&EGG_BTN_UP)&&!(MODAL->pvinput&EGG_BTN_UP)) pause_move(modal,0,-1);
          if ((input&EGG_BTN_DOWN)&&!(MODAL->pvinput&EGG_BTN_DOWN)) pause_move(modal,0,1);
          if ((input&EGG_BTN_SOUTH)&&!(MODAL->pvinput&EGG_BTN_SOUTH)) pause_activate(modal);
          if ((input&EGG_BTN_WEST)&&!(MODAL->pvinput&EGG_BTN_WEST)) pause_cancel(modal);
          MODAL->pvinput=input;
        }
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
      } break;
  }
}

/* Init.
 */
 
static int _pause_init(struct modal *modal,int x,int y) {
  modal->name="pause";
  modal->del=_pause_del;
  modal->update=_pause_update;
  modal->render=_pause_render;
  modal->opaque=0;
  modal->passive=0;
  
  egg_play_sound(RID_sound_pause);
  MODAL->phase=PAUSE_PHASE_BEGIN;
  MODAL->pocketx=x;
  MODAL->pockety=y;
  MODAL->input_blackout=1;
  
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
