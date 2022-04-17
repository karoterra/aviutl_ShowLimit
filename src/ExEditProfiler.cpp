#include "ExEditProfiler.hpp"

#include <algorithm>
#include <filesystem>
#include <vector>

#include "Sha256Hasher.hpp"
#include "NamesBuffer.hpp"

namespace fs = std::filesystem;

constexpr std::string_view kBullet1{ "■" };
constexpr std::string_view kBullet2{ "●" };
constexpr std::string_view kBulletE{ "　" };
constexpr std::string_view kIndent{ "　" };

constexpr std::string_view kLuaLibName{ "lua51.dll" };

const std::vector<std::string> defaultAnm{
    "震える",
    "振り子",
    "弾む",
    "座標の拡大縮小(個別オブジェクト)",
    "画面外から登場",
    "ランダム方向から登場",
    "拡大縮小して登場",
    "ランダム間隔で落ちながら登場",
    "弾んで登場",
    "広がって登場",
    "起き上がって登場",
    "何処からともなく登場",
    "反復移動",
    "座標の回転(個別オブジェクト)",
    "立方体(カメラ制御)",
    "球体(カメラ制御)",
    "砕け散る",
    "点滅",
    "点滅して登場",
    "簡易変形",
    "簡易変形(カメラ制御)",
    "リール回転",
    "万華鏡",
    "円形配置",
    "ランダム配置",
};
const std::vector<std::string> defaultObj{
    "集中線",
    "走査線",
    "カウンター",
    "レンズフレア",
    "雲",
    "星",
    "雪",
    "雨",
    "ランダム小物配置(カメラ制御)",
    "ライン(移動軌跡)",
    "扇型",
    "多角形",
    "周辺ボケ光量",
    "フレア",
    "水面",
};
const std::vector<std::string> defaultScn{
    // 組込みシーンチェンジ
    "クロスフェード",
    "ワイプ(円)",
    "ワイプ(四角)",
    "ワイプ(時計)",
    "スライス",
    "スワップ",
    "スライド",
    "縮小回転",
    "押し出し(横)",
    "押し出し(縦)",
    "回転(横)",
    "回転(縦)",
    "キューブ回転(横)",
    "キューブ回転(縦)",
    "フェードアウトイン",
    "放射ブラー",
    "ぼかし",
    "ワイプ(横)",
    "ワイプ(縦)",
    "ロール(横)",
    "ロール(縦)",
    "ランダムライン",
    // 標準スクリプト
    "発光",
    "レンズブラー",
    "ドア",
    "起き上がる",
    "リール回転",
    "図形ワイプ",
    "図形で隠す",
    "図形で隠す(放射)",
    "砕け散る",
    "ページめくり",
};
const std::vector<std::string> defaultCam{
    "手ぶれ",
    "目標中心回転",
    "目標サイズ固定視野角",
};
const std::vector<std::string> defaultTra{
    "補間移動",
    "回転",
};
const std::vector<std::string> defaultFigure{
    "背景",
    "円",
    "四角形",
    "三角形",
    "五角形",
    "六角形",
    "星型",
    "(ファイルから選択)"
};
const std::vector<std::string> defaultTransition{
    "ワイプ(円)",
    "ワイプ(四角)",
    "ワイプ(時計)",
    "ワイプ(横)",
    "ワイプ(縦)",
};

void WriteNamesBuffer(
    std::ostream& dest, const NamesBuffer& buffer,
    const std::vector<std::string>& defaults, const ScriptsOption& opt)
{
    std::string current_dir = "";
    for (auto [name, dir] : buffer) {
        if (!opt.output_default && std::find(defaults.begin(), defaults.end(), name) != defaults.end()) {
            continue;
        }

        if (dir != current_dir) {
            current_dir = dir;
            dest << kIndent << kBullet2 << dir << "\n";
        }
        dest << kIndent << kIndent << name << "\n";
    }
}

void ExEditProfiler::WriteProfile(std::ostream& dest, const ScriptsOption& opt)
{
    if (!IsSupported()) return;

    WriteExEditDetail(dest);
    dest << "\n";

    dest << kBullet1 << "アニメーション効果\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kAnmOffset), kAnmMax), defaultAnm, opt);
    dest << kBullet1 << "カスタムオブジェクト\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kObjOffset), kObjMax), defaultObj, opt);
    dest << kBullet1 << "シーンチェンジ\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kScnOffset), kScnMax), defaultScn, opt);
    dest << kBullet1 << "カメラ効果\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kCamOffset), kCamMax), defaultCam, opt);
    dest << kBullet1 << "トラックバー変化方法\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kTraOffset), kTraMax), defaultTra, opt);
    dest << kBullet1 << "図形\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kFigureOffset), kFigureMax), defaultFigure, opt);
    dest << kBullet1 << "トランジション\n";
    WriteNamesBuffer(dest, NamesBuffer(GetNamesBuffer(kTransitionOffset), kTransitionMax), defaultTransition, opt);
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
