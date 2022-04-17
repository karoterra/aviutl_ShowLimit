#pragma once

struct PluginsOption {
    bool enable_name;
    bool enable_info;
    bool enable_path;
    bool enable_hash;

    int enable_count;

    void Update() {
        enable_count = enable_name + enable_info + enable_path + enable_hash;
    }
};
