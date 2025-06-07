/* shared_symbols.h
 * Consumed by both the game and the tools.
 */

#ifndef SHARED_SYMBOLS_H
#define SHARED_SYMBOLS_H

#define NS_sys_tilesize 16
#define NS_sys_mapw 16
#define NS_sys_maph 14

#define CMD_map_image      0x20 /* u16:imageid */
#define CMD_map_location   0x21 /* u8:longitude u8:latitude */
#define CMD_map_field      0x40 /* u16:k u16:v ; Set a store field at load. */
#define CMD_map_treadle    0x41 /* u8:x u8:y u16:fld */
#define CMD_map_stompbox   0x42 /* u8:x u8:y u16:fld */
#define CMD_map_switchable 0x43 /* u8:x u8:y u16:fld */
#define CMD_map_bombable   0x44 /* u8:x u8:y u16:fld */
#define CMD_map_sprite     0x61 /* u16:pos u16:spriteid u32:reserved */
#define CMD_map_door       0x62 /* u16:pos u16:mapid u16:dstpos u16:reserved */

#define CMD_sprite_solid    0x01
#define CMD_sprite_airborne 0x02
#define CMD_sprite_monster  0x03
#define CMD_sprite_image    0x20 /* u16:imageid */
#define CMD_sprite_tile     0x21 /* u8:tileid u8:xform */
#define CMD_sprite_sprtype  0x22 /* u16:sprtype */
#define CMD_sprite_layer    0x23 /* u8:layer u8:reserved */

#define NS_tilesheet_physics     1
#define NS_tilesheet_neighbors   0
#define NS_tilesheet_family      0
#define NS_tilesheet_weight      0

#define NS_physics_vacant    0
#define NS_physics_solid     1
#define NS_physics_hole      2

#define NS_sprtype_dummy 0
#define NS_sprtype_hero 1
#define NS_sprtype_princess 2
#define NS_sprtype_flamethrower 3
#define NS_sprtype_trap 4
#define NS_sprtype_bubblesaur 5
#define NS_sprtype_bubble 6
#define NS_sprtype_ssflame 7
#define NS_sprtype_selfie 8
#define NS_sprtype_bomb 9
#define NS_sprtype_candy 10
#define NS_sprtype_treasure 11
#define NS_sprtype_flagand 12
#define NS_sprtype_splash 13
#define NS_sprtype_soulballs 14
#define NS_sprtype_explode 15
#define NS_sprtype_smoke 16
#define NS_sprtype_knife 17
#define NS_sprtype_bug 18
#define NS_sprtype_prize 19
#define NS_sprtype_caterpillar 20
#define NS_sprtype_tank 21
#define NS_sprtype_missile 22
#define NS_sprtype_king 23
#define NS_sprtype_endprincess 24
#define NS_sprtype_boss 25
#define NS_sprtype_bossfire 26
#define NS_sprtype_pepperfire 27
#define SPRTYPE_FOR_EACH \
  _(dummy) \
  _(hero) \
  _(princess) \
  _(flamethrower) \
  _(trap) \
  _(bubblesaur) \
  _(bubble) \
  _(ssflame) \
  _(selfie) \
  _(bomb) \
  _(candy) \
  _(treasure) \
  _(flagand) \
  _(splash) \
  _(soulballs) \
  _(explode) \
  _(smoke) \
  _(knife) \
  _(bug) \
  _(prize) \
  _(caterpillar) \
  _(tank) \
  _(missile) \
  _(king) \
  _(endprincess) \
  _(boss) \
  _(bossfire) \
  _(pepperfire)

#define NS_fld_zero              0 /* immutable */
#define NS_fld_one               1 /* immutable */
#define NS_fld_got_broom         2 /* have item... */
#define NS_fld_got_pepper        3
#define NS_fld_got_compass       4
#define NS_fld_got_stopwatch     5
#define NS_fld_got_camera        6
#define NS_fld_got_snowglobe     7
#define NS_fld_got_wand          8
#define NS_fld_got_bomb          9
#define NS_fld_got_candy        10
#define NS_fld_qty_pepper       11 /* quantity of item... */
#define NS_fld_qty_bomb         12
#define NS_fld_qty_candy        13
#define NS_fld_equipped         14 /* Which item equipped (0,NS_fld_got_*) */
#define NS_fld_escort           15 /* Princess is saved and following me. Clears if she dies. */
#define NS_fld_boss_visited     16
#define NS_fld_boss_dead        17
#define NS_fld_twin1_a          18
#define NS_fld_twin1_b          19
#define NS_fld_twin1_c          20
#define NS_fld_twin2_a          21
#define NS_fld_twin2_b          22
#define NS_fld_twin2_c          23
#define NS_fld_oneway1          24
#define NS_fld_oneway2          25
#define NS_fld_bombe            26
#define NS_fld_bombctr          27
#define NS_fld_flamethrow1      28
#define NS_fld_bug              29
#define NS_fld_shortcut1        30
#define NS_fld_death_count      31
#define NS_fld_knivesoff        32
#define FLD_COUNT               33

#endif
