#pragma once

#include <ostream>
#include <filesystem>
#include <string_view>

#include <aviutl.hpp>
#include <exedit.hpp>

#include "ScriptsOption.hpp"
#include "PluginsOption.hpp"

class ExEditProfiler
{
public:
    static constexpr size_t kAnmOffset = 0xc1f08;
    static constexpr size_t kAnmMax = 0x8000;

    static constexpr size_t kObjOffset = 0xce090;
    static constexpr size_t kObjMax = 0x4000;

    static constexpr size_t kScnOffset = 0xaef38;
    static constexpr size_t kScnMax = 0x8000;

    static constexpr size_t kCamOffset = 0xd20d0;
    static constexpr size_t kCamMax = 0x2000;

    static constexpr size_t kTraOffset = 0xca010;
    static constexpr size_t kTraMax = 0x4000;

    static constexpr size_t kFigureOffset = 0xa9a90;
    static constexpr size_t kFigureMax = 0x4000;

    static constexpr size_t kTransitionOffset = 0xba6d0;
    static constexpr size_t kTransitionMax = 0x4000;

    static constexpr size_t kExaExoOffset = 0x135c70;
    static constexpr size_t kExaExoMax = 0x10000;

    static constexpr size_t kExtensionOffset = 0x14cb58;
    static constexpr size_t kExtensionMax = 0x800;

    static constexpr size_t kExEditFilterArrayOffset = 0x187c98;
    static constexpr size_t kExEditFilterCountOffset = 0x146248;
    static constexpr size_t kExEditFilterMax = 512;

    static constexpr size_t kSusiePluginOffset = 0x2321f0;
    static constexpr size_t kSusiePluginMax = 32;

    static constexpr std::string_view kExEditName{ "拡張編集" };
    static constexpr std::string_view kExEdit92{ "拡張編集(exedit) version 0.92 by ＫＥＮくん" };

    void Init(AviUtl::FilterPlugin* filter) {
        exedit_ = nullptr;
        AviUtl::SysInfo si;
        filter->exfunc->get_sys_info(nullptr, &si);
        for (int i = 0; i < si.filter_n; i++) {
            auto fp = filter->exfunc->get_filterp(i);
            if (kExEditName == fp->name && kExEdit92 == fp->information) {
                exedit_ = fp;
                break;
            }
        }
    }

    bool IsSupported() const {
        return exedit_ != nullptr;
    }

    size_t GetAnmUsed() const {
        return GetNamesBufferUsed(kAnmOffset, kAnmMax);
    }

    size_t GetObjUsed() const {
        return GetNamesBufferUsed(kObjOffset, kObjMax);
    }

    size_t GetScnUsed() const {
        return GetNamesBufferUsed(kScnOffset, kScnMax);
    }

    size_t GetCamUsed() const {
        return GetNamesBufferUsed(kCamOffset, kCamMax);
    }

    size_t GetTraUsed() const {
        return GetNamesBufferUsed(kTraOffset, kTraMax);
    }

    size_t GetFigureUsed() const {
        return GetNamesBufferUsed(kFigureOffset, kFigureMax);
    }

    size_t GetTransitionUsed() const {
        return GetNamesBufferUsed(kTransitionOffset, kTransitionMax);
    }

    size_t GetExaExoUsed() const {
        return GetNamesBufferUsed(kExaExoOffset, kExaExoMax);
    }

    size_t GetExtensionUsed() const {
        return GetNamesBufferUsed(kExtensionOffset, kExtensionMax);
    }

    size_t GetExEditFilterNum() const {
        if (!IsSupported()) return 0;
        return ReadUInt32(kExEditFilterCountOffset);
    }

    size_t GetSusiePluginNum() const {
        if (!IsSupported()) return 0;
        const auto spi = GetPtr<ExEdit::structSPI>(kSusiePluginOffset);
        size_t count = 0;
        for (size_t i = 0; i < kSusiePluginMax; i++) {
            if (spi[i].hmodule != NULL) {
                count++;
            }
        }
        return count;
    }

    void WriteProfile(std::ostream& dest, const ScriptsOption& opt);

    void WriteExEditFilterProfile(std::ostream& dest, const std::filesystem::path& aviutl_dir, const PluginsOption& opt);

    void WriteSusiePluginProfile(std::ostream& dest, const std::filesystem::path& aviutl_dir, const PluginsOption& opt);

private:
    AviUtl::FilterPlugin* exedit_;

    char* GetNamesBuffer(size_t offset) const {
        if (!IsSupported()) return nullptr;
        return reinterpret_cast<char*>(reinterpret_cast<size_t>(exedit_->dll_hinst) + offset);
    }

    static size_t GetNamesBufferLength(const char* buf, size_t size) {
        size_t i;
        for (i = 0; i < size; i++) {
            if (buf[i] != '\0') continue;
            if (buf[i + 1] == '\0') break;
        }
        return i;
    }

    size_t GetNamesBufferUsed(size_t offset, size_t size) const {
        if (!IsSupported()) return 0;
        auto buf = GetNamesBuffer(offset);
        return GetNamesBufferLength(buf, size);
    }

    template<typename T>
    T* GetPtr(size_t offset) const {
        if (!IsSupported()) return nullptr;
        return reinterpret_cast<T*>(reinterpret_cast<size_t>(exedit_->dll_hinst) + offset);
    }

    uint32_t ReadUInt32(size_t offset) const {
        if (!IsSupported()) return 0;
        return *GetPtr<uint32_t>(offset);
    }

    void WriteExEditDetail(std::ostream& dest);
};

