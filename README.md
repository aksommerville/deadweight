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

- [x] Reliable segfault, just drop 8 bombs, segfaults before they explode.
- - Caused by walking (g.spritev) when it reallocates. Avoid iterating it directly; deref at each step if there's any chance new sprites get allocated.
- [ ] Sprites.
- - [ ] Pushable blocks.
- - [x] Oscillating traps.
- - [x] Roaming monsters. (multiple skins)
- - [x] Straight-shooting monsters.
- - [ ] Target-shooting monsters.
- - [ ] Sentinel monsters. Stands in one place, attacks when you draw near.
- [x] Monsters drop prizes when killed.
- [x] Overlay status bar with candy, pepper, and bomb quantities.
- [ ] Boss fight, immediately before rescue, once per session.
- [ ] Modals.
- - [ ] Game over. Success only. Game will not be loseable.
- [x] Maps.
- [ ] 3 side quests.
- [ ] Consider preserving one or two maps' worth of "all monsters dead". So you can go in first, kill the monsters, then come back with the Princess.
- [ ] Proper graphics.
- [ ] Visual fanfare on picking up prizes.
- [ ] '' opening treadle (etc) locks.
- [ ] Decorative snow during a snowglobe earthquake? Might be conspicuously nes-implausible.
- [ ] Consider simplifying explosion and pepper-fire; they are both pretty obviously not possible for NES.
- [ ] Sound effects.
- [ ] Music.
- [ ] Prevent monsters from triggering treadles by accident. Keep it possible when deliberate, eg with wand.
- [ ] Game over: Trigger when entering throne room with the princess.
- [ ] There's a bug somewhere in collision detection that lets you pass thru solid sprites sometimes when also colliding against the map. Ignoring it for now.
- [x] Script to validate images: No more than 4 colors per tile, counting transparency, and all opaque colors must be in palette.
- [ ] Interrupted transitions snap complete before proceeding (eg enter a neighbor map and quickly change your mind). Is it feasible to start partway thru in those cases?
- [ ] Errata.
- - [ ] Itch page.

## Agenda

- [x] 2025-05-30T07:00 F Jam begins.
- [x] 2025-05-30 EOD   F Have global framework ready: Modals, map loader, state engine, sprite loader.
- [x] 2025-05-31 EOD   S Hero, Princess, transitions, dialogue.
- [x] 2025-06-01 EOD   U Sprites and items.
- [x] 2025-06-06 EOD   F Rough world layout.
- [ ] 2025-06-07 EOD   S Sound effects.
- [ ] 2025-06-08 EOD   U Music, final graphics.
- [ ] 2025-06-13 EOD   F Polished and ready to go.
- [ ] 2025-06-14 EOD   S '' panic if not.
- [ ] 2025-06-15 EOD   U Have everything finished and submitted.
- [ ] 2025-06-16T07:00 M Jam ends.
