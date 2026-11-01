# Infill Clock

A 3D-printing watchface for Pebble. The time is drawn as hollow digits with
solid outer walls. Inside the walls, an infill pattern rises through the hour
as the minutes pass — like a print filling in on the bed.

The infill style changes every hour (lines, honeycomb, wavy gyroid, triangles),
so a glance at the pattern roughly tells you which hour block you are in.
Color watches also tint the infill by hour.

## Platforms

Pebble Time 2 (Emery), Pebble Time (Basalt), Pebble 2 / Time Round-class
(Diorite), and Pebble 2 Duo (Flint).

## Build and install

The Pebble SDK runs on Linux and macOS only. On **Windows**, `build.ps1` runs it
in a container (install [Podman Desktop](https://podman-desktop.io) or Docker
Desktop first):

```powershell
./build.ps1
```

The first run builds the container image and downloads the SDK. The watchface
ends up in `build/InfillClock.pbw`.

On **macOS/Linux**, [install the SDK](https://developer.repebble.com/sdk) and
run `pebble build`.

Put it on the watch by opening the `.pbw` in the Pebble app, or:

```powershell
./build.ps1 -Phone 192.168.1.23
```

## CloudPebble

Import `dist/Infill-Clock-cloudpebble.zip` into CloudPebble (or rebuild it with
`python build-env/make_zip.py`).

## How to read it

- **Digit shapes** — hours and minutes (`HH:MM`).
- **Infill height** — progress through the hour (empty near `:00`, full by `:59`).
- **Infill pattern** — changes each hour: lines → honeycomb → gyroid → triangles.
- **Infill color** — changes on the hour (color watches).

## Settings

Open the gear icon next to Infill Clock in the Pebble phone app:

- **Time** — 12h / 24h / system, leading zero
- **Date** — show/hide and format
- **Display** — pattern caption, lock pattern or keep auto-by-hour
- **Colors** (color watches) — background, walls, caption, auto/custom infill
