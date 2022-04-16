#include "ExEditProfiler.hpp"

#include <filesystem>

#include "Sha256Hasher.hpp"
#include "NamesBuffer.hpp"

namespace fs = std::filesystem;

constexpr std::string_view kBullet1{ "■" };
constexpr std::string_view kBullet2{ "●" };
constexpr std::string_view kBulletE{ "　" };
constexpr std::string_view kIndent{ "　" };

constexpr std::string_view kLuaLibName{ "lua51.dll" };

void WriteNamesBuffer(std::ostream& dest, const NamesBuffer& buffer) {
    std::string current_dir = "";
    for (auto [name, dir] : buffer) {
        if (dir != current_dir) {
            current_dir = dir;
            dest << kIndent << kBullet2 << dir << "\n";
        }
        dest << kIndent << kIndent << name << "\n";
    }
}

void ExEditProfiler::WriteProfile(std::ostream& dest)
{
    if (!IsSupported()) return;

    WriteExEditDetail(dest);
    dest << "\n";

    dest << kBullet1 << "アニメーション効果\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kAnmOffset), kAnmMax));
    dest << kBullet1 << "カスタムオブジェクト\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kObjOffset), kObjMax));
    dest << kBullet1 << "シーンチェンジ\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kScnOffset), kScnMax));
    dest << kBullet1 << "カメラ効果\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kCamOffset), kCamMax));
    dest << kBullet1 << "トラックバー変化方法\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kTraOffset), kTraMax));
    dest << kBullet1 << "図形\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kFigureOffset), kFigureMax));
    dest << kBullet1 << "トランジション\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kTransitionOffset), kTransitionMax));
}

void ExEditProfiler::WriteExEditDetail(std::ostream& dest)
{
    dest << kBullet1 << exedit_->name << "\n";
    dest << kIndent << "詳細: " << exedit_->information << "\n";

    char buf[MAX_PATH];
    GetModuleFileName(exedit_->dll_hinst, buf, MAX_PATH);
    fs::path exedit_path(buf);
    dest << kIndent << "パス: " << exedit_path.string() << "\n";

    Sha256Hasher hasher;
    dest << kIndent << "SHA256: " << hasher.getFileHash(exedit_path.string().c_str()) << "\n";

    dest << kIndent << kLuaLibName << "\n";
    fs::path lua_path = exedit_path.parent_path().append(kLuaLibName);
    dest << kIndent << kIndent << "SHA256: " << hasher.getFileHash(lua_path.string().c_str()) << "\n";
}
