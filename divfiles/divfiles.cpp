#include <iostream>
#include <filesystem>
#include <vector>
#include <random>
#include <algorithm>
#include <string>

namespace fs = std::filesystem;

// ファイル名（拡張子なし）を取得するヘルパー関数
std::string GetStem(const fs::path& path) 
{
    return path.stem().string();
}

int makedir(fs::path dirPath);


// ランダムにファイルの一部を別のディレクトリにコピーする関数
void CopyRandomFiles(
    const fs::path& sourceImages, 
    const fs::path& sourceLabels, 
    const fs::path& destTrainImages, 
    const fs::path& destTrainLabels, 
    const fs::path& destValidImages, 
    const fs::path& destValidLabels, 
    float percentage
) 
{
    std::vector<fs::path> jpgFiles;

    // *.jpgファイルをリストアップ
    for (const auto& entry : fs::directory_iterator(sourceImages)) {
        if (
            entry.is_regular_file() && 
            (entry.path().extension() == ".JPG"|| entry.path().extension() == ".jpg")
        )
        {
            jpgFiles.push_back(entry.path());
        }
    }

    // ファイルリストをシャッフル
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(jpgFiles.begin(), jpgFiles.end(), g);

    size_t filesToCopy = static_cast<size_t>(std::ceil(jpgFiles.size() * (percentage / 100.0)));

    // 選択されたファイルをコピー
    for (size_t i = 0; i < filesToCopy; ++i) {
        fs::path destImagePath = destTrainImages / jpgFiles[i].filename();
        fs::copy(jpgFiles[i], destImagePath, fs::copy_options::overwrite_existing);

        // 同名の.txtファイルをコピー
        fs::path sourceLabelPath = sourceLabels / jpgFiles[i].stem().concat(".txt");
        fs::path destLabelPath = destTrainLabels / sourceLabelPath.filename();
        if (fs::exists(sourceLabelPath)) {
            fs::copy(sourceLabelPath, destLabelPath, fs::copy_options::overwrite_existing);
        }

        jpgFiles[i].clear(); // コピーしたらリストから削除
    }

    // コピーされなかったファイルをvalidディレクトリにコピー
    for (const auto& path : jpgFiles) {
        if (path.empty()) continue; // 既にコピーされたファイルをスキップ
        fs::path destImagePath = destValidImages / path.filename();
        fs::copy(path, destImagePath, fs::copy_options::overwrite_existing);

        // 同名の.txtファイルをコピー
        fs::path sourceLabelPath = sourceLabels / path.stem().concat(".txt");
        fs::path destLabelPath = destValidLabels / sourceLabelPath.filename();
        if (fs::exists(sourceLabelPath)) {
            fs::copy(sourceLabelPath, destLabelPath, fs::copy_options::overwrite_existing);
        }
    }
}


int main(int argc, char* argv[])
{
//    std::string path_source = "C:\\Programming\\python\\yolov5\\data\\new_images\\20240301_tozaidouro_truckyard";
//    std::string path_dest = "C:\\Programming\\python\\yolov5\\data\\new_images\\20240301_tozaidouro_truckyard_tmp";

    std::string path_source = "";
    std::string path_dest = "";
    float percentage = 50.0f; // コピーする割合 (%)

    if (argc != 4)
    {
        std::cout << argv[0] << std::endl;
        std::cout << "Copy files for yolo" << std::endl;
        std::cout << "usage: " << std::endl;
        std::cout << argv[0] << " [source_path] [dest_path] [divide_value:1-99]" << std::endl;

        std::cout << "[source_path]┳images━jpgfiles" << std::endl;
        std::cout << "             ┗labels━txtfiles" << std::endl;
        std::cout << "[dest_path]┳train┳images━jpgfiles" << std::endl;
        std::cout << "           ┃     ┗labels━txtfiles" << std::endl;
        std::cout << "           ┗valid┳images━jpgfiles" << std::endl;
        std::cout << "                  ┗labels━txtfiles" << std::endl;

        return 0;
    }
    else
    {
        path_source = argv[1];
        path_dest = argv[2];

        int number = std::atoi(argv[3]); // 文字列を数値に変換

        // 取得したパスと数値を表示
        std::cout << "source      : " << path_source << std::endl;
        std::cout << "destination : " << path_dest << std::endl;
        std::cout << "divide value: " << number << "[%]" << std::endl;

        percentage = number;
    }

    //fs::path sourceImages = "C:\\Programming\\python\\yolov5\\data\\new_images\\20240301_tozaidouro_truckyard\\images";
    //fs::path sourceLabels = "C:\\Programming\\python\\yolov5\\data\\new_images\\20240301_tozaidouro_truckyard\\labels";

    fs::path sourceImages = path_source + "\\images";
    fs::path sourceLabels = path_source + "\\labels";

    //fs::path destTrainImages = "C:\\Programming\\python\\yolov5\\data\\new_images\\20240301_tozaidouro_truckyard\\train\\images";
    //fs::path destTrainLabels = "C:\\Programming\\python\\yolov5\\data\\new_images\\20240301_tozaidouro_truckyard\\train\\labels";
    //fs::path destValidImages = "C:\\Programming\\python\\yolov5\\data\\new_images\\20240301_tozaidouro_truckyard\\valid\\images";
    //fs::path destValidLabels = "C:\\Programming\\python\\yolov5\\data\\new_images\\20240301_tozaidouro_truckyard\\valid\\labels";

    fs::path destTrainImages = path_dest+"\\train\\images";
    fs::path destTrainLabels = path_dest + "\\train\\labels";
    fs::path destValidImages = path_dest + "\\valid\\images";
    fs::path destValidLabels = path_dest + "\\valid\\labels";

    makedir(destTrainImages);
    makedir(destTrainLabels);
    makedir(destValidImages);
    makedir(destValidLabels);

    try {
        CopyRandomFiles(sourceImages, sourceLabels, destTrainImages, destTrainLabels, destValidImages, destValidLabels, percentage);
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
    // ディレクトリが存在しない場合に作成
    if (!fs::exists(dirPath)) 
    {
        try 
        {
            // ディレクトリ作成
            if (fs::create_directories(dirPath))
            {
                std::cout << "ディレクトリを作成しました: " << dirPath << std::endl;
            }
            else 
            {
                std::cout << "ディレクトリを作成できませんでした。" << std::endl;
            }
        }
        catch (const fs::filesystem_error& e) 
        {
            std::cerr << "エラー: " << e.what() << std::endl;
        }
    }
    else 
    {
        std::cout << "ディレクトリは既に存在します: " << dirPath << std::endl;
    }

    return 0;
}