#include <Windows.h>
#include <CommCtrl.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include <aviutl.hpp>

#include "Sha256Hasher.hpp"
#include "AviUtlProfiler.hpp"
#include "ExEditProfiler.hpp"

using namespace std::literals::string_literals;
using AviUtl::FilterPlugin;

constexpr int kIdPluginCopyButton = 1001;
constexpr int kIdPluginSaveButton = 1002;
constexpr int kIdScriptCopyButton = 1101;
constexpr int kIdScriptSaveButton = 1102;
constexpr int kIdList = 1900;

const char* kFilterName = "上限確認";
const char* kFilterInformation = "上限確認 v0.3.0 by karoterra";

const FilterPlugin* g_filter = nullptr;

HFONT g_font = NULL;
HWND g_list = NULL;
HWND g_plugin_name = NULL;
HWND g_plugin_info = NULL;
HWND g_plugin_path = NULL;
HWND g_plugin_hash = NULL;
HWND g_script_output_default = NULL;

AviUtlProfiler g_aviutl_profiler;
ExEditProfiler g_exedit_profiler;

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

void SetFont(HWND hwnd) {
    SendMessage(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), MAKELPARAM(TRUE, 0));
}

bool CreateFilterWindow(FilterPlugin* fp) {
    InitCommonControls();

    RECT rc;
    GetClientRect(fp->hwnd, &rc);
    auto width = rc.right - rc.left;
    auto height = rc.bottom - rc.top;

    DWORD style = NULL;

    int listHeight = static_cast<int>(height - 140);
    g_list = CreateWindowEx(
        0, WC_LISTVIEW, nullptr,
        WS_CHILD | WS_VISIBLE | LVS_REPORT,
        0, 0, width, listHeight,
        fp->hwnd, reinterpret_cast<HMENU>(kIdList), fp->dll_hinst, NULL
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
    SetListItem(0, "スクリプト名 ANM", g_exedit_profiler.GetAnmUsed(), ExEditProfiler::kAnmMax);
    SetListItem(1, "スクリプト名 OBJ", g_exedit_profiler.GetObjUsed(), ExEditProfiler::kObjMax);
    SetListItem(2, "スクリプト名 SCN", g_exedit_profiler.GetScnUsed(), ExEditProfiler::kScnMax);
    SetListItem(3, "スクリプト名 CAM", g_exedit_profiler.GetCamUsed(), ExEditProfiler::kCamMax);
    SetListItem(4, "スクリプト名 TRA", g_exedit_profiler.GetTraUsed(), ExEditProfiler::kTraMax);
    SetListItem(5, "図形名", g_exedit_profiler.GetFigureUsed(), ExEditProfiler::kFigureMax);
    SetListItem(6, "トランジション名", g_exedit_profiler.GetTransitionUsed(), ExEditProfiler::kTransitionMax);
    SetListItem(7, "EXA/EXO", g_exedit_profiler.GetExaExoUsed(), ExEditProfiler::kExaExoMax);
    SetListItem(8, "入力プラグイン", g_aviutl_profiler.GetInputNum(), AviUtlProfiler::kInputCountMax);
    SetListItem(9, "出力プラグイン", g_aviutl_profiler.GetOutputNum(), AviUtlProfiler::kOutputCountMax);
    SetListItem(10, "フィルタプラグイン", g_aviutl_profiler.GetFilterNum(), AviUtlProfiler::kFilterCountMax);
    SetListItem(11, "色変換プラグイン", g_aviutl_profiler.GetColorNum(), AviUtlProfiler::kColorCountMax);

    // プラグイングループ
    HWND hwnd = CreateWindowEx(
        0, "BUTTON", "プラグイン",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        0, listHeight, width / 2, height - listHeight,
        fp->hwnd, NULL, fp->dll_hinst, NULL
    );
    SetFont(hwnd);

    hwnd = CreateWindowEx(
        0, "BUTTON", "クリップボードにコピー",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, listHeight + 20, width / 2 - 20, 18,
        fp->hwnd, reinterpret_cast<HMENU>(kIdPluginCopyButton), fp->dll_hinst, NULL
    );
    SetFont(hwnd);
    hwnd = CreateWindowEx(
        0, "BUTTON", "ファイルに保存",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, listHeight + 40, width / 2 - 20, 18,
        fp->hwnd, reinterpret_cast<HMENU>(kIdPluginSaveButton), fp->dll_hinst, NULL
    );
    SetFont(hwnd);

    g_plugin_name = CreateWindowEx(
        0, "BUTTON", "プラグイン名",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        10, listHeight + 60, width / 2 - 20, 18,
        fp->hwnd, NULL, fp->dll_hinst, NULL
    );
    SetFont(g_plugin_name);
    SendMessage(g_plugin_name, BM_SETCHECK, BST_CHECKED, 0);
    g_plugin_info = CreateWindowEx(
        0, "BUTTON", "詳細情報",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        10, listHeight + 80, width / 2 - 20, 18,
        fp->hwnd, NULL, fp->dll_hinst, NULL
    );
    SetFont(g_plugin_info);
    SendMessage(g_plugin_info, BM_SETCHECK, BST_CHECKED, 0);
    g_plugin_path = CreateWindowEx(
        0, "BUTTON", "ファイル名",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        10, listHeight + 100, width / 2 - 20, 18,
        fp->hwnd, NULL, fp->dll_hinst, NULL
    );
    SetFont(g_plugin_path);
    SendMessage(g_plugin_path, BM_SETCHECK, BST_CHECKED, 0);
    g_plugin_hash = CreateWindowEx(
        0, "BUTTON", "SHA256ハッシュ",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        10, listHeight + 120, width / 2 - 20, 18,
        fp->hwnd, NULL, fp->dll_hinst, NULL
    );
    SetFont(g_plugin_hash);
    SendMessage(g_plugin_hash, BM_SETCHECK, BST_CHECKED, 0);

    // スクリプトグループ
    style = WS_CHILD | WS_VISIBLE | BS_GROUPBOX
        | (g_exedit_profiler.IsSupported() ? 0 : WS_DISABLED);
    hwnd = CreateWindowEx(
        0, "BUTTON", "スクリプト", style,
        width / 2, listHeight, width / 2, height - listHeight,
        fp->hwnd, NULL, fp->dll_hinst, NULL
    );
    SetFont(hwnd);

    style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON
        | (g_exedit_profiler.IsSupported() ? 0 : WS_DISABLED);
    hwnd = CreateWindowEx(
        0, "BUTTON", "クリップボードにコピー", style,
        width / 2 + 10, listHeight + 20, width / 2 - 20, 18,
        fp->hwnd, reinterpret_cast<HMENU>(kIdScriptCopyButton), fp->dll_hinst, NULL
    );
    SetFont(hwnd);
    hwnd = CreateWindowEx(
        0, "BUTTON", "ファイルに保存", style,
        width / 2 + 10, listHeight + 40, width / 2 - 20, 18,
        fp->hwnd, reinterpret_cast<HMENU>(kIdScriptSaveButton), fp->dll_hinst, NULL
    );
    SetFont(hwnd);

    style = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX
        | (g_exedit_profiler.IsSupported() ? 0 : WS_DISABLED);
    g_script_output_default = CreateWindowEx(
        0, "BUTTON", "標準スクリプトを出力する", style,
        width / 2 + 10, listHeight + 60, width / 2 - 20, 18,
        fp->hwnd, NULL, fp->dll_hinst, NULL
    );
    SetFont(g_script_output_default);
    SendMessage(g_script_output_default, BM_SETCHECK, BST_UNCHECKED, 0);

    return true;
}

bool CopyToClipboard(std::string s) {
    if (!OpenClipboard(g_filter->hwnd)) {
        return false;
    }
    EmptyClipboard();

    HGLOBAL hg = GlobalAlloc(GHND | GMEM_SHARE, s.length() + 1);
    LPSTR buf = static_cast<LPSTR>(GlobalLock(hg));
    lstrcpy(buf, s.c_str());
    GlobalUnlock(hg);

    SetClipboardData(CF_TEXT, hg);

    CloseClipboard();
    return true;
}

bool IsChecked(HWND hCheck) {
    return SendMessage(hCheck, BM_GETCHECK, 0, 0) != FALSE;
}

PluginsOption GetPluginsOption() {
    PluginsOption opt{
        .enable_name = IsChecked(g_plugin_name),
        .enable_info = IsChecked(g_plugin_info),
        .enable_path = IsChecked(g_plugin_path),
        .enable_hash = IsChecked(g_plugin_hash),
    };
    opt.Update();
    return opt;
}

ScriptsOption GetScriptsOption() {
    ScriptsOption opt{
        .output_default = IsChecked(g_script_output_default),
    };
    return opt;
}

BOOL func_init(FilterPlugin* fp) {
    g_aviutl_profiler.Init(fp);
    g_exedit_profiler.Init(fp);

    g_filter = fp;

    NONCLIENTMETRICS ncm{ .cbSize = sizeof(NONCLIENTMETRICS) };
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
    g_font = CreateFontIndirect(&ncm.lfCaptionFont);

    return TRUE;
}

BOOL func_exit(FilterPlugin* fp) {
    DeleteObject(g_font);
    return TRUE;
}

BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, FilterPlugin* fp) {
    switch (message) {
    case AviUtl::detail::FilterPluginWindowMessage::ChangeActive:
        CreateFilterWindow(fp);
        break;
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case kIdPluginCopyButton: {
            std::ostringstream os;
            auto opt = GetPluginsOption();
            g_aviutl_profiler.WritePluginsProfile(os, opt);
            CopyToClipboard(os.str());
            break;
        }
        case kIdPluginSaveButton: {
            char name[MAX_PATH];
            if (fp->exfunc->dlg_get_save_name(name, "テキスト (*.txt)\0*.txt", "plugins.txt") != FALSE) {
                try {
                    std::ofstream ofs(name);
                    auto opt = GetPluginsOption();
                    g_aviutl_profiler.WritePluginsProfile(ofs, opt);
                }
                catch (const std::exception& e) {
                    std::ostringstream os;
                    os << '"' << name << "\"への書き込みに失敗しました。\n" << e.what();
                    MessageBox(g_filter->hwnd, os.str().c_str(), kFilterName, MB_OK | MB_ICONERROR);
                }
            }
            break;
        }
        case kIdScriptCopyButton: {
            std::ostringstream os;
            auto opt = GetScriptsOption();
            g_exedit_profiler.WriteProfile(os, opt);
            CopyToClipboard(os.str());
            break;
        }
        case kIdScriptSaveButton: {
            char name[MAX_PATH];
            if (fp->exfunc->dlg_get_save_name(name, "テキスト (*.txt)\0*.txt", "scripts.txt") != FALSE) {
                try {
                    std::ofstream ofs(name);
                    auto opt = GetScriptsOption();
                    g_exedit_profiler.WriteProfile(ofs, opt);
                }
                catch (const std::exception& e) {
                    std::ostringstream os;
                    os << '"' << name << "\"への書き込みに失敗しました。\n" << e.what();
                    MessageBox(g_filter->hwnd, os.str().c_str(), kFilterName, MB_OK | MB_ICONERROR);
                }
            }
            break;
        }
        }
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
    .y = 390,
    .name = const_cast<char*>(kFilterName),
    .func_init = func_init,
    .func_exit = func_exit,
    .func_WndProc = func_WndProc,
    .information = const_cast<char*>(kFilterInformation),
};

extern "C" __declspec(dllexport) AviUtl::FilterPluginDLL * GetFilterTable(void) {
    return &filter_src;
}
