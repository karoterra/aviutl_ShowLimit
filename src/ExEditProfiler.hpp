#pragma once

#include <ostream>
#include <string_view>

#include <aviutl.hpp>

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

    void WriteProfile(std::ostream& dest);

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

    void WriteExEditDetail(std::ostream& dest);
};

