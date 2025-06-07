#include "game/game.h"

#define GAMEOVER_PEN_PERIOD 0.080
#define GAMEOVER_CONSOLE_SIZE (NS_sys_mapw*NS_sys_maph*4)

struct modal_gameover {
  struct modal hdr;
  
  /* The entire console content is generated at init.
   * Then we gradually advance (consolep) and render up to that.
   */
  uint8_t console[GAMEOVER_CONSOLE_SIZE];
  int consolep;
  double penclock;
};

#define MODAL ((struct modal_gameover*)modal)

/* Cleanup.
 */
 
static void _gameover_del(struct modal *modal) {
}

/* Input.
 */
 
static void _gameover_input(struct modal *modal) {
  if (
    ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH))||
    ((g.input&EGG_BTN_AUX1)&&!(g.pvinput&EGG_BTN_AUX1))
  ) {
    if (MODAL->consolep<GAMEOVER_CONSOLE_SIZE) {
      MODAL->consolep=GAMEOVER_CONSOLE_SIZE;
      egg_play_sound(RID_sound_text_skip);
    } else {
      modal->defunct=1;
      modal_new_hello();
    }
  }
}

/* Update.
 */
 
static void _gameover_update(struct modal *modal,double elapsed) {
  if (MODAL->consolep<GAMEOVER_CONSOLE_SIZE) {
    MODAL->penclock+=elapsed;
    int beep=0;
    while (MODAL->penclock>=GAMEOVER_PEN_PERIOD) {
      if (MODAL->consolep>=GAMEOVER_CONSOLE_SIZE) break;
      MODAL->penclock-=GAMEOVER_PEN_PERIOD;
      // Condense whitespace, consume one time unit for the lot, and do not beep.
      if (MODAL->console[MODAL->consolep]<=0x20) {
        MODAL->consolep++;
        while ((MODAL->consolep<GAMEOVER_CONSOLE_SIZE)&&(MODAL->console[MODAL->consolep]<=0x20)) MODAL->consolep++;
      } else {
        MODAL->consolep++;
        beep=1;
      }
    }
    if (beep) egg_play_sound(RID_sound_text);
  }
}

/* Render.
 */
 
static void _gameover_render(struct modal *modal) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,nes_colors[0]);
  const uint8_t *src=MODAL->console;
  int remaining=MODAL->consolep;
  int yi=NS_sys_maph*2,dsty=4;
  for (;yi-->0;dsty+=8) {
    int xi=NS_sys_mapw*2,dstx=4;
    for (;xi-->0;dstx+=8,src++) {
      if (--remaining<0) return;
      if (*src<=0x20) continue;
      graf_draw_tile(&g.graf,g.texid_tilefont,dstx,dsty,*src,0);
    }
  }
}

/* Add text to the console.
 */
 
static void gameover_text_general(struct modal *modal,int row,int align,int strix,const struct strings_insertion *insv,int insc) {
  if ((row<0)||(row>=NS_sys_maph*2)) return;
  const int dsta=NS_sys_mapw*2;
  char *dst=(char*)MODAL->console+row*NS_sys_mapw*2;
  int dstc=strings_format(dst,dsta,1,strix,insv,insc);
  if ((dstc<1)||(dstc>dsta)) return;
  int shift=0;
  if (align==0) shift=(dsta-dstc)>>1;
  else if (align>0) shift=dsta-dstc;
  if (shift>0) {
    memmove(dst+shift,dst,dstc);
    memset(dst,0,shift);
  }
}
 
static void gameover_text_(struct modal *modal,int row,int strix) {
  struct strings_insertion ins={0};
  gameover_text_general(modal,row,0,strix,&ins,1);
}

static void gameover_texti(struct modal *modal,int row,int strix,int v) {
  struct strings_insertion ins={'i',.i=v};
  gameover_text_general(modal,row,-1,strix,&ins,1);
}

static void gameover_textt(struct modal *modal,int row,int strix,double f) {
  int ms=(int)(f*1000.0);
  if (ms<0) ms=0;
  int s=ms/1000; ms%=1000;
  int m=s/60; s%=60;
  int h=m/60; m%=60;
  if (h>9) { h=9; m=s=99; ms=999; }
  char tmp[11]={
    '0'+h,
    ':',
    '0'+m/10,
    '0'+m%10,
    ':',
    '0'+s/10,
    '0'+s%10,
    ':',
    '0'+ms/100,
    '0'+(ms/10)%10,
    '0'+ms%10,
  };
  struct strings_insertion ins={'s',.s={.v=tmp,.c=sizeof(tmp)}};
  gameover_text_general(modal,row,-1,strix,&ins,1);
}

/* Init.
 */
 
static int _gameover_init(struct modal *modal) {
  modal->name="gameover";
  modal->ctor=modal_new_gameover;
  modal->del=_gameover_del;
  modal->input=_gameover_input;
  modal->update=_gameover_update;
  modal->render=_gameover_render;
  modal->opaque=1;
  modal->passive=0;
  
  int itemc=0,sidequestc=0;
  if (store_get(NS_fld_got_broom)) itemc++;
  if (store_get(NS_fld_got_compass)) itemc++;
  if (store_get(NS_fld_got_stopwatch)) itemc++;
  if (store_get(NS_fld_got_wand)) itemc++;
  if (store_get(NS_fld_got_snowglobe)) itemc++;
  if (store_get(NS_fld_got_camera)) itemc++;
  if (store_get(NS_fld_got_pepper)) itemc++;
  if (store_get(NS_fld_got_candy)) itemc++;
  if (store_get(NS_fld_got_bomb)) itemc++;
  if (store_get(NS_fld_sq1)) sidequestc++;
  if (store_get(NS_fld_sq2)) sidequestc++;
  if (store_get(NS_fld_sq3)) sidequestc++;
  
  gameover_text_(modal, 9,24);
  gameover_textt(modal,11,25,g.playtime);
  gameover_texti(modal,12,26,store_get(NS_fld_death_count));
  gameover_texti(modal,13,27,g.deadprincessc);
  gameover_texti(modal,14,28,g.deadmonsterc);
  gameover_texti(modal,15,29,itemc);
  gameover_texti(modal,16,30,sidequestc);
  gameover_text_(modal,18,31);
  
  return 0;
}

/* New.
 */
 
struct modal *modal_new_gameover() {
  struct modal *modal=modal_new(sizeof(struct modal_gameover));
  if (!modal) return 0;
  if (_gameover_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
