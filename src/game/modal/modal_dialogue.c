#include "game/game.h"

// Dialogue uses 8x8 tiles, not the map's 16x16.
#define DIALOGUE_COLC 30
#define DIALOGUE_ROWC 5
#define DIALOGUE_MARGIN 4

#define DIALOGUE_PHASE_BEGIN 1
#define DIALOGUE_PHASE_RUN 2
#define DIALOGUE_PHASE_END 3

#define DIALOGUE_BEGIN_TIME 0.400
#define DIALOGUE_END_TIME   0.200
#define DIALOGUE_VERTEX_TIME 0.080

struct modal_dialogue {
  struct modal hdr;
  int pocketx,pockety;
  int dstx,dsty,dstw,dsth; // final dimensions
  int phase;
  double phaseclock;
  struct egg_draw_tile vtxv[DIALOGUE_COLC*DIALOGUE_ROWC];
  int vtxp,vtxc; // (vtxp) is how many to render right now; (vtxc) how many there are total.
  double vtxclock;
};

#define MODAL ((struct modal_dialogue*)modal)

/* Cleanup.
 */
 
static void _dialogue_del(struct modal *modal) {
}

/* Cancel.
 */
 
static void dialogue_advance(struct modal *modal) {
  switch (MODAL->phase) {
    case DIALOGUE_PHASE_BEGIN: {
        MODAL->phase=DIALOGUE_PHASE_RUN;
        MODAL->phaseclock=0.0;
      } break;
    case DIALOGUE_PHASE_END: break;
    case DIALOGUE_PHASE_RUN: {
        if (MODAL->vtxp>=MODAL->vtxc) {
          egg_play_sound(RID_sound_dismiss);
          MODAL->phase=DIALOGUE_PHASE_END;
          MODAL->phaseclock=0.0;
        } else {
          egg_play_sound(RID_sound_text_skip);
          MODAL->vtxp=MODAL->vtxc;
        }
      } break;
  }
}

/* Input.
 */
 
static void _dialogue_input(struct modal *modal) {
  if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) {
    g.input_blackout|=EGG_BTN_WEST;
    dialogue_advance(modal);
  }
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    g.input_blackout|=EGG_BTN_SOUTH;
    dialogue_advance(modal);
  }
}

/* Update.
 */
 
static void _dialogue_update(struct modal *modal,double elapsed) {
  switch (MODAL->phase) {
    
    case DIALOGUE_PHASE_BEGIN: {
        if ((MODAL->phaseclock+=elapsed)>=DIALOGUE_BEGIN_TIME) {
          MODAL->phase=DIALOGUE_PHASE_RUN;
          MODAL->phaseclock=0.0;
        }
      } break;
      
    case DIALOGUE_PHASE_END: {
        if ((MODAL->phaseclock+=elapsed)>=DIALOGUE_END_TIME) {
          modal->defunct=1;
        }
      } break;
      
    case DIALOGUE_PHASE_RUN: {
        MODAL->vtxclock+=elapsed;
        int pip=0;
        while (MODAL->vtxclock>=DIALOGUE_VERTEX_TIME) {
          if (MODAL->vtxp>=MODAL->vtxc) break;
          MODAL->vtxclock-=DIALOGUE_VERTEX_TIME;
          MODAL->vtxp++;
          pip=1;
        }
        if (pip) egg_play_sound(RID_sound_text);
      } break;
  }
}

/* Render.
 */
 
