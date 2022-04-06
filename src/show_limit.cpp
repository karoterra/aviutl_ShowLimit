#include <aviutl_exedit_sdk/aviutl.hpp>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include <sstream>
#include <iomanip>

using AviUtl::FilterPlugin;
using AviUtl::InputPlugin;
using AviUtl::OutputPlugin;
using AviUtl::ColorPlugin;
using AviUtl::SysInfo;

const char* FILTER_NAME = "上限確認";
const char* FILTER_INFORMATION = "上限確認 v0.1.0 by karoterra";

const char* EXEDIT_NAME = "拡張編集";
const char* EXEDIT_92 = "拡張編集(exedit) version 0.92 by ＫＥＮくん";
const size_t ANM_OFFSET = 0xc1f08;
const size_t ANM_MAX = 0x8000;
const size_t OBJ_OFFSET = 0xce090;
const size_t OBJ_MAX = 0x4000;
const size_t SCN_OFFSET = 0xaef83;
const size_t SCN_MAX = 0x8000;
const size_t CAM_OFFSET = 0xd20d0;
const size_t CAM_MAX = 0x2000;
const size_t TRA_OFFSET = 0xca010;
const size_t TRA_MAX = 0x4000;
const size_t FIGURE_OFFSET = 0xa9a90;
const size_t FIGURE_MAX = 0x4000;;
const size_t TRANSITION_OFFSET = 0xba6d0;
const size_t TRANSITION_MAX = 0x4000;

const size_t INPUT_COUNT_OFFSET = 0xb5ac4;
const size_t OUTPUT_COUNT_OFFSET = 0x24b93c;
const size_t COLOR_COUNT_OFFSET = 0xb5ac0;
const size_t INPUT_COUNT_MAX = 32;
const size_t OUTPUT_COUNT_MAX = 32;
const size_t FILTER_COUNT_MAX = 96;
const size_t COLOR_COUNT_MAX = 32;

const FilterPlugin* g_exedit = nullptr;
size_t g_anm_used = 0;
size_t g_obj_used = 0;
size_t g_scn_used = 0;
size_t g_cam_used = 0;
size_t g_tra_used = 0;
size_t g_figure_used = 0;
size_t g_transition_used = 0;

size_t g_input_used = 0;
size_t g_output_used = 0;
size_t g_filter_used = 0;
size_t g_color_used = 0;

HWND g_list = NULL;

FilterPlugin* GetExedit(FilterPlugin* fp) {
    SysInfo si;
    fp->exfunc->get_sys_info(nullptr, &si);
    for (int i = 0; i < si.filter_n; i++) {
        FilterPlugin* fp1 = fp->exfunc->get_filterp(i);
        if (strcmp(fp1->name, EXEDIT_NAME) == 0 && strcmp(fp1->information, EXEDIT_92) == 0) {
            return fp1;
        }
    }
    return nullptr;
}

bool IsValidAviUtl(FilterPlugin* fp) {
    SysInfo si;
    fp->exfunc->get_sys_info(nullptr, &si);
    return strcmp("1.10", si.info) == 0 && si.build == 11003;
}

size_t NamesBufLen(const char* buf, size_t size) {
    size_t i;
    for (i = 0; i < size; i++) {
        if (buf[i] != '\0') continue;
        if (buf[i + 1] == '\0') break;
    }
    return i;
}

size_t GetNamesBufUsed(size_t base, size_t offset, size_t size) {
    auto buf = reinterpret_cast<const char*>(base + offset);
    return NamesBufLen(buf, size);
}

uint32_t ReadUInt32(size_t base, size_t offset) {
    auto p = reinterpret_cast<uint32_t*>(base + offset);
    return *p;
}

void CheckLimit() {
    if (g_exedit == nullptr) return;

    auto base = reinterpret_cast<size_t>(g_exedit->dll_hinst);
    g_anm_used = GetNamesBufUsed(base, ANM_OFFSET, ANM_MAX);
    g_obj_used = GetNamesBufUsed(base, OBJ_OFFSET, OBJ_MAX);
    g_scn_used = GetNamesBufUsed(base, SCN_OFFSET, SCN_MAX);
    g_cam_used = GetNamesBufUsed(base, CAM_OFFSET, CAM_MAX);
    g_tra_used = GetNamesBufUsed(base, TRA_OFFSET, TRA_MAX);
    g_figure_used = GetNamesBufUsed(base, FIGURE_OFFSET, FIGURE_MAX);
    g_transition_used = GetNamesBufUsed(base, TRANSITION_OFFSET, TRANSITION_MAX);

    SysInfo si;
    g_exedit->exfunc->get_sys_info(nullptr, &si);
    g_filter_used = si.filter_n;

    base = reinterpret_cast<size_t>(g_exedit->hinst_parent);
    g_input_used = ReadUInt32(base, INPUT_COUNT_OFFSET);
    g_output_used = ReadUInt32(base, OUTPUT_COUNT_OFFSET);
    g_color_used = ReadUInt32(base, COLOR_COUNT_OFFSET);
}

