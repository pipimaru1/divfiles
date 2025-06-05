#include <iostream>
#include <filesystem>
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <cmath>      // std::ceilのため
#include <cstdlib>    // std::atoi, std::atofのため

namespace fs = std::filesystem;

// ファイル名（拡張子なし）を取得するヘルパー関数
std::string GetStem(const fs::path& path)
{
    return path.stem().string();
}

int makedir(fs::path dirPath);

// ランダムにファイルの一部を別のディレクトリにコピーする関数
// 新たに reductionFactor パラメータを追加（例: 0.1なら全体の10%を使用）
void CopyRandomFiles(
    const fs::path& sourceImages,
    const fs::path& sourceLabels,
    const fs::path& destTrainImages,
    const fs::path& destTrainLabels,
    const fs::path& destValidImages,
    const fs::path& destValidLabels,
    float percentage,
    float reductionFactor
)
{
    std::vector<fs::path> jpgFiles;

    // *.JPG, *.jpg ファイルをリストアップ
    for (const auto& entry : fs::directory_iterator(sourceImages)) {
        if (entry.is_regular_file() &&
            (entry.path().extension() == ".JPG" || entry.path().extension() == ".jpg"))
        {
            jpgFiles.push_back(entry.path());
        }
    }

    // ファイルリストをシャッフル
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(jpgFiles.begin(), jpgFiles.end(), g);

    // reductionFactor によりデータ数を減らす（例：全体の10%を使用）
    size_t totalFiles = jpgFiles.size();
    size_t totalFilesToUse = static_cast<size_t>(std::ceil(totalFiles * reductionFactor));
    jpgFiles.resize(totalFilesToUse);

    // トレーニング用ファイル数を計算
    size_t trainFilesCount = static_cast<size_t>(std::ceil(totalFilesToUse * (percentage / 100.0)));

    // 選択されたファイルをトレーニング用ディレクトリにコピー
    for (size_t i = 0; i < trainFilesCount; ++i) {
        fs::path destImagePath = destTrainImages / jpgFiles[i].filename();
        fs::copy(jpgFiles[i], destImagePath, fs::copy_options::overwrite_existing);

        // 同名のラベルファイル(.txt)もコピー
        fs::path sourceLabelPath = sourceLabels / jpgFiles[i].stem().concat(".txt");
        fs::path destLabelPath = destTrainLabels / sourceLabelPath.filename();
        if (fs::exists(sourceLabelPath)) {
            fs::copy(sourceLabelPath, destLabelPath, fs::copy_options::overwrite_existing);
        }
        // コピー済みのファイルはリスト内で空にする
        jpgFiles[i].clear();
    }

    // コピーされなかったファイルを検証用ディレクトリにコピー
    for (const auto& path : jpgFiles) {
        if (path.empty()) continue;
        fs::path destImagePath = destValidImages / path.filename();
        fs::copy(path, destImagePath, fs::copy_options::overwrite_existing);

        fs::path sourceLabelPath = sourceLabels / path.stem().concat(".txt");
        fs::path destLabelPath = destValidLabels / sourceLabelPath.filename();
        if (fs::exists(sourceLabelPath)) {
            fs::copy(sourceLabelPath, destLabelPath, fs::copy_options::overwrite_existing);
        }
    }
}

int main(int argc, char* argv[])
{
    std::string path_source = "";
    std::string path_dest = "";
    float percentage = 50.0f;      // トレーニング用にコピーする割合(%)
    float reductionFactor = 1.0f;  // 使用する全データの割合（1.0なら全て、0.1なら10%）

    // 引数が不足している場合、usageを表示
    if (argc < 4) {
        std::cout << argv[0] << std::endl;
        std::cout << "Copy files for yolo" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << argv[0] << " [source_path] [dest_path] [divide_value:1-99] [reduction_factor:0.001-1.0 (optional)]" << std::endl;
        std::cout << "[source_path]┳images━jpgファイル" << std::endl;
        std::cout << "             ┗labels━txtファイル" << std::endl;
        std::cout << "[dest_path]┳train┳images━jpgファイル" << std::endl;
        std::cout << "           ┃     ┗labels━txtファイル" << std::endl;
        std::cout << "           ┗valid┳images━jpgファイル" << std::endl;
        std::cout << "                  ┗labels━txtファイル" << std::endl;
        return 0;
    }
    else {
        path_source = argv[1];
        path_dest = argv[2];

        int divideValue = std::atoi(argv[3]); // 文字列から数値へ変換
        if (divideValue < 1 || divideValue > 99) {
            std::cerr << "divide_value は 1～99 の範囲で指定してください。" << std::endl;
            return 1;
        }
        percentage = static_cast<float>(divideValue);

        // reduction_factor が指定されていれば取得、指定がなければデフォルトは1.0
        if (argc >= 5) {
            reductionFactor = std::atof(argv[4]);
            if (reductionFactor < 0.001f || reductionFactor > 1.0f) {
                std::cerr << "reduction_factor は 0.001～1.0 の範囲で指定してください。" << std::endl;
                return 1;
            }
        }
    }

    fs::path sourceImages = path_source + "\\images";
    fs::path sourceLabels = path_source + "\\labels";

    fs::path destTrainImages = path_dest + "\\train\\images";
    fs::path destTrainLabels = path_dest + "\\train\\labels";
    fs::path destValidImages = path_dest + "\\valid\\images";
    fs::path destValidLabels = path_dest + "\\valid\\labels";

    makedir(destTrainImages);
    makedir(destTrainLabels);
    makedir(destValidImages);
    makedir(destValidLabels);

    try {
        CopyRandomFiles(sourceImages, sourceLabels, destTrainImages, destTrainLabels, destValidImages, destValidLabels, percentage, reductionFactor);
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "FileSystem error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Files have been copied successfully." << std::endl;

    return 0;
}

int makedir(fs::path dirPath)
{
    if (!fs::exists(dirPath)) {
        try {
            if (fs::create_directories(dirPath))
            {
                std::cout << "ディレクトリを作成しました: " << dirPath << std::endl;
            }
            else {
                std::cout << "ディレクトリを作成できませんでした。" << std::endl;
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "エラー: " << e.what() << std::endl;
        }
    }
    else {
        std::cout << "ディレクトリは既に存在します: " << dirPath << std::endl;
    }
    return 0;
}
