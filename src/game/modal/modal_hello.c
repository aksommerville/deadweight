#include "game/game.h"

struct modal_hello {
  struct modal hdr;
  int title_texid,title_w,title_h;
  const char *msg_copyright,*msg_press_start,*msg_nesjam,*msg_bits,*msg_play,*msg_quit;
  int msg_copyrightc,msg_press_startc,msg_nesjamc,msg_bitsc,msg_playc,msg_quitc;
  double blinkclock;
  int focusp; // 0,1,2 = bits,play,quit
  int bits0; // (g.bits) at startup. We'll reload the global graphics at dismiss, if it changed.
};

#define MODAL ((struct modal_hello*)modal)

/* Cleanup.
 */
 
static void _hello_del(struct modal *modal) {
  egg_texture_del(MODAL->title_texid);
}

/* Banner image.
 */
 
static void hello_load_banner(struct modal *modal) {
  if (!MODAL->title_texid) {
    MODAL->title_texid=egg_texture_new();
  }
  if (g.bits==8) {
    egg_texture_load_image(MODAL->title_texid,RID_image_title);
  } else {
    egg_texture_load_image(MODAL->title_texid,RID_image_title16);
  }
  egg_texture_get_status(&MODAL->title_w,&MODAL->title_h,MODAL->title_texid);
}

/* If bits changed, reload global images.
 * They were loaded at init, but user might have switched us to 8-bit mode.
 */
 
static void hello_commit_bits(struct modal *modal) {
  if (g.bits==MODAL->bits0) return;
  if (g.bits==8) {
    egg_texture_load_image(g.texid_tiles,RID_image_tiles);
    egg_texture_load_image(g.texid_sprites,RID_image_sprites);
  } else {
    egg_texture_load_image(g.texid_tiles,RID_image_tiles16);
    egg_texture_load_image(g.texid_sprites,RID_image_sprites16);
  }
}

/* Digested input.
 */
 
static void _hello_move(struct modal *modal,int dx,int dy) {
  egg_play_sound(RID_sound_uimotion);
  if (dy) {
    MODAL->focusp+=dy;
    if (MODAL->focusp<0) MODAL->focusp=2;
    else if (MODAL->focusp>=3) MODAL->focusp=0;
  } else if (dx&&(MODAL->focusp==0)) {
    g.bits=(g.bits==8)?16:8;
    hello_load_banner(modal);
  }
}

static void _hello_activate(struct modal *modal) {
  switch (MODAL->focusp) {
    case 0: _hello_move(modal,1,0); break;
    case 1: {
        hello_commit_bits(modal);
        if (session_reset()<0) return;
        modal->defunct=1;
        modal_new_play();
      } break;
    case 2: egg_terminate(0); break;
  }
}

static void _hello_cancel(struct modal *modal) {
}

/* Input state change.
 */
 
static void _hello_input(struct modal *modal) {
  if ((g.input&EGG_BTN_LEFT)&&!(g.pvinput&EGG_BTN_LEFT)) _hello_move(modal,-1,0);
  if ((g.input&EGG_BTN_RIGHT)&&!(g.pvinput&EGG_BTN_RIGHT)) _hello_move(modal,1,0);
  if ((g.input&EGG_BTN_UP)&&!(g.pvinput&EGG_BTN_UP)) _hello_move(modal,0,-1);
  if ((g.input&EGG_BTN_DOWN)&&!(g.pvinput&EGG_BTN_DOWN)) _hello_move(modal,0,1);
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    g.input_blackout|=EGG_BTN_SOUTH;
    _hello_activate(modal);
  }
  if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) _hello_cancel(modal);
  if ((g.input&EGG_BTN_AUX1)&&!(g.pvinput&EGG_BTN_AUX1)) {
    g.input_blackout|=EGG_BTN_AUX1;
    _hello_activate(modal);
  }
}

/* Update.
 */
 
static void _hello_update(struct modal *modal,double elapsed) {
  MODAL->blinkclock+=elapsed;
  if (MODAL->blinkclock>0.900) {
    MODAL->blinkclock-=0.900;
  }
}

/* Render.
 */
 
static void _hello_render(struct modal *modal) {

  // Flat background. Black in 16-bit mode and green in 8-bit mode (ugh why did i pick green...)
  if (g.bits==8) {
    graf_draw_rect(&g.graf,0,0,FBW,FBH,nes_colors[48]);
  } else {
    graf_draw_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  }
  // Static banner.
  graf_draw_decal(&g.graf,MODAL->title_texid,(FBW>>1)-(MODAL->title_w>>1),20,0,0,MODAL->title_w,MODAL->title_h,0);
  
  // Static text.
  dw_draw_string((FBW>>1)-(4*MODAL->msg_copyrightc),192,MODAL->msg_copyright,MODAL->msg_copyrightc,3);
  dw_draw_string((FBW>>1)-(4*MODAL->msg_nesjamc),200,MODAL->msg_nesjam,MODAL->msg_nesjamc,3);
  
  // Menu options.
  dw_draw_string(100,145,MODAL->msg_bits,MODAL->msg_bitsc,1);
  dw_draw_string(100,153,MODAL->msg_play,MODAL->msg_playc,1);
  dw_draw_string(100,161,MODAL->msg_quit,MODAL->msg_quitc,1);
  graf_set_tint(&g.graf,0xffff00ff);
  graf_draw_tile(&g.graf,g.texid_tilefont,88,145+MODAL->focusp*8,0x18,0);
  graf_set_tint(&g.graf,0);
  dw_draw_string(100+MODAL->msg_bitsc*8+8,145,"8",1,(g.bits==8)?49:4);
  dw_draw_string(100+MODAL->msg_bitsc*8+24,145,"16",2,(g.bits==16)?49:4);
}

/* Init.
 */
 
static int _hello_init(struct modal *modal) {
  modal->name="hello";
  modal->ctor=modal_new_hello;
  modal->del=_hello_del;
  modal->input=_hello_input;
  modal->update=_hello_update;
  modal->render=_hello_render;
  modal->opaque=1;
  modal->passive=0;
  MODAL->focusp=1; // "Play"
  MODAL->bits0=g.bits;
  
  hello_load_banner(modal);
  MODAL->msg_copyrightc=strings_get(&MODAL->msg_copyright,1,3);
  MODAL->msg_press_startc=strings_get(&MODAL->msg_press_start,1,4);
  MODAL->msg_nesjamc=strings_get(&MODAL->msg_nesjam,1,20);
  MODAL->msg_bitsc=strings_get(&MODAL->msg_bits,1,41);
  MODAL->msg_playc=strings_get(&MODAL->msg_play,1,42);
  MODAL->msg_quitc=strings_get(&MODAL->msg_quit,1,43);
  
  egg_play_song(RID_song_tickled_pink,0,1);
  
  return 0;
}

/* New.
 */
 
struct modal *modal_new_hello() {
  struct modal *modal=modal_new(sizeof(struct modal_hello));
  if (!modal) return 0;
  if (_hello_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
