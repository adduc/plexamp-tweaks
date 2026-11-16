# plexamp-tweaks

An `LD_PRELOAD` shim that lets you disable smooth scrolling and set a default zoom level in the
**Tauri / WebKitGTK** builds of [Plexamp](https://www.plex.tv/plexamp/) for Linux (4.50.x beta
and later), installed from Flathub.

## Why

Plexamp 4.13 and earlier were Electron apps, so Chromium flags like `--disable-smooth-scrolling`
and `--force-device-scale-factor` worked. The newer beta builds use Tauri, which renders with
WebKitGTK (`libwebkit2gtk-4.1`). WebKitGTK ignores Chromium flags, and no environment variable
controls smooth scrolling: that is a `WebKitSettings` property the app has to set itself.

Plexamp gets its settings object by calling `webkit_web_view_get_settings()` from the shared
library. This shim replaces that function. It calls the real one, then applies your settings the
first time it sees each webview.

## Requirements

- Plexamp from Flathub, beta branch (`com.plexamp.Plexamp//beta`)
- `gcc`, `make`, `flatpak`

## Usage

```sh
make override          # build, install, and write the flatpak override
```

Then quit Plexamp completely (including from the tray) and start it again.

Change the defaults with make variables:

```sh
make override ZOOM=1.1 SMOOTH_SCROLLING=0 HW_ACCEL=never
make override ZOOM=     # leave out zoom
```

| Target       | What it does                                                                 |
|--------------|------------------------------------------------------------------------------|
| `all`        | Builds `plexamp-tweaks.so`                                                   |
| `install`    | Copies it to `~/.var/app/com.plexamp.Plexamp/tweaks/`, where the sandbox can read it |
| `override`   | `install`, then sets the env vars with `flatpak override --user`             |
| `show`       | Prints the current user override                                             |
| `unoverride` | Removes only the variables this project sets                                 |
| `uninstall`  | `unoverride`, then deletes the installed library                             |

## Settings

| Variable                   | Values                         | Effect                                          |
|----------------------------|--------------------------------|-------------------------------------------------|
| `PLEXAMP_SMOOTH_SCROLLING` | `0` / `1`                      | `WebKitSettings:enable-smooth-scrolling`        |
| `PLEXAMP_ZOOM`             | e.g. `1.25`                    | Starting page zoom (`webkit_web_view_set_zoom_level`) |
| `PLEXAMP_HW_ACCEL`         | `never` / `always` / `ondemand`| `WebKitSettings:hardware-acceleration-policy`   |
| `PLEXAMP_TWEAKS_DEBUG`     | any value                      | Prints `[plexamp-tweaks] applied…` to stderr    |

To check that the shim is working:

```sh
flatpak run --env=PLEXAMP_TWEAKS_DEBUG=1 com.plexamp.Plexamp//beta
```

## Other scaling options (no shim needed)

- **Ctrl+Shift+`=`** / **Ctrl+Shift+`-`**: Plexamp's own font size control (bigger / smaller). This changes text size inside the app,
  separately from `PLEXAMP_ZOOM`, which scales the whole page.
- `GDK_SCALE=2`: whole-number GTK scaling.

Plexamp turns off Tauri's built-in page zoom keys, so plain Ctrl+`=` / Ctrl+`-` / Ctrl+`0` and
Ctrl+scroll wheel don't zoom. That makes `PLEXAMP_ZOOM` the only way to scale the whole page by a
fractional amount.
- `GDK_DPI_SCALE=1.2`: scales text only.

## Notes and limits

- The override and the library live outside the app install, so they survive Flatpak updates.
- `flatpak override` is per app ID, so it also applies to the stable (Electron) branch. The shim
  does nothing there.
- Smooth scrolling here means animated mouse-wheel and keyboard scrolling. Touchpad momentum
  scrolling may still happen.
- If a future Plexamp stops calling `webkit_web_view_get_settings`, the shim will silently do
  nothing. Use `PLEXAMP_TWEAKS_DEBUG=1` to check.
