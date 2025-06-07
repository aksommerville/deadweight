# Dead Weight

Requires [Egg](https://github.com/aksommerville/egg) to build.

2025-05-30 For NES Jam 2025, theme "SAVE THE PRINCESS".

Short Zelda-style adventure. Rescue the Princess, then escort her back to the castle.
The gimmick: Rescuing her is stupid easy due to your overpowered inventory.
Once she's in your care, items become more difficult to use. (emergently, eg your broom only seats one).
If you die, the screen resets, no problemo.
If she dies, she respawns back at the dungeon.

What happens to the Princess when the Hero dies?
I guess she resets to initial position like everything else. ...better yet, nothing. She just carries on.

Strict single-screen presentation. Most volatile state will reset with screen changes.
256x224 pixels fb, makes exactly 16x14 cells of 16x16 pixels.
Don't use doors or ladders: There's just a single plane to the world.

Actions like Full Moon: (A) to use, (B) to choose.
One thing equipped at a time.
Some items will have a count: Pepper, Bomb, Candy.

## TODO

- [x] Shortcut switch just south of the broom chest: Require snowglobe to actuate.
- [ ] Pre-boss-fight cutscene?
- [ ] Post-boss-fight cutscene?
- [ ] Modals.
- - [ ] Game over. Success only. Game will not be loseable.
- [ ] 3 side quests.
- [x] Switch to turn off the knives, south of frontdoor, so you don't have to do that first loop more than once.
- [ ] NPCs with helpful dialogue.
- [ ] Proper graphics.
- [x] Earthquake something off the edge, it stops responding, good. But you should be able to earthquake it back toward the action, at least.
- [ ] Visual fanfare on picking up prizes.
- [ ] '' opening treadle (etc) locks.
- [ ] Decorative snow during a snowglobe earthquake? Might be conspicuously nes-implausible.
- [ ] Consider simplifying explosion and pepper-fire; they are both pretty obviously not possible for NES.
- [x] Sound effects.
- - [x] Injure hero.
- - [x] Injure princess.
- - [x] Injure boss.
- - [x] Injure monster.
- - [x] Stopwatch, ongoing.
- [ ] Music.
- - `we_need_norris`: Play, after boss.
- - `dead_weight`: Finale.
- - `tickled_pink`: Hello.
- - [ ] Play, before boss.
- - [ ] Boss fight.
- [x] Boss: Kill fireballs when he dies.
- [ ] Prevent monsters from triggering treadles by accident. Keep it possible when deliberate, eg with wand.
- [ ] Game over: Trigger when entering throne room with the princess.
- [ ] Sometimes face direction doesn't update after dropping the broom. Usually happens to me at the map right after acquiring broom.
- [ ] There's a bug somewhere in collision detection that lets you pass thru solid sprites sometimes when also colliding against the map. Ignoring it for now.
- [ ] Interrupted transitions snap complete before proceeding (eg enter a neighbor map and quickly change your mind). Is it feasible to start partway thru in those cases?
- [ ] Errata.
- - [ ] Itch page.

## Agenda

- [x] 2025-05-30T07:00 F Jam begins.
- [x] 2025-05-30 EOD   F Have global framework ready: Modals, map loader, state engine, sprite loader.
- [x] 2025-05-31 EOD   S Hero, Princess, transitions, dialogue.
- [x] 2025-06-01 EOD   U Sprites and items.
- [x] 2025-06-06 EOD   F Rough world layout.
- [x] 2025-06-07 EOD   S Sound effects.
- [ ] 2025-06-08 EOD   U Music, final graphics.
- [ ] 2025-06-13 EOD   F Polished and ready to go.
- [ ] 2025-06-14 EOD   S '' panic if not.
- [ ] 2025-06-15 EOD   U Have everything finished and submitted.
- [ ] 2025-06-16T07:00 M Jam ends.
