# Foxhollow Fly Mode

A standalone native mod for Star Fox Adventures running through Foxhollow. It lets the player fly up and down during normal on-foot gameplay, and return to the last safe position.

It is based on the already-tested Fly Mode from the Foxhollow `feature/cheat-menu-fly` branch, controlled directly by the Home, End, Page Up and Page Down keys.

## Controls

| Key | Action |
| --- | --- |
| Home | Toggle Fly Mode |
| End | Return to Safe Position |
| Page Up | Fly Up |
| Page Down | Fly Down |

- Page Up and Page Down work while held. Holding both at once cancels out.
- Home and End act once per press. Holding them does not repeat.
- A Page Up or Page Down key that is already held when Fly Mode turns on, or when the game window regains focus, has to be released and pressed again before it moves the player.
- Keys only work during gameplay while the Foxhollow window has keyboard focus.
- On compact keyboards and laptops, Home, End, Page Up and Page Down may need Fn or another keyboard layer.
- On Windows, the numeric keypad's 7, 1, 9 and 3 keys also act as Home, End, Page Up and Page Down while Num Lock is off.
- Fly Mode starts off and turns off when you leave the current save (returning to the title screen, the save select or a soft reset).
- There is no on-screen display. Changes are written to the Foxhollow log, for example `[Fly Mode] Enabled`.

## Behavior

- Fly Mode works while Fox or Krystal is controllable on foot, including while swimming and in combat stances. It does nothing in the Arwing, on the CloudRunner, during cutscenes, while climbing, while carrying an object or while controls are locked.
- Page Up and Page Down do nothing while the Viewfinder or the World Map is open.
- Horizontal movement stops when the next step would leave the map, including when another mod makes that step longer (such as Fast Movement).
- While Fly Mode is on, the safe position is updated automatically, but only while the player is somewhere valid: inside the map, with floor below, and not on a moving platform, warp or scripted interaction.
- End also works after Fly Mode is turned off, returning to the last safe position saved while it was on.
- Changing map or layer, or warping, clears the saved safe position, so End never returns you to a position from another area.

## Compatibility

Works on its own, and together with:

- [Foxhollow Player Cheats](https://github.com/saulob/Foxhollow-Player-Cheats): Fast Movement doubles horizontal flying speed. Fly Up and Fly Down keep their normal speed.
- [Foxhollow Noclip](https://github.com/saulob/Foxhollow-Noclip): fly through walls. Noclip's fall protection steps aside while Fly Mode controls the height (rising, descending or hovering).

All three mods can be installed and used at the same time, in any load order.

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
      linux-amd64/
        mod.so
      linux-arm64/
        mod.so
      macos-x86_64/
        mod.so
      macos-arm64/
        mod.so
```

Foxhollow only loads the library in the folder that matches your system and ignores the others, so you only need the folder for your platform.

Restart the game after installing.

## Platform support

| Platform | Folder | Status |
| --- | --- | --- |
| Windows x64 | `windows-amd64` | Tested in game |
| Linux x86_64 | `linux-amd64` | Build validation by GitHub Actions pending, in-game testing pending |
| Linux ARM64 | `linux-arm64` | Build validation by GitHub Actions pending, in-game testing pending |
| macOS Apple Silicon | `macos-arm64` | Build validation by GitHub Actions pending, in-game testing pending |
| macOS Intel | `macos-x86_64` | Build validation by GitHub Actions pending, in-game testing pending |

Official Foxhollow builds are currently published for Windows x64, Linux x86_64 and macOS Apple Silicon. The Linux ARM64 and macOS Intel libraries are for Foxhollow builds you compile yourself. Windows on ARM is not supported.

On Linux and macOS, Home, End, Page Up and Page Down are read from the keyboard state of Foxhollow's own SDL3 runtime, so the mod needs no extra libraries (no separate SDL install) and does not use X11, Wayland or macOS keyboard APIs directly. If the Foxhollow log shows `[Fly Mode] disabled: ...`, the mod could not find the game functions or keyboard input it needs and left the game unchanged.

## Repository

https://github.com/saulob/Foxhollow-Fly-Mode

## License

[MIT](LICENSE)
