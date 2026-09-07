# Cutty 🎬

**Cutty** is a lightweight, fast, and cross-platform C++ command-line wrapper around **FFmpeg** and **FFprobe**. It simplifies media file inspection, playback, audio extraction, and video transcoding through a clean, intuitive CLI interface.

Created by **M4RiXX**. Released under the **MIT License**.

---

## 🚀 Features

- 🔍 **Media File Inspection**: Quickly retrieve file size, duration, video resolution, width, height, framerate, codec, and pixel format.
- 🪶 **Human-Readable Output**: Optional `-H` / `--human` flag to format file sizes (KB, MB, GB) and durations (`1h 20m 15s`).
- ⏯️ **Instant Playback**: Streamlined integration with `ffplay` for video and audio playback.
- 🎵 **Audio Extraction**: Easily extract raw or processed audio tracks to specified output files.
- ⚡ **H.264 Video Conversion**: Transcode videos with customizable encoding presets (`ultrafast` to `veryslow`) and custom framerates (`--fps`).
- 🛡️ **Safe Execution**: Uses `subprocess.h` for command execution without vulnerable shell calls (`system()`).
- ⚠️ **Dependency Verification**: Automatically verifies if FFmpeg is installed in system `PATH` before performing operations.

---

## 🛠️ Prerequisites

To run Cutty, ensure you have **FFmpeg** (along with `ffprobe` and `ffplay`) installed and available in your system `PATH`:

- **Ubuntu / Debian**:
  ```bash
  sudo apt update && sudo apt install ffmpeg
  ```
- **Windows (Chocolatey / Scoop)**:
  ```powershell
  choco install ffmpeg
  ```

---

## 📥 Installation

Download the latest release for your operating system from the **[GitHub Releases](../../releases)** page.

### 🐧 Linux (Debian / Ubuntu)
1. Download the `.deb` package (`cutty_*_amd64.deb`).
2. Install it using `apt` (this automatically installs required dependencies):
   ```bash
   sudo apt install ./cutty_1.0.0_amd64.deb
   ```

### 🪟 Windows
- **Installer** (`.exe`): Download and run `cutty-setup.exe`. The installer will automatically add Cutty to your system `PATH`.
- **Portable** (`.zip`): Extract `cutty.exe` anywhere and manually add its directory to your System Environment Variables (`PATH`).

---

## 🏗️ Building from Source

To compile Cutty yourself, you need a C++17 compiler and **CMake 3.16+**:

```bash
# Clone the repository
git clone [https://github.com/M4RiXX0/cutty.git](https://github.com/M4RiXX0/cutty.git)
cd cutty

# Configure and build
cmake -B build
cmake --build build
```

The compiled executable will be located in the `build/` folder (`build/Release/` on Windows).

---

## 📖 Usage & Examples

### General Syntax
```bash
cutty -i <input_file> [options]
```

### 1. File Inspection (Read-Only)

| Flag | Description | Example |
| :--- | :--- | :--- |
| `-I`, `--info` | Full FFprobe metadata dump | `cutty -i test.mp4 -I` |
| `--size` | Display file size | `cutty -i test.mp4 --size -H` |
| `--duration` | Display video/audio duration | `cutty -i test.mp4 --duration -H` |
| `--resolution` | Display resolution (Width x Height) | `cutty -i test.mp4 --resolution` |
| `--width` | Display video width | `cutty -i test.mp4 --width` |
| `--height` | Display video height | `cutty -i test.mp4 --height` |
| `--framerate` | Display framerate (FPS) | `cutty -i test.mp4 --framerate -H` |
| `--codec` | Display video codec name | `cutty -i test.mp4 --codec` |
| `--pixel-format`| Display video pixel format | `cutty -i test.mp4 --pixel-format` |

*Tip: Add `-H` or `--human` to format numbers into human-readable units (e.g., `1m 30s` or `14.20 MB`).*

### 2. Media Playback

Play video with `ffplay`:
```bash
cutty -i video.mp4 -p
```

Play audio stream only:
```bash
cutty -i video.mp4 -pa
```
or
```bash
cutty -i video.mp4 -p -a
```

### 3. Audio Extraction

Extract audio track from video to an output file:
```bash
cutty -i input.mp4 -e -o audio.mp3
```

### 4. Video Transcoding (H.264)

Convert video to H.264 format with default preset (`fast`):
```bash
cutty -i input.mkv -c -o output.mp4
```

Specify custom encoding preset and framerate:
```bash
cutty -i input.mkv -c -o output.mp4 --preset slow --fps 60
```

Available presets: `ultrafast`, `superfast`, `veryfast`, `faster`, `fast`, `medium`, `slow`, `slower`, `veryslow`.

---

## 📜 Global Options

- `-h`, `--help`: Display help message and options list.
- `-v`, `--version`: Display Cutty version number (`1.0`).

---

## 📄 License

This project is open-source and licensed under the **MIT License**. See the [LICENSE](LICENSE) file for full details.