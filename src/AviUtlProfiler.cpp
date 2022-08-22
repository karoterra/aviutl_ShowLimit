#include "AviUtlProfiler.hpp"

#include <filesystem>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <TlHelp32.h>

#include "Sha256Hasher.hpp"

namespace fs = std::filesystem;
using namespace std::literals::string_literals;

constexpr std::string_view kBullet1{ "■" };
constexpr std::string_view kBullet2{ "●" };
constexpr std::string_view kBulletE{ "　" };
constexpr std::string_view kIndent{ "　" };

template<typename T>
bool HasFlag(T x, T flag)
{
    return (static_cast<uint32_t>(x) & static_cast<uint32_t>(flag)) != 0;
}

void WritePluginData(
    std::ostream& dest, Sha256Hasher& hasher, const PluginsOption& opt,
    const fs::path& aviutl_dir,
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
            lines.back().append(fs::relative(path, aviutl_dir).generic_string());
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

void AviUtlProfiler::WritePluginsProfile(std::ostream& dest, const PluginsOption& opt)
{
    Sha256Hasher hasher;

    dest << kBullet1 << "AviUtl\n";
    fs::path aviutl_path = GetAviUtlPath();
    fs::path aviutl_dir = aviutl_path.parent_path();
    AviUtl::SysInfo si;
    exfunc_->get_sys_info(nullptr, &si);
    dest << kIndent << "バージョン: " << si.info << "\n";
    dest << kIndent << "ビルド: " << si.build << "\n";
    dest << kIndent << "パス: " << aviutl_path.generic_string() << "\n";
    dest << kIndent << "SHA256: " << hasher.getFileHash(aviutl_path) << "\n\n";

    if (opt.enable_count == 0)
        return;

    if (IsSupported()) {
        dest << kBullet1 << "入力プラグイン\n";
        auto inputs = GetPtr<AviUtl::InputPlugin>(kInputArrayOffset);
        auto input_num = GetInputNum();
        for (size_t i = 0; i < input_num; i++) {
            if (HasFlag(inputs[i].flag, AviUtl::detail::InputPluginFlag::Builtin))
                continue;

            WritePluginData(dest, hasher, opt, aviutl_dir,
                inputs[i].name2, inputs[i].information2, inputs[i].path);
        }
        if (opt.enable_count == 1) dest << "\n";

        dest << kBullet1 << "出力プラグイン\n";
        auto outputs = GetPtr<AviUtl::OutputPlugin>(kOutputArrayOffset);
        auto output_num = GetOutputNum();
        for (size_t i = 0; i < output_num; i++) {
            if (HasFlag(outputs[i].flag, AviUtl::detail::OutputPluginFlag::Builtin))
                continue;

            WritePluginData(dest, hasher, opt, aviutl_dir,
                outputs[i].name2, outputs[i].information2, outputs[i].path);
        }
        if (opt.enable_count == 1) dest << "\n";
    }

    dest << kBullet1 << "フィルタプラグイン\n";
    auto filter_num = GetFilterNum();
    for (size_t i = 0; i < filter_num; i++) {
        auto fp = exfunc_->get_filterp(i);
        if (fp->dll_hinst == NULL)
            continue;

        char buf[MAX_PATH];
        GetModuleFileName(fp->dll_hinst, buf, MAX_PATH);
        WritePluginData(dest, hasher, opt, aviutl_dir,
            fp->name, fp->information, buf);
    }
    if (opt.enable_count == 1)
        dest << "\n";

    if (IsSupported()) {
        dest << kBullet1 << "色変換プラグイン\n";
        auto colors = GetPtr<AviUtl::ColorPlugin>(kColorArrayOffset);
        auto color_num = GetColorNum();
        for (size_t i = 0; i < color_num; i++) {
            if (HasFlag(colors[i].flag, AviUtl::detail::ColorPluginFlag::Builtin))
                continue;

            WritePluginData(dest, hasher, opt, aviutl_dir,
                colors[i].name, colors[i].information, colors[i].path);
        }
        if (opt.enable_count == 1)
            dest << "\n";

        dest << kBullet1 << "言語拡張リソースプラグイン\n";
        auto langs = GetPtr<LanguagePlugin>(kLanguageArrayOffset);
        auto lang_num = GetLanguageNum();
        for (size_t i = 1; i < lang_num; i++) {
            WritePluginData(dest, hasher, opt, aviutl_dir,
                langs[i].name, langs[i].information, langs[i].path);
        }
        if (opt.enable_count == 1)
            dest << "\n";
    }

    WriteOtherPluginsProfile(dest, opt);
}

void AviUtlProfiler::WriteOtherPluginsProfile(std::ostream& dest, const PluginsOption& opt)
{
    dest << kBullet1 << "その他のプラグイン\n";

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, NULL);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    MODULEENTRY32 me32{ .dwSize = sizeof(MODULEENTRY32) };
    if (Module32First(snapshot, &me32) == FALSE) {
        CloseHandle(snapshot);
        return;
    }

    Sha256Hasher hasher;
    fs::path aviutl_path = GetAviUtlPath();
    fs::path aviutl_dir = aviutl_path.parent_path();
    do {
        fs::path plugin_path = me32.szExePath;
        if (plugin_path.extension() != ".aul" || IsLanguagePlugin(plugin_path.string().c_str())) {
            continue;
        }

        WritePluginData(dest, hasher, opt, aviutl_dir,
            plugin_path.filename().string().c_str(),
            plugin_path.filename().string().c_str(),
            plugin_path.string().c_str());
    } while (Module32Next(snapshot, &me32) != FALSE);

    CloseHandle(snapshot);
}
