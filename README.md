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
- [ ] Sprites.
- - [ ] Hero.
- - [ ] Princess.
- - [ ] Switchable flamethrowers.
- - [ ] Pushable blocks.
- - [ ] Switchable doors.
- - [ ] Oscillating traps.
- - [ ] Responsive traps.
- - [ ] Roaming monsters. (multiple skins)
- - [ ] Targetted fireballs. Target the Hero or the Princess?
- - [ ] Straight-shooting monsters.
- - [ ] Target-shooting monsters.
- - [ ] Sentinel monsters. Stands in one place, attacks when you draw near.
- [ ] Boss fight, immediately before rescue, once per session.
- [ ] Dialogue.
- [ ] Items.
- - [ ] Broom. Like Full Moon.
- - [ ] Pepper. Burn everything within some radius. Light candles. Kill Princess.
- - [ ] Bell. Goes jingle jingle.
- - [ ] Stopwatch. Like Too Heavy.
- - [ ] Camera. Like Too Heavy. Maybe prompt when actuated "warp or replace?"
- - [ ] Snowglobe. Like Full Moon.
- - [ ] Wand. Summon moveable objects. (not for general spell casting, that won't be a thing).
- - [ ] Bomb. Destroy things, esp Princesses.
- - [ ] Candy. Distact monsters.
- [ ] Modals.
- - [ ] Hello.
- - [ ] Game over. Success only. Game will not be loseable.
- - [ ] Pause. Inventory and options, like Full Moon.
- [ ] Maps.
- [ ] Proper graphics.
- [ ] Sound effects.
- [ ] Music.
- [ ] Errata.
- - [ ] Itch page.

## Agenda

- [x] 2025-05-30T07:00 F Jam begins.
- [x] 2025-05-30 EOD   F Have global framework ready: Modals, map loader, state engine, sprite loader.
- [ ] 2025-05-31 EOD   S Hero, Princess, transitions, dialogue.
- [ ] 2025-06-01 EOD   U Sprites and items.
- [ ] 2025-06-06 EOD   F Rough world layout.
- [ ] 2025-06-07 EOD   S Sound effects.
- [ ] 2025-06-08 EOD   U Music, final graphics.
- [ ] 2025-06-13 EOD   F Polished and ready to go.
- [ ] 2025-06-14 EOD   S '' panic if not.
- [ ] 2025-06-15 EOD   U Have everything finished and submitted.
- [ ] 2025-06-16T07:00 M Jam ends.
