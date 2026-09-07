#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <iomanip>

#include "subprocess.h"

int run_cmd(const std::vector<std::string>& args) {
    std::vector<const char*> c_args;
    for (const auto& arg : args) {
        c_args.push_back(arg.c_str());
    }
    c_args.push_back(nullptr);

    struct subprocess_s process;
    int options = subprocess_option_search_user_path | subprocess_option_inherit_environment;
    int result = subprocess_create(c_args.data(), options, &process);
    if (result != 0) return -1;

    int process_return = 0;
    subprocess_join(&process, &process_return);
    subprocess_destroy(&process);
    return process_return;
}

std::string capture_cmd(const std::vector<std::string>& args) {
    std::vector<const char*> c_args;
    for (const auto& arg : args) {
        c_args.push_back(arg.c_str());
    }
    c_args.push_back(nullptr);

    struct subprocess_s process;
    int options = subprocess_option_search_user_path | subprocess_option_inherit_environment;
    int result = subprocess_create(c_args.data(), options, &process);
    if (result != 0) return "";

    FILE* p_stdout = subprocess_stdout(&process);
    std::string output = "";
    if (p_stdout) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), p_stdout) != nullptr) {
            output += buffer;
        }
    }

    int process_return = 0;
    subprocess_join(&process, &process_return);
    subprocess_destroy(&process);
    return output;
}

bool is_ffmpeg_installed() {
    const char* args[] = {"ffmpeg", "-version", nullptr};
    struct subprocess_s process;
    int options = subprocess_option_search_user_path | subprocess_option_inherit_environment;
    
    int result = subprocess_create(args, options, &process);
    if (result != 0) return false;

    int process_return = 0;
    subprocess_join(&process, &process_return);
    subprocess_destroy(&process);

    return (process_return == 0);
}

uint64_t get_file_size(const std::string& path) {
    try {
        return std::filesystem::file_size(path);
    } catch (...) {
        return 0;
    }
}

std::string format_size(uint64_t bytes, bool human) {
    if (!human) {
        return std::to_string(bytes);
    }

    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    if (bytes >= GB) {
        ss << (bytes / GB) << " GB";
    } else if (bytes >= MB) {
        ss << (bytes / MB) << " MB";
    } else if (bytes >= KB) {
        ss << (bytes / KB) << " KB";
    } else {
        ss << bytes << " B";
    }

    return ss.str();
}

double parse_fps(const std::string& fps_str) {
    size_t slash_pos = fps_str.find('/');
    if (slash_pos == std::string::npos) {
        try { return std::stod(fps_str); } catch (...) { return 0.0; }
    }

    try {
        double num = std::stod(fps_str.substr(0, slash_pos));
        double den = std::stod(fps_str.substr(slash_pos + 1));
        if (den == 0.0) return 0.0;
        return num / den;
    } catch (...) {
        return 0.0;
    }
}

bool has_video_stream(const std::string& filepath) {
    std::string result = capture_cmd({
        "ffprobe", "-v", "error", 
        "-select_streams", "v:0", 
        "-show_entries", "stream=codec_type", 
        "-of", "default=noprint_wrappers=1:nokey=1", 
        filepath
    });

    return (result.find("video") != std::string::npos);
}

std::string format_duration(double total_seconds, bool human) {
    if (!human) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(6) << total_seconds;
        return ss.str();
    }

    int total_s = static_cast<int>(total_seconds);
    int hours = total_s / 3600;
    int minutes = (total_s % 3600) / 60;
    int seconds = total_s % 60;

    std::stringstream ss;
    if (hours > 0) {
        ss << hours << "h " << minutes << "m " << seconds << "s";
    } else if (minutes > 0) {
        ss << minutes << "m " << seconds << "s";
    } else {
        ss << std::fixed << std::setprecision(2) << total_seconds << "s";
    }

    return ss.str();
}

