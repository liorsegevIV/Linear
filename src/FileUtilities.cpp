#include "FileUtilities.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

// ── Path helpers ──────────────────────────────────────────────────────────────

fs::path FileUtilities::join(const fs::path& base, const fs::path& relative) {
    return base / relative;
}

fs::path FileUtilities::normalize(const fs::path& path) {
    return fs::weakly_canonical(path);
}

bool FileUtilities::exists(const fs::path& path) {
    return fs::exists(path);
}

void FileUtilities::ensureDirectory(const fs::path& path) {
    std::error_code ec;
    fs::create_directories(path, ec);
    if (ec) {
        throw std::runtime_error("Failed to create directory '" + path.string() +
                                 "': " + ec.message());
    }
}

// ── Read / write ──────────────────────────────────────────────────────────────

std::string FileUtilities::readText(const fs::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open file for reading: " + path.string());
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void FileUtilities::writeText(const fs::path& path,
                               const std::string& content,
                               bool overwrite) {
    if (!overwrite && fs::exists(path)) {
        throw std::runtime_error("File already exists (overwrite=false): " +
                                 path.string());
    }
    std::ofstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing: " + path.string());
    }
    file << content;
    if (!file) {
        throw std::runtime_error("Failed to write to file: " + path.string());
    }
}

std::vector<std::uint8_t> FileUtilities::readBytes(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for reading: " + path.string());
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void FileUtilities::writeBytes(const fs::path& path,
                                const std::vector<std::uint8_t>& data,
                                bool overwrite) {
    if (!overwrite && fs::exists(path)) {
        throw std::runtime_error("File already exists (overwrite=false): " +
                                 path.string());
    }
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing: " + path.string());
    }
    file.write(reinterpret_cast<const char*>(data.data()),
               static_cast<std::streamsize>(data.size()));
    if (!file) {
        throw std::runtime_error("Failed to write to file: " + path.string());
    }
}

nlohmann::json FileUtilities::readJson(const fs::path& path) {
    const std::string text = readText(path);
    try {
        return nlohmann::json::parse(text);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON parse error in '" + path.string() +
                                 "': " + e.what());
    }
}

void FileUtilities::writeJson(const fs::path& path,
                               const nlohmann::json& json,
                               bool overwrite,
                               int indent) {
    writeText(path, json.dump(indent) + "\n", overwrite);
}

// ── Copy / move / delete ──────────────────────────────────────────────────────

void FileUtilities::copy(const fs::path& source,
                          const fs::path& dest,
                          bool overwrite) {
    const auto opts = overwrite ? fs::copy_options::overwrite_existing
                                : fs::copy_options::none;
    std::error_code ec;
    fs::copy(source, dest, opts, ec);
    if (ec) {
        throw std::runtime_error("Copy failed from '" + source.string() + "' to '" +
                                 dest.string() + "': " + ec.message());
    }
}

void FileUtilities::move(const fs::path& source,
                          const fs::path& dest,
                          bool overwrite) {
    if (!overwrite && fs::exists(dest)) {
        throw std::runtime_error("Destination already exists (overwrite=false): " +
                                 dest.string());
    }
    std::error_code ec;
    fs::rename(source, dest, ec);
    if (ec) {
        // fs::rename fails across devices; fall back to copy-then-delete
        fs::copy(source, dest, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            throw std::runtime_error("Move (copy stage) failed from '" +
                                     source.string() + "': " + ec.message());
        }
        fs::remove(source, ec);
        if (ec) {
            throw std::runtime_error("Move (remove stage) failed for '" +
                                     source.string() + "': " + ec.message());
        }
    }
}

void FileUtilities::remove(const fs::path& path, bool missingOk) {
    if (!fs::exists(path)) {
        if (missingOk) {
            return;
        }
        throw std::runtime_error("File does not exist: " + path.string());
    }
    std::error_code ec;
    fs::remove_all(path, ec);
    if (ec) {
        throw std::runtime_error("Failed to remove '" + path.string() +
                                 "': " + ec.message());
    }
}

// ── Directory enumeration ─────────────────────────────────────────────────────

std::vector<fs::path> FileUtilities::listDirectory(const fs::path& dir) {
    if (!fs::is_directory(dir)) {
        throw std::runtime_error("Not a directory: " + dir.string());
    }
    std::vector<fs::path> entries;
    for (const auto& entry : fs::directory_iterator(dir)) {
        entries.push_back(entry.path());
    }
    std::sort(entries.begin(), entries.end());
    return entries;
}

namespace {

// Wildcard match supporting * (any sequence) and ? (any single character)
bool wildcardMatch(const std::string& pattern, const std::string& text) {
    const char* p = pattern.c_str();
    const char* t = text.c_str();
    const char* star = nullptr;
    const char* afterStar = nullptr;

    while (*t != '\0') {
        if (*p == '?' || *p == *t) {
            ++p;
            ++t;
        } else if (*p == '*') {
            star = p++;
            afterStar = t;
        } else if (star != nullptr) {
            p = star + 1;
            t = ++afterStar;
        } else {
            return false;
        }
    }
    while (*p == '*') {
        ++p;
    }
    return *p == '\0';
}

} // namespace

std::vector<fs::path> FileUtilities::glob(const fs::path& dir,
                                           const std::string& pattern) {
    if (!fs::is_directory(dir)) {
        throw std::runtime_error("Not a directory: " + dir.string());
    }
    std::vector<fs::path> results;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (wildcardMatch(pattern, entry.path().filename().string())) {
            results.push_back(entry.path());
        }
    }
    std::sort(results.begin(), results.end());
    return results;
}

std::vector<fs::path> FileUtilities::rglob(const fs::path& dir,
                                             const std::string& pattern) {
    if (!fs::is_directory(dir)) {
        throw std::runtime_error("Not a directory: " + dir.string());
    }
    std::vector<fs::path> results;
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (wildcardMatch(pattern, entry.path().filename().string())) {
            results.push_back(entry.path());
        }
    }
    std::sort(results.begin(), results.end());
    return results;
}

std::vector<fs::path> FileUtilities::filterByExtension(
    const std::vector<fs::path>& files, const std::string& extension) {
    const std::string ext =
        (!extension.empty() && extension[0] != '.') ? ("." + extension) : extension;
    std::vector<fs::path> result;
    for (const auto& f : files) {
        if (f.extension() == ext) {
            result.push_back(f);
        }
    }
    return result;
}
