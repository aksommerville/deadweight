# Dead Weight

Requires [Egg](https://github.com/aksommerville/egg) to build.

2025-05-30 For NES Jam 2025, theme "SAVE THE PRINCESS".

Short Zelda-style adventure. Rescue the Princess, then escort her back to the castle.
The gimmick: Rescuing her is stupid easy due to your overpowered inventory.
Once she's in your care, items become more difficult to use. (emergently, eg your broom only seats one).
If you die, the screen resets, no problemo.
If she dies, she respawns back at the dungeon.

What happens to the Princess when the Hero dies?
I guess she resets to initial position like everything else.

Strict single-screen presentation. Most volatile state will reset with screen changes.
256x224 pixels fb, makes exactly 16x14 cells of 16x16 pixels.
Don't use doors or ladders: There's just a single plane to the world.

Actions like Full Moon: (A) to use, (B) to choose.
One thing equipped at a time.
Some items will have a count: Pepper, Bomb, Candy.

## TODO

- [x] Starter graphics.
- [x] Modals. Do a heavy intelligent modal stack, eg dialogue should overlay play and be its own modal.
- [x] Map loader with transitions.
- [x] State engine. Build in a distinction between persistent fields and screen-scoped ones.
- [x] Move input management to the global level. I'm just like two steps in and it's already a mess.
- [ ] Sprites.
- - [x] Hero.
- - [x] Princess.
- - [x] Switchable flamethrowers.
- - [ ] Pushable blocks.
- - [x] Switchable doors.
- - [ ] Oscillating traps.
- - [x] Responsive traps.
- - [ ] Roaming monsters. (multiple skins)
- - [x] Targetted fireballs. Target the Hero or the Princess? ...prefer targetting the princess
- - [ ] Straight-shooting monsters.
- - [ ] Target-shooting monsters.
- - [ ] Sentinel monsters. Stands in one place, attacks when you draw near.
- [ ] Hero and princess take damage.
- [ ] Boss fight, immediately before rescue, once per session.
- [x] Dialogue.
- [x] Items.
- - [x] Show Dot carrying whatever's equipped.
- - [x] Broom. Like Full Moon.
- - [x] Pepper. Burn everything within some radius. Light candles. Kill Princess.
- - [x] Compass. Points toward the Princess.
- - [x] Stopwatch. Like Too Heavy.
- - [x] Camera. Like Too Heavy. Maybe prompt when actuated "warp or replace?"
- - [x] Snowglobe. Like Full Moon.
- - [x] Wand. Summon moveable objects. (not for general spell casting, that won't be a thing).
- - [x] Bomb. Destroy things, esp Princesses.
- - [x] Candy. Distact monsters.
- [ ] Modals.
- - [x] Hello.
- - [ ] Game over. Success only. Game will not be loseable.
- - [x] Pause. Inventory and options, like Full Moon.
- [ ] Maps.
- - The initial path from Throne to Dungeon should be 11 screens exactly: The endpoints, and 9 challenge screens where you get a new item.
- - Boss is in the dungeon, same place you get the princess.
- - First challenge screen should give you the Compass. Don't let them make a wrong turn.
- - Should have plenty of other space, and there should be required side paths for the return trip.
- - 9 is kind of a lot. Should we make a few items optional?
- [x] Compass: Before princess sprite, identify the dungeon map and point to its center.
- [x] Egg editor: World map preview.
- [x] Should we start with something else, instead of Compass? Something non-passive, so the player gets to try out his A button.
- [ ] Bomb explosion.
- [ ] Candy attraction.
- [ ] Broom: prevent disengaging over a hole. Important!
- [ ] Broom is allowing me to sneak offscreen around walls. I thought physics was preventing that! It must.
- [x] The hello splash has got to go, alas. Needs to look silly and whimsical. More Kirby, less Ninja Gaiden.
- [x] Princess should trigger treadles.
- - Make a separate quantized-position-tracking service responsible for both Dot and Princess.
- - It's important that they consistently trade off holding a treadle, switching places and such.
- [ ] Proper graphics.
- [ ] Decorative snow during a snowglobe earthquake? Might be conspicuously nes-implausible.
- [ ] Sound effects.
- [ ] Music.
- [ ] Game over: Trigger when entering throne room with the princess.
- [ ] There's a bug somewhere in collision detection that lets you pass thru solid sprites sometimes when also colliding against the map. Ignoring it for now.
- [ ] Script to validate images: No more than 4 colors per tile, counting transparency, and all opaque colors must be in palette.
- [ ] Interrupted transitions snap complete before proceeding (eg enter a neighbor map and quickly change your mind). Is it feasible to start partway thru in those cases?
- [ ] Errata.
- - [ ] Itch page.

## Agenda

- [x] 2025-05-30T07:00 F Jam begins.
- [x] 2025-05-30 EOD   F Have global framework ready: Modals, map loader, state engine, sprite loader.
- [x] 2025-05-31 EOD   S Hero, Princess, transitions, dialogue.
- [x] 2025-06-01 EOD   U Sprites and items.
- [ ] 2025-06-06 EOD   F Rough world layout.
- [ ] 2025-06-07 EOD   S Sound effects.
- [ ] 2025-06-08 EOD   U Music, final graphics.
- [ ] 2025-06-13 EOD   F Polished and ready to go.
- [ ] 2025-06-14 EOD   S '' panic if not.
- [ ] 2025-06-15 EOD   U Have everything finished and submitted.
- [ ] 2025-06-16T07:00 M Jam ends.
