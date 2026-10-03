# Foxhollow Fly Mode

A standalone native mod for Star Fox Adventures running through Foxhollow. It lets the player fly up and down during normal on-foot gameplay, and return to the last safe position.

It is based on the already-tested Fly Mode from the Foxhollow `feature/cheat-menu-fly` branch, controlled directly by keys F5-F8.

## Controls

| Key | Action |
| --- | --- |
| F5 | Toggle Fly Mode |
| F6 | Fly Up |
| F7 | Fly Down |
| F8 | Return to Safe Position |

- F6 and F7 work while held. Holding both at once cancels out.
- F5 and F8 act once per press. Holding them does not repeat.
- A key that is already held when Fly Mode turns on, or when the game window regains focus, has to be released and pressed again before it moves the player.
- Keys only work during gameplay while the game window is focused.
- Fly Mode starts off and turns off when you leave the current save (returning to the title screen, the save select or a soft reset).
- There is no on-screen display. Changes are written to the Foxhollow log, for example `[Fly Mode] Enabled`.

## Behavior

- Fly Mode works while Fox or Krystal is controllable on foot, including while swimming and in combat stances. It does nothing in the Arwing, on the CloudRunner, during cutscenes, while climbing, while carrying an object or while controls are locked.
- F6 and F7 do nothing while the Viewfinder or the World Map is open.
- Horizontal movement stops when the next step would leave the map.
- While Fly Mode is on, the safe position is updated automatically, but only while the player is somewhere valid: inside the map, with floor below, and not on a moving platform, warp or scripted interaction.
- F8 also works after Fly Mode is turned off, returning to the last safe position saved while it was on.
- Changing map or layer, or warping, clears the saved safe position, so F8 never returns you to a position from another area.

## Installation

**Recommended:** install through the Foxhollow Launcher once the mod is published there.

**Manual:** place the extracted mod folder in the Foxhollow Launcher's `mods` folder, so it looks like this:

```
mods/
  com.saulob.fly-mode/
    mod.json
    lib/
      windows-amd64/
        mod.dll
```

Restart the game after installing.

## Platform support

- Windows x64

Other Foxhollow platforms are not supported by this mod yet.

## Repository

https://github.com/saulob/Foxhollow-Fly-Mode

## License

[MIT](LICENSE)
