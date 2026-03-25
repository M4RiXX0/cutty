#!/usr/bin/env python3
from pathlib import Path
import json
import subprocess
import sys
import shutil

VERSION = "1.0"

def human_size(size_bytes: int) -> str:
    if size_bytes == 0:
        return "0 bytes"
    for unit, div in [("GB", 1<<30), ("MB", 1<<20), ("KB", 1<<10), ("bytes", 1)]:
        if size_bytes >= div:
            return f"{size_bytes / div:.2f} {unit}"
    return "0 bytes"


def human_duration(seconds: float) -> str:
    if seconds <= 0:
        return "0.00s"
    days    = int(seconds // 86400)
    hours   = int((seconds % 86400) // 3600)
    minutes = int((seconds % 3600) // 60)
    secs    = seconds % 60
    if days:    return f"{days}d {hours:02d}:{minutes:02d}:{secs:05.2f}"
    if hours:   return f"{hours:02d}:{minutes:02d}:{secs:05.2f}"
    if minutes: return f"{minutes:02d}:{secs:05.2f}"
    return f"{secs:.2f}s"


def get_probe_data(path: Path) -> dict:
    cmd = ["ffprobe", "-v", "quiet", "-print_format", "json",
           "-show_format", "-show_streams", str(path)]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, check=True)
        return json.loads(r.stdout)
    except Exception as e:
        print(f"ffprobe error: {e}", file=sys.stderr)
        sys.exit(2)


def fps_from_str(s: str) -> float:
    if '/' not in s: return 0.0
    try:
        num, den = map(int, s.split('/'))
        return num / den if den else 0.0
    except:
        return 0.0


def show_help():
    print("""cutty – simple ffmpeg wrapper
Made by M4RiXX

Usage:
cutty -i <file> [options]

Options:
--play                play file with ffplay
--info [-h]           show basic file information
--size [-h]           show file size
--duration [-h]       show duration
--resolution          show video resolution
--framerate           show video framerate (fps)
--codec               show video codec name
--pixel-format        show video pixel format
--convert -o <output>
    [--preset <name>] convert to H.264 (default: fast)
    [--fps <number>]  force output framerate
--version             show version
--help                show this help
""")
    sys.exit(0)


args = sys.argv[1:]

if not shutil.which("ffmpeg") or not shutil.which("ffprobe"):
    if len(args) > 1 and args[1] in ("--help", "--version"):
        if not args or "--help" in args:
            show_help()
        if "--version" in args:
            print(f"cutty {VERSION}")
            if shutil.which("ffmpeg"):
                subprocess.run(["ffmpeg", "-version"], check=False)
        sys.exit(0)
    
    print("ffmpeg / ffprobe is not installed", file=sys.stderr)
    sys.exit(1)

if not args or "--help" in args:
    show_help()
    sys.exit(0)

if "--version" in args or "-v" in args:
    print(f"cutty {VERSION}")
    subprocess.run(["ffmpeg", "-version"], check=False)
    sys.exit(0)

i = args.index("-i")
if i + 1 >= len(args):
    print("-i requires file path", file=sys.stderr)
    sys.exit(1)

input_path = Path(args[i + 1]).resolve()
if not input_path.is_file():
    print(f"File not found: {input_path}", file=sys.stderr)
    sys.exit(1)

human = "-h" in args

if "--play" in args:
    subprocess.run(["ffplay", str(input_path)], check=True)
    sys.exit(0)

if "--info" in args:
    data = get_probe_data(input_path)
    print(f"File:        {input_path.name}")
    size_b = int(data["format"].get("size", 0))
    print(f"Size:        {human_size(size_b) if human else f'{size_b} bytes'}")
    dur = float(data["format"].get("duration", 0))
    print(f"Duration:    {human_duration(dur) if human else f'{dur:.2f}s'}")

    for idx, s in enumerate(data.get("streams", []), 1):
        codec = s.get("codec_name", "?")
        typ = s.get("codec_type", "?").upper()

        if typ == "VIDEO":
            w = s.get("width", "?")
            h = s.get("height", "?")
            fps = fps_from_str(s.get("r_frame_rate", "0/1"))
            pix = s.get("pix_fmt", "?")
            print(f"\nVideo {idx}:    {w}x{h}  {codec}  {fps:.3f} fps  {pix}")
        elif typ == "AUDIO":
            ch = s.get("channels", "?")
            sr = s.get("sample_rate", "?")
            print(f"\nAudio {idx}:    {codec}  {ch} ch  {sr} Hz")
        elif typ == "SUBTITLE":
            print(f"\nSubtitle {idx}: {codec}")
    sys.exit(0)

if "--size" in args:
    data = get_probe_data(input_path)
    size_b = int(data["format"].get("size", 0))
    print(human_size(size_b) if human else f"{size_b} bytes")
    sys.exit(0)

if "--duration" in args:
    data = get_probe_data(input_path)
    dur = float(data["format"].get("duration", 0))
    print(human_duration(dur) if human else f"{dur:.2f}s")
    sys.exit(0)

if "--resolution" in args:
    data = get_probe_data(input_path)
    for s in data.get("streams", []):
        if s.get("codec_type") == "video":
            print(f"{s.get('width', '?')}x{s.get('height', '?')}")
            break
    sys.exit(0)

if "--framerate" in args:
    data = get_probe_data(input_path)
    for s in data.get("streams", []):
        if s.get("codec_type") == "video":
            fps = fps_from_str(s.get("r_frame_rate", "0/1"))
            print(f"{fps:.3f}")
            sys.exit(0)

if "--codec" in args:
    data = get_probe_data(input_path)
    for s in data.get("streams", []):
        if s.get("codec_type") == "video":
            print(s.get("codec_name", "?"))
            break
    sys.exit(0)

if "--pixel-format" in args:
    data = get_probe_data(input_path)
    for s in data.get("streams", []):
        if s.get("codec_type") == "video":
            print(s.get("pix_fmt", "?"))
            break
    sys.exit(0)

if "--convert" in args:
    if "-o" not in args:
        print("missing -o output file", file=sys.stderr)
        sys.exit(1)
    o_idx = args.index("-o")
    if o_idx + 1 >= len(args):
        print("-o requires filename", file=sys.stderr)
        sys.exit(1)
    out_path = Path(args[o_idx + 1]).resolve()

    preset = "fast"
    if "--preset" in args:
        p_idx = args.index("--preset")
        if p_idx + 1 < len(args):
            preset = args[p_idx + 1]

    fps = None
    if "--fps" in args:
        f_idx = args.index("--fps")
        if f_idx + 1 < len(args):
            try:
                fps = float(args[f_idx + 1])
            except ValueError:
                print("invalid --fps value", file=sys.stderr)
                sys.exit(1)

    cmd = [
        "ffmpeg", "-i", str(input_path),
        "-c:v", "libx264", "-preset", preset,
    ]
    if fps is not None:
        cmd += ["-r", str(fps)]
    cmd += [str(out_path)]

    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Conversion failed: {e}", file=sys.stderr)
        sys.exit(3)
    sys.exit(0)

print("Unknown option combination. Use --help", file=sys.stderr)
sys.exit(1)