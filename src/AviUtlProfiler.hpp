#pragma once

#include <ostream>
#include <string>

#include <aviutl.hpp>

#include "PluginsOption.hpp"

class AviUtlProfiler
{
public:
    static constexpr int32_t kBuildNum110 = 11003;

    static constexpr size_t kInputArrayOffset = 0xb8cb0;
    static constexpr size_t kInputCountOffset = 0xb5ac4;
    static constexpr size_t kInputCountMax = 32;

    static constexpr size_t kOutputArrayOffset = 0x24fbf0;
    static constexpr size_t kOutputCountOffset = 0x24b93c;
    static constexpr size_t kOutputCountMax = 32;

    static constexpr size_t kFilterCountMax = 96;

    static constexpr size_t kColorArrayOffset = 0xb5ac8;
    static constexpr size_t kColorCountOffset = 0xb5ac0;
    static constexpr size_t kColorCountMax = 32;

    void Init(AviUtl::FilterPlugin* filter) {
        exfunc_ = filter->exfunc;
        hinst_ = filter->hinst_parent;
        AviUtl::SysInfo si;
        exfunc_->get_sys_info(nullptr, &si);
        build_num_ = si.build;
    }

    bool IsSupported() const {
        return build_num_ == kBuildNum110;
    }

    size_t GetInputNum() const {
        if (!IsSupported()) return 0;
        return ReadUInt32(kInputCountOffset);
    }

    size_t GetOutputNum() const {
        if (!IsSupported()) return 0;
        return ReadUInt32(kOutputCountOffset);
    }

    size_t GetFilterNum() const {
        AviUtl::SysInfo si;
        exfunc_->get_sys_info(nullptr, &si);
        return si.filter_n;
    }

    size_t GetColorNum() const {
        if (!IsSupported()) return 0;
        return ReadUInt32(kColorCountOffset);
    }

    std::string GetAviUtlPath() const {
        char path[MAX_PATH];
        GetModuleFileName(hinst_, path, MAX_PATH);
        return path;
    }

    void WritePluginsProfile(std::ostream& dest, const PluginsOption& opt);

private:
    int32_t build_num_;
    AviUtl::ExFunc* exfunc_;
    HINSTANCE hinst_;

    uint32_t ReadUInt32(size_t offset) const {
        return *reinterpret_cast<uint32_t*>(reinterpret_cast<size_t>(hinst_) + offset);
    }
};
