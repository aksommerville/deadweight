/* modal_boss.c
 * Shows a cutscene exactly once per session, when you enter the dungeon.
 * We're triggered by sprite_boss and regulated by NS_fld_boss_cutscene.
 */

#include "game/game.h"

#define BOSS_SCENEY 32
#define BOSS_SCENEH 112
#define BOSS_SLIDEC 3
#define BOSS_IMAGE_ADVANCE_RATE 2.000
#define BOSS_IMAGE_RETREAT_RATE 4.000
#define BOSS_TYPEWRITER_TIME 0.080
#define BOSS_MSG_LIMIT 64

struct modal_boss {
  struct modal hdr;
  int texid; // Single-use graphics, all in one image.
  int slidep;
  
  /* Each slide consists of a left and right image (or none), and a string (38,39,40).
   * Images slide in and out horizontally, and the string pays out one character at a time.
   */
  double limaget,rimaget; // 0..1; Count up during advance and down during retreat.
  int lsrcx,lsrcy,rsrcx,rsrcy; // Top row is the background image, so (srcy!=0) is the indication that an image is in use.
  int lretreat,rretreat; // If nonzero, we are still showing the previous image, and it's sliding out.
  int lsrcxnext,lsrcynext,rsrcxnext,rsrcynext; // If retreating.
  struct egg_draw_tile msg[BOSS_MSG_LIMIT];
  int msgc,msgp;
  double msgclock;
};

#define MODAL ((struct modal_boss*)modal)

/* Cleanup.
 */
 
static void _boss_del(struct modal *modal) {
  egg_texture_del(MODAL->texid);
}

/* To end of slide.
 */
 
static void boss_cut_slide(struct modal *modal) {
  MODAL->limaget=1.0;
  MODAL->rimaget=1.0;
  MODAL->lretreat=0;
  MODAL->rretreat=0;
  if (MODAL->msgp<MODAL->msgc) {
    MODAL->msgp=MODAL->msgc;
    egg_play_sound(RID_sound_text_skip);
  }
}

/* Generate (msg,msgc) from the given text.
 */
 
static void boss_prepare_text(struct modal *modal,const char *src,int srcc) {
  MODAL->msgc=0;
  const int margin=10;
  const int wlimit=(FBW-margin*2)/8;
  const int dstx0=margin+4;
  int hlimit=BOSS_MSG_LIMIT/wlimit;
  int dsty=BOSS_SCENEY+BOSS_SCENEH+margin+4;
  int dstx=dstx0;
  int col=0,row=0,srcp=0;
  while (srcp<srcc) {
    if ((unsigned char)src[srcp]<=0x20) { // ok to skip leading and trailing space.
      srcp++;
      continue;
    }
    // Everything that isn't space is a word char and can't break. (unless a single word is longer than the output).
    const char *word=src+srcp;
    int wordc=0;
    while ((srcp+wordc<srcc)&&((unsigned char)src[srcp+wordc]>0x20)&&(wordc<wlimit)) wordc++;
    if (col) { col++; dstx+=8; } // If we're not at the line start, add a space (no tile)
    if (col+wordc>wlimit) { // Too long, and we're not at the row start. Skip to the next line.
      col=0;
      row++;
      if (row>=hlimit) break;
      dsty+=8;
      dstx=dstx0;
    }
    // Now it does fit at the insertion point. Make it so.
    srcp+=wordc;
    int i=wordc;
    for (;i-->0;word++,col++,dstx+=8) {
      struct egg_draw_tile *vtx=MODAL->msg+MODAL->msgc++;
      vtx->dstx=dstx;
      vtx->dsty=dsty;
      vtx->tileid=*word;
      vtx->xform=0;
    }
  }
}

/* Next slide or terminate modal.
 */
 
static void boss_next_slide(struct modal *modal) {
  MODAL->slidep++;
  MODAL->msgc=0;
  MODAL->msgp=0;
  MODAL->msgclock=0.0;
  if (MODAL->lsrcy&&(MODAL->limaget>0.0)) {
    MODAL->lretreat=1;
  } else {
    MODAL->lretreat=0;
  }
  if (MODAL->rsrcy&&(MODAL->rimaget>0.0)) {
    MODAL->rretreat=1;
  } else {
    MODAL->rretreat=0;
  }
  int strix=0;
  switch (MODAL->slidep) {
  
    case 1: {
        MODAL->lsrcynext=0;
        MODAL->rsrcxnext=FBW>>1;
        MODAL->rsrcynext=BOSS_SCENEH;
        strix=38;
      } break;
      
    case 2: {
        MODAL->lsrcxnext=0;
        MODAL->lsrcynext=BOSS_SCENEH;
        MODAL->rsrcxnext=FBW>>1;
        MODAL->rsrcynext=BOSS_SCENEH;
        strix=39;
      } break;
      
    case 3: {
        // limage: Dot turns from "I'll save you!" to "Oh no a monster!". Don't pan her out in between; lie about the previous image.
        MODAL->lsrcx=MODAL->lsrcxnext=0;
        MODAL->lsrcy=MODAL->lsrcynext=BOSS_SCENEH*2;
        MODAL->rsrcxnext=FBW>>1;
        MODAL->rsrcynext=BOSS_SCENEH*2;
        strix=40;
      } break;
      
    default: {
        modal->defunct=1;
      }
  }
  if ((MODAL->lsrcx==MODAL->lsrcxnext)&&(MODAL->lsrcy==MODAL->lsrcynext)) {
    MODAL->lretreat=0;
  } else if (!MODAL->lretreat) {
    MODAL->lsrcx=MODAL->lsrcxnext;
    MODAL->lsrcy=MODAL->lsrcynext;
    MODAL->limaget=0.0;
  }
  if ((MODAL->rsrcx==MODAL->rsrcxnext)&&(MODAL->rsrcy==MODAL->rsrcynext)) {
    MODAL->rretreat=0;
  } else if (!MODAL->rretreat) {
    MODAL->rsrcx=MODAL->rsrcxnext;
    MODAL->rsrcy=MODAL->rsrcynext;
    MODAL->rimaget=0.0;
  }
  if (strix) {
    const char *src;
    int srcc=strings_get(&src,1,strix);
    if (srcc>0) boss_prepare_text(modal,src,srcc);
  }
}

