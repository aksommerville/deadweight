#include "game/game.h"

struct modal_hello {
  struct modal hdr;
  int title_texid,title_w,title_h;
  const char *msg_copyright,*msg_press_start;
  int msg_copyrightc,msg_press_startc;
  double blinkclock;
};

#define MODAL ((struct modal_hello*)modal)

/* Cleanup.
 */
 
static void _hello_del(struct modal *modal) {
  egg_texture_del(MODAL->title_texid);
}

/* Digested input.
 */
 
static void _hello_move(struct modal *modal,int dx,int dy) {
}

static void _hello_activate(struct modal *modal) {
  if (session_reset()<0) return;
  modal->defunct=1;
  modal_new_play();
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
  graf_draw_rect(&g.graf,0,0,FBW,FBH,nes_colors[48]);
  graf_draw_decal(&g.graf,MODAL->title_texid,(FBW>>1)-(MODAL->title_w>>1),20,0,0,MODAL->title_w,MODAL->title_h,0);
  dw_draw_string((FBW>>1)-(4*MODAL->msg_copyrightc),184,MODAL->msg_copyright,MODAL->msg_copyrightc,3);
  if (MODAL->blinkclock<0.750) {
    dw_draw_string((FBW>>1)-(4*MODAL->msg_press_startc),145,MODAL->msg_press_start,MODAL->msg_press_startc,1);
  }
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
  
  egg_texture_load_image(MODAL->title_texid=egg_texture_new(),RID_image_title);
  egg_texture_get_status(&MODAL->title_w,&MODAL->title_h,MODAL->title_texid);
  MODAL->msg_copyrightc=strings_get(&MODAL->msg_copyright,1,3);
  MODAL->msg_press_startc=strings_get(&MODAL->msg_press_start,1,4);
  
  egg_play_song(RID_song_dead_weight,0,1);
  
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
