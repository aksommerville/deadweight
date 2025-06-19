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

## Agenda

- [x] 2025-05-30T07:00 F Jam begins.
- [x] 2025-05-30 EOD   F Have global framework ready: Modals, map loader, state engine, sprite loader.
- [x] 2025-05-31 EOD   S Hero, Princess, transitions, dialogue.
- [x] 2025-06-01 EOD   U Sprites and items.
- [x] 2025-06-06 EOD   F Rough world layout.
- [x] 2025-06-07 EOD   S Sound effects.
- [x] 2025-06-08 EOD   U Music, final graphics.
- [x] 2025-06-13 EOD   F Polished and ready to go.
- [x] 2025-06-14 EOD   S '' panic if not.
- [x] 2025-06-15 EOD   U Have everything finished and submitted.
- [x] 2025-06-16T07:00 M Jam ends.

## TODO

- [x] Should be able to trigger twopass gates by leaving the princess on one and cameraing to the other.
- - See video by ecaroh.games on our NES Jam submission page -- the princess's switch turns off when you teleport!
- [x] Return to throne with Princess afield. She's on her throne but shouldn't be.
