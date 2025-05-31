#include "game.h"

struct g g={0};

void egg_client_quit(int status) {
}

int egg_client_init() {
  
  int fbw=0,fbh=0;
  egg_texture_get_status(&fbw,&fbh,1);
  if ((fbw!=FBW)||(fbh!=FBH)) return -1;
  
  if ((g.romc=egg_get_rom(0,0))<=0) return -1;
  if (!(g.rom=malloc(g.romc))) return -1;
  if (egg_get_rom(g.rom,g.romc)!=g.romc) return -1;
  strings_set_rom(g.rom,g.romc);
  
  if (!(g.font=font_new())) return -1;
  if (font_add_image_resource(g.font,0x0020,RID_image_font9_0020)<0) return -1;
  if (egg_texture_load_image(g.texid_tilefont=egg_texture_new(),RID_image_tilefont)<0) return -1;
  if (egg_texture_load_image(g.texid_tiles=egg_texture_new(),RID_image_tiles)<0) return -1;
  if (egg_texture_load_image(g.texid_sprites=egg_texture_new(),RID_image_sprites)<0) return -1;
  // Being a jam game, we'll try to keep all the map tiles in one image, and ditto all the sprite tiles.
  
  if (egg_texture_load_raw(g.transition_texid=egg_texture_new(),FBW,FBH,FBW<<2,0,0)<0) return -1;
  
  if (session_init()<0) return -1;
  
  srand_auto();
  
  if (!modal_new_hello()) return -1;
  
  return 0;
}

void egg_client_update(double elapsed) {

  int input=egg_input_get_one(0);
  if (g.input_blackout) {
    g.input_blackout&=input;
    input&=~g.input_blackout;
  }
  if ((input!=g.pvinput)||(input!=g.input)) {
    g.pvinput=g.input;
    g.input=input;
    if ((g.input&EGG_BTN_AUX3)&&!(g.pvinput&EGG_BTN_AUX3)) {
      egg_terminate(0);
      return;
    }
    modals_input();
    g.input&=~g.input_blackout;
  }
  
  modals_update(elapsed);
  if (!g.modalc) { // If the modal stack is depleted, launch Hello.
    if (!modal_new_hello()) egg_terminate(1);
  }
}

void egg_client_render() {
  graf_reset(&g.graf);
  modals_render();
  graf_flush(&g.graf);
}

// https://lospec.com/palette-list/nintendo-entertainment-system
const uint32_t nes_colors[55]={
  0x000000ff,0xfcfcfcff,0xf8f8f8ff,0xbcbcbcff,0x7c7c7cff,
  0xa4e4fcff,0x3cbcfcff,0x0078f8ff,0x0000fcff,0xb8b8f8ff,
  0x6888fcff,0x0058f8ff,0x0000bcff,0xd8b8f8ff,0x9878f8ff,
  0x6844fcff,0x4428bcff,0xf8b8f8ff,0xf878f8ff,0xd800ccff,
  0x940084ff,0xf8a4c0ff,0xf85898ff,0xe40058ff,0xa80020ff,
  0xf0d0b0ff,0xf87858ff,0xf83800ff,0xa81000ff,0xfce0a8ff,
  0xfca044ff,0xe45c10ff,0x881400ff,0xf8d878ff,0xf8b800ff,
  0xac7c00ff,0x503000ff,0xd8f878ff,0xb8f818ff,0x00b800ff,
  0x007800ff,0xb8f8b8ff,0x58d854ff,0x00a800ff,0x006800ff,
  0xb8f8d8ff,0x58f898ff,0x00a844ff,0x005800ff,0x00fcfcff,
  0x00e8d8ff,0x008888ff,0x004058ff,0xf8d8f8ff,0x787878ff,
};

void dw_draw_string(int x,int y,const char *src,int srcc,int colorp) {
  if (!src) return;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if ((colorp<0)||(colorp>=55)) colorp=1;
  graf_set_tint(&g.graf,nes_colors[colorp]);
  graf_draw_tile_buffer(&g.graf,g.texid_tilefont,x,y,(void*)src,srcc,1,srcc);
  graf_set_tint(&g.graf,0);
}
