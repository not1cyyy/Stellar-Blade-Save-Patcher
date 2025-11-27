#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

fs::path getStellarBladeSave() {
    char path[MAX_PATH];

    bool found = SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path));
    if (found){
        fs::path localAppData = path;

        return localAppData / "SB" / "Saved" / "SaveGames" / "StellarBladeSave00.sav";
    }

    return {};
}

std::ptrdiff_t findSequence(const std::vector<unsigned char>& buffer) {
    const std::vector<unsigned char> targetBytes = {
        0x53, 0x42, 0x53, 0x61, 0x76, 0x65, 0x47, 0x61,
        0x6D, 0x65, 0x44, 0x61, 0x74, 0x61, 0x5F, 0x4F,
        0x6E, 0x6C, 0x69, 0x6E, 0x65, 0x41, 0x63, 0x63,
        0x6F, 0x75, 0x6E, 0x74, 0x49, 0x6E, 0x66, 0x6F
    }; // SBSaveGameData_OnlineAccountInfo

    auto it = std::search(buffer.begin(), buffer.end(),
                          targetBytes.begin(), targetBytes.end());

    if (it != buffer.end()) {
        std::ptrdiff_t index = std::distance(buffer.begin(), it);
        std::cout << "Found target byte sequence at offset: 0x"
                  << std::hex << index << std::dec << std::endl;
        return index;
    } else {
        std::cout << "Target byte sequence not found in the save file." << std::endl;
        return -1;
    }
}
int main() {
    fs::path saveFile = getStellarBladeSave();
    if (!fs::exists(saveFile)) {
        std::cout << "Save file not found at: " << saveFile << std::endl;
        return 1;
    }

    std::ifstream file(saveFile, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open save file." << std::endl;
        return 1;
    }

    // Read the entire file into a buffer
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Call findSequence
    std::ptrdiff_t offset = findSequence(buffer);
    if (offset == -1 || offset >= buffer.size()) {
    std::cout << "Invalid offset in original save file." << std::endl;
    return 1;
    }

    // Read from offset till the end of file 
    std::vector<unsigned char> importantBytes(buffer.begin() + offset, buffer.end());

    fs::path currentDir = fs::current_path();
    fs::path savFile;

    // Find the first .sav file in the current directory
    for (auto& entry : fs::directory_iterator(currentDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sav") {
            savFile = entry.path();
            break;
        }
    }

    if (savFile.empty()) {
        std::cout << "No .sav file found in the current directory." << std::endl;
        return 1;
    }

    std::cout << "Found .sav file: " << savFile << std::endl;

    // Read the entire .sav file into a buffer
    std::ifstream fffile(savFile, std::ios::binary);
    if (!fffile) {
        std::cout << "Failed to open .sav file." << std::endl;
        return 1;
    }

    std::vector<unsigned char> savBuffer((std::istreambuf_iterator<char>(fffile)), std::istreambuf_iterator<char>());
    fffile.close();

    // Call findSequence on savBuffer
    std::ptrdiff_t savOffset = findSequence(savBuffer);
    if (savOffset == -1) {
        std::cout << "Target byte sequence not found in the .sav file." << std::endl;
        return 1;
    }

    // Ensure savBuffer can fit importantBytes starting from savOffset
    if (savBuffer.size() - savOffset < importantBytes.size()) {
        savBuffer.resize(savOffset + importantBytes.size()); // resize if necessary
    }

    // Overwrite savBuffer from savOffset with importantBytes
    std::copy(importantBytes.begin(), importantBytes.end(), savBuffer.begin() + savOffset);

    // Write modified savBuffer back to the .sav file
    std::ofstream outFile(savFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cout << "Failed to open .sav file for writing." << std::endl;
        return 1;
    }
    outFile.write(reinterpret_cast<const char*>(savBuffer.data()), savBuffer.size());
    outFile.close();

    // Rename the .sav file to StellarBladeSave00.sav
    fs::path newSavFile = savFile.parent_path() / "StellarBladeSave00.sav";
    if (saveFile != newSavFile){
        fs::rename(savFile, newSavFile); 
    }

    std::cout << "Successfully patched the .sav file." << std::endl;
    std::cout << "Overwrote " << importantBytes.size() << " bytes at offset 0x" << std::hex << savOffset << std::dec << std::endl;

}   