void SetListItem(int row, const char* name, size_t used, size_t limit) {
    LVITEM item{
        .mask = LVIF_TEXT,
        .iItem = row,
        .iSubItem = 0,
        .pszText = const_cast<char*>(name),
    };
    ListView_InsertItem(g_list, &item);

    std::string text = std::to_string(used);
    item.iSubItem = 1;
    item.pszText = const_cast<char*>(text.c_str());
    ListView_SetItem(g_list, &item);

    text = std::to_string(limit - used);
    item.iSubItem = 2;
    item.pszText = const_cast<char*>(text.c_str());
    ListView_SetItem(g_list, &item);

    text = std::to_string(limit);
    item.iSubItem = 3;
    item.pszText = const_cast<char*>(text.c_str());
    ListView_SetItem(g_list, &item);

    std::ostringstream oss;
    oss << std::setprecision(1) << std::fixed << ((double)used / limit * 100);
    oss << " %";
    text = oss.str();
    item.iSubItem = 4;
    item.pszText = const_cast<char*>(text.c_str());
    ListView_SetItem(g_list, &item);
}

bool CreateFilterWindow(FilterPlugin* fp) {
    InitCommonControls();

    RECT rc;
    GetClientRect(fp->hwnd, &rc);

    g_list = CreateWindowEx(
        0, WC_LISTVIEW, nullptr,
        WS_CHILD | WS_VISIBLE | LVS_REPORT,
        0, 0,
        rc.right - rc.left, rc.bottom - rc.top,
        fp->hwnd, NULL, fp->dll_hinst, NULL
    );
    if (g_list == NULL) return false;

    // columns
    LVCOLUMN col{ .mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH };
    col.fmt = LVCFMT_LEFT;
    col.iSubItem = 0;
    col.cx = 100;
    col.pszText = const_cast<char*>("名前");
    if (ListView_InsertColumn(g_list, 0, &col) < 0) {
        return false;
    }
    col.fmt = LVCFMT_RIGHT;
    col.iSubItem = 1;
    col.cx = 60;
    col.pszText = const_cast<char*>("使用量");
    if (ListView_InsertColumn(g_list, 1, &col) < 0) {
        return false;
    }
    col.iSubItem = 2;
    col.pszText = const_cast<char*>("空き");
    if (ListView_InsertColumn(g_list, 2, &col) < 0) {
        return false;
    }
    col.iSubItem = 3;
    col.pszText = const_cast<char*>("上限");
    if (ListView_InsertColumn(g_list, 3, &col) < 0) {
        return false;
    }
    col.iSubItem = 4;
    col.pszText = const_cast<char*>("使用率");
    if (ListView_InsertColumn(g_list, 4, &col) < 0) {
        return false;
    }

    // items
    SetListItem(0, "スクリプト名 ANM", g_anm_used, ANM_MAX);
    SetListItem(1, "スクリプト名 OBJ", g_obj_used, OBJ_MAX);
    SetListItem(2, "スクリプト名 SCN", g_scn_used, SCN_MAX);
    SetListItem(3, "スクリプト名 CAM", g_cam_used, CAM_MAX);
    SetListItem(4, "スクリプト名 TRA", g_tra_used, TRA_MAX);
    SetListItem(5, "図形名", g_figure_used, FIGURE_MAX);
    SetListItem(6, "トランジション名", g_transition_used, TRANSITION_MAX);
    SetListItem(7, "入力プラグイン", g_input_used, INPUT_COUNT_MAX);
    SetListItem(8, "出力プラグイン", g_output_used, OUTPUT_COUNT_MAX);
    SetListItem(9, "フィルタプラグイン", g_filter_used, FILTER_COUNT_MAX);
    SetListItem(10, "色変換プラグイン", g_color_used, COLOR_COUNT_MAX);

    return true;
}

BOOL func_init(FilterPlugin* fp) {
    if (!IsValidAviUtl(fp)) {
        MessageBox(
            fp->hwnd,
            "このAviUtlには対応していません",
            FILTER_NAME,
            MB_OK | MB_ICONERROR
        );
        return FALSE;
    }

    g_exedit = GetExedit(fp);
    if (g_exedit == nullptr) {
        MessageBox(
            fp->hwnd,
            "対応する拡張編集が見つかりません。",
            FILTER_NAME,
            MB_OK | MB_ICONERROR
        );
        return FALSE;
    }
    return TRUE;
}

BOOL func_exit(FilterPlugin* fp) {
    return TRUE;
}

BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, FilterPlugin* fp) {
    switch (message) {
    case AviUtl::detail::FilterPluginWindowMessage::ChangeActive:
        CheckLimit();
        CreateFilterWindow(fp);
        break;
    }
    return FALSE;
}

using AviUtl::detail::FilterPluginFlag;

AviUtl::FilterPluginDLL filter_src{
    .flag = FilterPluginFlag::AlwaysActive | FilterPluginFlag::PriorityLowest
        | FilterPluginFlag::ExInformation
        | FilterPluginFlag::DispFilter | FilterPluginFlag::WindowSize,
    .x = 400,
    .y = 250,
    .name = const_cast<char*>(FILTER_NAME),
    .func_init = func_init,
    .func_exit = func_exit,
    .func_WndProc = func_WndProc,
    .information = const_cast<char*>(FILTER_INFORMATION),
};

extern "C" __declspec(dllexport) AviUtl::FilterPluginDLL * GetFilterTable(void) {
    return &filter_src;
}