static void _dialogue_render(struct modal *modal) {
  switch (MODAL->phase) {
  
    case DIALOGUE_PHASE_BEGIN: {
        double t=MODAL->phaseclock/DIALOGUE_BEGIN_TIME;
        if (t<0.0) t=0.0; else if (t>1.0) t=1.0;
        double r=1.0-t;
        int x=(int)(MODAL->pocketx*r+MODAL->dstx*t);
        int y=(int)(MODAL->pockety*r+MODAL->dsty*t);
        int w=(int)(MODAL->dstw*t);
        int h=(int)(MODAL->dsth*t);
        graf_draw_rect(&g.graf,x,y,w,h,nes_colors[0]);
      } break;
      
    case DIALOGUE_PHASE_END: {
        double t=MODAL->phaseclock/DIALOGUE_END_TIME;
        if (t<0.0) t=0.0; else if (t>1.0) t=1.0;
        double r=1.0-t;
        int x=(int)(MODAL->pocketx*t+MODAL->dstx*r);
        int y=(int)(MODAL->pockety*t+MODAL->dsty*r);
        int w=(int)(MODAL->dstw*r);
        int h=(int)(MODAL->dsth*r);
        graf_draw_rect(&g.graf,x,y,w,h,nes_colors[0]);
      } break;
      
    case DIALOGUE_PHASE_RUN: {
        graf_draw_rect(&g.graf,MODAL->dstx,MODAL->dsty,MODAL->dstw,MODAL->dsth,nes_colors[0]);
        graf_flush(&g.graf);
        egg_draw_tile(1,g.texid_tilefont,MODAL->vtxv,MODAL->vtxp);
      } break;
  }
}

/* Measure word.
 * Everything except LF and Space is a letter and must combine.
 */
 
static int dialogue_measure_word(const char *src,int srcc) {
  int srcp=0;
  while (srcp<srcc) {
    if (src[srcp]==0x20) break;
    if (src[srcp]==0x0a) break;
    srcp++;
  }
  return srcp;
}

/* Init.
 */
 
static int _dialogue_init(struct modal *modal,const char *msg,int msgc,int x,int y) {
  modal->name="dialogue";
  modal->ctor=modal_new_dialogue;
  modal->del=_dialogue_del;
  modal->input=_dialogue_input;
  modal->update=_dialogue_update;
  modal->render=_dialogue_render;
  modal->opaque=0;
  modal->passive=0;
  
  MODAL->pocketx=x;
  MODAL->pockety=y;
  MODAL->phase=DIALOGUE_PHASE_BEGIN;
  
  MODAL->dstw=DIALOGUE_COLC*8+(DIALOGUE_MARGIN<<1)+1; // Tiles are biased slightly right. 1 extra pixel balances the margin visually.
  MODAL->dsth=DIALOGUE_ROWC*8+(DIALOGUE_MARGIN<<1);
  MODAL->dstx=(FBW>>1)-(MODAL->dstw>>1);
  MODAL->dsty=FBH-MODAL->dsth-(NS_sys_tilesize>>1);
  
  int msgp=0,row=0,col=0;
  int dsty=MODAL->dsty+DIALOGUE_MARGIN+4;
  int dstx0=MODAL->dstx+DIALOGUE_MARGIN+4;
  int dstx=dstx0;
  while (msgp<msgc) {
    if (msg[msgp]==0x20) {
      msgp++;
      dstx+=8;
      col++;
      continue;
    }
    if (msg[msgp]==0x0a) {
      if (++row>=DIALOGUE_ROWC) break;
      msgp++;
      dsty+=8;
      dstx=dstx0;
      col=0;
      continue;
    }
    int wordlen=dialogue_measure_word(msg+msgp,msgc-msgp);
    if (wordlen<1) break;
    if (col&&(col>DIALOGUE_COLC-wordlen)) {
      if (++row>=DIALOGUE_ROWC) break;
      dsty+=8;
      dstx=dstx0;
      col=0;
    }
    if (col>DIALOGUE_COLC-wordlen) {
      wordlen=DIALOGUE_COLC-col;
    }
    int i=wordlen;
    for (;i-->0;msgp++,dstx+=8,col++) {
      struct egg_draw_tile *vtx=MODAL->vtxv+MODAL->vtxc++;
      vtx->dstx=dstx;
      vtx->dsty=dsty;
      vtx->tileid=msg[msgp];
      vtx->xform=0;
    }
  }
  
  egg_play_sound(RID_sound_dialogue);
  
  return 0;
}

/* New.
 */
 
struct modal *modal_new_dialogue(int x,int y,int rid,int ix,const struct strings_insertion *insv,int insc) {
  struct modal *modal=modal_new(sizeof(struct modal_dialogue));
  if (!modal) return 0;
  
  char msg[256];
  int msgc=strings_format(msg,sizeof(msg),rid,ix,insv,insc);
  if ((msgc<0)||(msgc>sizeof(msg))) msgc=0;
  
  if (_dialogue_init(modal,msg,msgc,x,y)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
