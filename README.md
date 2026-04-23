# cutty

Super simple CLI wrapper for `ffmpeg` / `ffprobe`  
Quickly get video file info and convert it – no fuss

## Features

- `--info [-h]` – show file info (size, duration, resolution, fps, codecs, pixel format…)
- `--play` – play file using ffplay
- Single shortcuts: `--size`, `--duration`, `--resolution`, `--framerate`, `--codec`, `--pixel-format`
- `--convert -o output.mp4 [--preset fast] [--fps 30]` – encode to H.264 (libx264)

## Requirements

**Always needed (on every platform):**
- [ffmpeg](https://ffmpeg.org/download.html) + ffprobe (included in the same package)

**cutty itself:**
- **Windows** → standalone `.exe` (no Python required)
- **Windows** → standalone `.deb` (no Python required)

Other Linux distros and macOS are not yet officially supported (you can still build from source if needed).

## Installation

### Windows

1. Go to [Releases](https://github.com/M4RiXX0/cutty/releases)
2. Download the latest `cutty.exe`
3. Move it to any folder (recommended: `C:\Tools\` or `C:\Program Files\cutty\`)
4. **Add that folder to your PATH**:
   - Search for "Environment Variables" in Windows Search
   - Edit the **Path** variable (System or User)
   - Add your folder path → OK
   - Restart Command Prompt / PowerShell
5. Install **ffmpeg**:
   - Download from https://www.gyan.dev/ffmpeg/builds/ → **ffmpeg-release-essentials.zip** (recommended)
   - Extract to e.g. `C:\ffmpeg\`
   - Add `C:\ffmpeg\bin` to PATH (same way as above)

### Debian / Ubuntu / Linux Mint

1. Go to [Releases](https://github.com/M4RiXX0/cutty/releases)
2. Download the latest `cutty.deb`
3. Install
4. Run apt install ffmpeg

Test it:

```cmd
cutty --version
cutty -i video.mp4 --info -h
