/* modal_camera.c
 * Appears when you actuate the camera, prompting to either set the warp point or warp to it.
 */
 
#include "game/game.h"

#define CAMERA_EDGE_MARGIN 4
#define CAMERA_CURSOR_MARGIN 4

struct modal_camera {
  struct modal hdr;
  const char *msg_snap,*msg_warp;
  int msg_snapc,msg_warpc;
  int x,y,w,h; // Full bounds. (constant)
  int cursorp; // 0,1
};

#define MODAL ((struct modal_camera*)modal)

static void _camera_del(struct modal *modal) {
}

// Drop any existing selfie sprite, and create the new one.
static void camera_update_sprites() {
  int i=g.spritec;
  while (i-->0) {
    struct sprite *sprite=g.spritev[i];
    if (sprite->type==&sprite_type_selfie) sprite->defunct=1;
  }
  if (g.map&&(g.camera_mapid==g.map->rid)) {
    sprite_spawn(g.camerax+0.5,g.cameray+0.5,0,&sprite_type_selfie,0);
  }
}

static void _camera_input(struct modal *modal) {
  if ((g.input&EGG_BTN_UP)&&!(g.pvinput&EGG_BTN_UP)) MODAL->cursorp^=1;
  if ((g.input&EGG_BTN_DOWN)&&!(g.pvinput&EGG_BTN_DOWN)) MODAL->cursorp^=1;
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    switch (MODAL->cursorp) {
      case 0: { // Take picture.
          if (!g.map||!g.hero) return;
          g.camera_mapid=g.map->rid;
          g.camerax=(int)g.hero->x;
          g.cameray=(int)g.hero->y;
          egg_play_sound(RID_sound_camera);
          modal->defunct=1;
          camera_update_sprites();
        } break;
      case 1: { // Warp to previous picture.
          if (!g.camera_mapid||!g.hero||store_get(NS_fld_win)) {
            egg_play_sound(RID_sound_reject);
            return;
          }
          egg_play_sound(RID_sound_warp);
          g.hero->x=g.camerax+0.5;
          g.hero->y=g.cameray+0.5;
          enter_map(g.camera_mapid,TRANSITION_TELEPORT);
          modal->defunct=1;
        } break;
    }
    g.input_blackout|=EGG_BTN_SOUTH;
    return;
  }
  if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) {
    egg_play_sound(RID_sound_dismiss);
    modal->defunct=1;
    g.input_blackout|=EGG_BTN_WEST;
    return;
  }
}

static void _camera_render(struct modal *modal) {
  graf_draw_rect(&g.graf,MODAL->x,MODAL->y,MODAL->w,MODAL->h,nes_colors[0]);
  dw_draw_string(
    MODAL->x+CAMERA_EDGE_MARGIN+NS_sys_tilesize+CAMERA_CURSOR_MARGIN+4,
    MODAL->y+CAMERA_EDGE_MARGIN+4,
    MODAL->msg_snap,MODAL->msg_snapc,1
  );
  dw_draw_string(
    MODAL->x+CAMERA_EDGE_MARGIN+NS_sys_tilesize+CAMERA_CURSOR_MARGIN+4,
    MODAL->y+CAMERA_EDGE_MARGIN+12,
    MODAL->msg_warp,MODAL->msg_warpc,g.camera_mapid?1:4
  );
  graf_draw_tile(&g.graf,g.texid_sprites,
    MODAL->x+CAMERA_EDGE_MARGIN+(NS_sys_tilesize>>1),
    MODAL->y+CAMERA_EDGE_MARGIN+4+MODAL->cursorp*8,
    0x34,0
  );
}

struct modal *modal_new_camera() {
  struct modal *modal=modal_new(sizeof(struct modal_camera));
  if (!modal) return 0;
  modal->name="camera";
  modal->ctor=modal_new_camera;
  modal->del=_camera_del;
  modal->input=_camera_input;
  modal->render=_camera_render;
  
  MODAL->msg_snapc=strings_get(&MODAL->msg_snap,1,17);
  MODAL->msg_warpc=strings_get(&MODAL->msg_warp,1,18);
  
  int msgc=(MODAL->msg_snapc>MODAL->msg_warpc)?MODAL->msg_snapc:MODAL->msg_warpc;
  MODAL->w=CAMERA_EDGE_MARGIN+NS_sys_tilesize+CAMERA_CURSOR_MARGIN+msgc*8+CAMERA_EDGE_MARGIN;
  MODAL->h=CAMERA_EDGE_MARGIN+8*2+CAMERA_EDGE_MARGIN;
  MODAL->x=(FBW>>1)-(MODAL->w>>1);
  MODAL->y=(FBH>>1)-(MODAL->h>>1);
  
  return modal;
}