int main(int argc, char* argv[]) {
    const std::string VERSION = "1.0";
    
    bool show_version = false;
    bool show_help = false;
    bool play = false;
    bool audio_only = false;
    bool extract_audio = false;
    bool show_info = false;
    bool show_human = false;
    bool show_size = false;
    bool show_duration = false;
    bool show_resolution = false;
    bool show_width = false;
    bool show_height = false;
    bool show_framerate = false;
    bool show_codec = false;
    bool show_pixel_format = false;
    bool convert = false;

    std::string input_file = "";
    std::string output_file = "";

    std::string preset = "fast";
    double fps = 0.0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--version") {
            show_version = true;
        } else if (arg == "--help") {
            show_help = true;
        } else if (arg == "--info") {
            show_info = true;
        } else if (arg == "--human") {
            show_human = true;
        } else if (arg == "--play") {
            play = true;
        } else if (arg == "--audio-only") {
            audio_only = true;
        } else if (arg == "--extract-audio" || arg == "-e") {
            extract_audio = true;
        } else if (arg == "--size") {
            show_size = true;
        } else if (arg == "--duration") {
            show_duration = true;
        } else if (arg == "--resolution") {
            show_resolution = true;
        } else if (arg == "--width") {
            show_width = true;
        } else if (arg == "--height") {
            show_height = true;
        } else if (arg == "--framerate") {
            show_framerate = true;
        } else if (arg == "--codec") {
            show_codec = true;
        } else if (arg == "--convert" || arg == "-c") {
            convert = true;
        } else if (arg == "--preset") {
            if (i + 1 < argc) preset = argv[++i];
            else { std::cerr << "Error: Missing preset value after " << arg << "\n"; return 1; }
        } else if (arg == "--fps") {
            if (i + 1 < argc) {
                try {
                    fps = std::stod(argv[++i]);
                } catch (...) {
                    std::cerr << "Error: Invalid fps value.\n";
                    return 1;
                }
            } else { 
                std::cerr << "Error: Missing fps value after " << arg << "\n"; return 1; }
        } else if (arg == "--pixel-format") {
            show_pixel_format = true;
        } else if (arg == "--input" || arg == "-i") {
            if (i + 1 < argc) input_file = argv[++i];
            else { std::cerr << "Error: Missing input file path after " << arg << "\n"; return 1; }
        } else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) output_file = argv[++i];
            else { std::cerr << "Error: Missing output file path after " << arg << "\n"; return 1; }
        } else if (arg.length() >= 2 && arg[0] == '-' && arg[1] != '-') {
            for (size_t j = 1; j < arg.length(); ++j) {
                char c = arg[j];
                switch (c) {
                    case 'v': show_version = true; break;
                    case 'h': show_help = true; break;
                    case 'H': show_human = true; break;
                    case 'I': show_info = true; break;
                    case 'a': audio_only = true; break;
                    case 'p': play = true; break;
                    case 'e':
                        std::cerr << "Error: Option -" << c << " cannot be combined with other flags.\n";
                        return 1;
                    case 'i':
                    case 'o':
                    case 'c':
                        std::cerr << "Error: Option -" << c << " requires an argument and cannot be combined with other flags.\n";
                        return 1;
                    default:
                        std::cerr << "Warning: Unknown option -" << c << "\n";
                }
            }
        } else {
            std::cerr << "Warning: Unknown argument " << arg << "\n";
        }
    }

    if (show_help) {
        std::cout 
            << "Cutty v" << VERSION << " - simple ffmpeg wrapper by M4RiXX\n\n"
            << "USAGE:\n"
            << "  cutty -i <file> [options]\n\n"
            << "GLOBAL OPTIONS:\n"
            << "  -h, --help              Show this help message and exit\n"
            << "  -v, --version           Show program's version number and exit\n"
            << "  -i, --input <file>      Input video/audio file path\n\n"
            << "INSPECTION OPTIONS (Read-only):\n"
            << "  -I, --info              Show all file information\n"
            << "  -H, --human             Display sizes/durations in human-readable format\n"
            << "      --size              Show file size only\n"
            << "      --duration          Show duration only\n"
            << "      --resolution        Show video resolution only\n"
            << "      --width             Show video width only\n"
            << "      --height            Show video height only\n"
            << "      --framerate         Show video framerate (fps) only\n"
            << "      --codec             Show video codec name only\n"
            << "      --pixel-format      Show video pixel format only\n\n"
            << "ACTIONS:\n"
            << "  -p, --play              Play file with ffplay\n"
            << "  -a, --audio-only        Use with -p or --play to play audio only\n"
            << "  -e, --extract-audio     Extract audio only\n"
            << "  -c, --convert           Convert video to H.264\n\n"
            << "EXTRACTION MODIFIERS (Use with -e):\n"
            << "  -o, --output <file>     Output file path (required for extraction)\n\n"
            << "CONVERSION MODIFIERS (Use with -c):\n"
            << "  -o, --output <file>     Output file path (required for conversion)\n"
            << "      --preset <preset>   H.264 preset: ultrafast..veryslow (default: " << preset << ")\n"
            << "      --fps <fps>         Force output framerate\n";

        return 0;
    }

    if (show_version) {
        std::cout << "version: " << VERSION << "\n";
        return 0;
    }

    // Sprawdzenie obecności FFmpeg przed wykonaniem jakichkolwiek akcji
    if (!is_ffmpeg_installed()) {
        std::cerr << "Error: 'ffmpeg' is not installed or not available in system PATH.\n"
                  << "Please install FFmpeg to use this application.\n";
        return 1;
    }

    if (input_file.empty() && output_file.empty() && !show_info) {
        std::cerr << "Error: No valid options provided. Use -h for help.\n";
        return 1;
    }

    if (!input_file.empty()) {
        if (!std::filesystem::exists(input_file)) {
            std::cerr << "Error: File does not exist\n";
            return 1;
        }

        if (show_info) {
            int result = run_cmd({"ffprobe", "-hide_banner", input_file});
            if (result != 0) {
                std::cerr << "Error: Could not retrieve info for file: " << input_file << "\n";
                return 1;
            }
            return 0;
        }

        if (play) {
            std::vector<std::string> cmd = {"ffplay", "-loglevel", "quiet", "-autoexit"};
            if (audio_only || !has_video_stream(input_file)) {
                cmd.push_back("-vn");
                cmd.push_back("-showmode");
                cmd.push_back("1");
            }
            cmd.push_back(input_file);

            run_cmd(cmd);
            return 0;
        }

        if (show_resolution) {
            std::string res = capture_cmd({
                "ffprobe", "-hide_banner", "-v", "error", 
                "-select_streams", "v:0", 
                "-show_entries", "stream=width,height", 
                "-of", "compact=p=0:nk=1:s=x", 
                input_file
            });
            std::cout << res;
            return 0;
        }

        if (show_width) {
            std::string width = capture_cmd({
                "ffprobe", "-hide_banner", "-v", "error", 
                "-select_streams", "v:0", 
                "-show_entries", "stream=width", 
                "-of", "default=noprint_wrappers=1:nokey=1", 
                input_file
            });
            std::cout << width;
            return 0;
        }

        if (show_height) {
            std::string height = capture_cmd({
                "ffprobe", "-hide_banner", "-v", "error", 
                "-select_streams", "v:0", 
                "-show_entries", "stream=height", 
                "-of", "default=noprint_wrappers=1:nokey=1", 
                input_file
            });
            std::cout << height;
            return 0;
        }

        if (show_pixel_format) {
            std::string pix_fmt = capture_cmd({
                "ffprobe", "-hide_banner", "-v", "error", 
                "-select_streams", "v:0", 
                "-show_entries", "stream=pix_fmt", 
                "-of", "default=noprint_wrappers=1:nokey=1", 
                input_file
            });
            std::cout << pix_fmt;
            return 0;
        }

        if (show_codec) {
            std::string codec = capture_cmd({
                "ffprobe", "-hide_banner", "-v", "error", 
                "-select_streams", "v:0", 
                "-show_entries", "stream=codec_name", 
                "-of", "default=noprint_wrappers=1:nokey=1", 
                input_file
            });
            std::cout << codec;
            return 0;
        }

        if (show_framerate) {
            std::string raw_fps = capture_cmd({
                "ffprobe", "-hide_banner", "-v", "error", 
                "-select_streams", "v:0", 
                "-show_entries", "stream=r_frame_rate", 
                "-of", "default=noprint_wrappers=1:nokey=1", 
                input_file
            });

            double parsed_fps = parse_fps(raw_fps);

            if (show_human) {
                std::cout << std::fixed << std::setprecision(2) << parsed_fps << " fps\n";
            } else {
                std::cout << std::fixed << std::setprecision(2) << parsed_fps << "\n";
            }

            return 0;
        }

        if (show_duration) {
            std::string raw_duration = capture_cmd({
                "ffprobe", "-hide_banner", "-v", "error", 
                "-show_entries", "format=duration", 
                "-of", "default=noprint_wrappers=1:nokey=1", 
                input_file
            });

            double seconds = 0.0;
            try {
                seconds = std::stod(raw_duration);
            } catch (...) {
                std::cerr << "Error: Could not parse duration\n";
                return 1;
            }

            std::cout << format_duration(seconds, show_human) << "\n";
            return 0;
        }

        if (show_size) {
            uint64_t bytes = get_file_size(input_file);
            std::cout << format_size(bytes, show_human) << "\n";
            return 0;
        }

        if (extract_audio) {
            if (output_file.empty()) {
                std::cerr << "Error: Output file not specified (-o)\n";
                return 1;
            }

            int result = run_cmd({
                "ffmpeg", "-hide_banner", "-loglevel", "error", 
                "-y", "-i", input_file, 
                "-vn", output_file
            });

            if (result != 0) {
                std::cerr << "Error: Extraction failed.\n";
                return 1;
            }
            return 0;
        }

        if (convert) {
            if (output_file.empty()) {
                std::cerr << "Error: Output file not specified\n";
                return 1;
            }

            if (preset != "ultrafast" && preset != "superfast" && preset != "veryfast" && 
                preset != "faster" && preset != "fast" && preset != "medium" && 
                preset != "slow" && preset != "slower" && preset != "veryslow") {
                std::cerr << "Error: Invalid preset value.\n";
                return 1;
            }

            std::vector<std::string> cmd = {
                "ffmpeg", "-hide_banner", "-loglevel", "error", 
                "-y", "-i", input_file, 
                "-c:v", "libx264", 
                "-preset", preset
            };

            if (fps != 0.0) {
                cmd.push_back("-r");
                cmd.push_back(std::to_string(fps));
            }

            cmd.push_back(output_file);

            run_cmd(cmd);
            return 0;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified (-i / --input)\n";
        return 1;
    }

    return 0;
}