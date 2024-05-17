#pragma once

#include <filesystem>
#include <string_view>

#include "Sha256Hasher.hpp"
#include "PluginsOption.hpp"

constexpr std::string_view kBullet1{ "■" };
constexpr std::string_view kBullet2{ "●" };
constexpr std::string_view kBulletE{ "　" };
constexpr std::string_view kIndent{ "　" };

template<typename T>
bool inline HasFlag(T x, T flag)
{
    return (static_cast<uint32_t>(x) & static_cast<uint32_t>(flag)) != 0;
}

void inline WritePluginData(
    std::ostream& dest, Sha256Hasher& hasher, const PluginsOption& opt,
    const std::filesystem::path& aviutl_dir,
    const char* name, const char* info, const char* path)
{
    std::vector<std::string> lines;
    if (opt.enable_name) {
        lines.push_back("名前: ");
        if (name != nullptr) lines.back().append(name);
    }
    if (opt.enable_info) {
        lines.push_back("詳細: ");
        if (info != nullptr) lines.back().append(info);
    }
    if (opt.enable_path) {
        lines.push_back("パス: ");
        if (path != nullptr) {
            lines.back().append(std::filesystem::relative(path, aviutl_dir).generic_string());
        }
    }
    if (opt.enable_hash) {
        lines.push_back("SHA256: ");
        if (path != nullptr) {
            try {
                lines.back().append(hasher.getFileHash(path));
            }
            catch (const std::runtime_error& e) {
                OutputDebugStringA(e.what());
            }
        }
    }

    dest << kIndent << kBullet2 << lines[0] << "\n";
    for (size_t i = 1; i < lines.size(); i++) {
        dest << kIndent << kBulletE << lines[i] << "\n";
    }
    if (opt.enable_count > 1)
        dest << "\n";
}