/* Input.
 */
 
static void _boss_input(struct modal *modal) {

  /* When the button goes down, jump to the end of the current slide and hold there.
   */
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    boss_cut_slide(modal);
    
  /* When the button goes up, start the next slide.
   */
  } else if (!(g.input&EGG_BTN_SOUTH)&&(g.pvinput&EGG_BTN_SOUTH)) {
    boss_next_slide(modal);
  }
}

/* Update.
 */
 
static void _boss_update(struct modal *modal,double elapsed) {

  if (MODAL->lretreat) {
    if ((MODAL->limaget-=elapsed*BOSS_IMAGE_RETREAT_RATE)<=0.0) {
      MODAL->limaget=0.0;
      MODAL->lretreat=0;
      MODAL->lsrcx=MODAL->lsrcxnext;
      MODAL->lsrcy=MODAL->lsrcynext;
    }
  } else if (MODAL->limaget<1.0) {
    if ((MODAL->limaget+=elapsed*BOSS_IMAGE_ADVANCE_RATE)>=1.0) {
      MODAL->limaget=1.0;
    }
  }
  
  if (MODAL->rretreat) {
    if ((MODAL->rimaget-=elapsed*BOSS_IMAGE_RETREAT_RATE)<=0.0) {
      MODAL->rimaget=0.0;
      MODAL->rretreat=0;
      MODAL->rsrcx=MODAL->rsrcxnext;
      MODAL->rsrcy=MODAL->rsrcynext;
    }
  } else if (MODAL->rimaget<1.0) {
    if ((MODAL->rimaget+=elapsed*BOSS_IMAGE_ADVANCE_RATE)>=1.0) {
      MODAL->rimaget=1.0;
    }
  }
  
  if (MODAL->msgp<MODAL->msgc) {
    MODAL->msgclock+=elapsed;
    int beep=0;
    while ((MODAL->msgp<MODAL->msgc)&&(MODAL->msgclock>=BOSS_TYPEWRITER_TIME)) {
      MODAL->msgclock-=BOSS_TYPEWRITER_TIME;
      char ch=MODAL->msg[MODAL->msgp++].tileid;
      if (ch>0x20) beep=1;
    }
    if (beep) egg_play_sound(RID_sound_text);
  }
}

/* Render.
 */
 
static void _boss_render(struct modal *modal) {
  int dstx=0;
  int dsty=BOSS_SCENEY;
  graf_draw_rect(&g.graf,0,0,FBW,FBH,nes_colors[0]);
  graf_draw_decal(&g.graf,MODAL->texid,dstx,dsty,0,0,FBW,BOSS_SCENEH,0);
  
  if (MODAL->lsrcy&&(MODAL->limaget>0.0)) {
    int x=(int)(MODAL->limaget*(FBW>>1))-(FBW>>1);
    graf_draw_decal(&g.graf,MODAL->texid,x,dsty,MODAL->lsrcx,MODAL->lsrcy,FBW>>1,BOSS_SCENEH,0);
  }
  if (MODAL->rsrcy&&(MODAL->rimaget>0.0)) {
    int x=FBW-(int)(MODAL->rimaget*(FBW>>1));
    graf_draw_decal(&g.graf,MODAL->texid,x,dsty,MODAL->rsrcx,MODAL->rsrcy,FBW>>1,BOSS_SCENEH,0);
  }
  
  if (MODAL->msgp) {
    graf_flush(&g.graf);
    egg_draw_tile(1,g.texid_tilefont,MODAL->msg,MODAL->msgp);
  }
}

/* Init.
 */
 
static int _boss_init(struct modal *modal) {
  modal->name="boss";
  modal->ctor=modal_new_boss;
  modal->del=_boss_del;
  modal->input=_boss_input;
  modal->update=_boss_update;
  modal->render=_boss_render;
  modal->opaque=1;
  modal->passive=0;
  
  egg_texture_load_image(MODAL->texid=egg_texture_new(),RID_image_boss);
  
  boss_next_slide(modal);
  
  return 0;
}

/* New.
 */
 
struct modal *modal_new_boss() {
  struct modal *modal=modal_new(sizeof(struct modal_boss));
  if (!modal) return 0;
  if (_boss_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
