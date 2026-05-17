#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

class FileUtilities {
public:
    FileUtilities() = delete;

    // ── Path helpers ──────────────────────────────────────────────────────────
    static std::filesystem::path join(const std::filesystem::path& base,
                                      const std::filesystem::path& relative);
    static std::filesystem::path normalize(const std::filesystem::path& path);
    static bool exists(const std::filesystem::path& path);
    static void ensureDirectory(const std::filesystem::path& path);

    // ── Read / write ──────────────────────────────────────────────────────────
    static std::string readText(const std::filesystem::path& path);
    static void writeText(const std::filesystem::path& path,
                          const std::string& content,
                          bool overwrite = true);

    static std::vector<std::uint8_t> readBytes(const std::filesystem::path& path);
    static void writeBytes(const std::filesystem::path& path,
                           const std::vector<std::uint8_t>& data,
                           bool overwrite = true);

    static nlohmann::json readJson(const std::filesystem::path& path);
    static void writeJson(const std::filesystem::path& path,
                          const nlohmann::json& json,
                          bool overwrite = true,
                          int indent = 4);

    // ── Copy / move / delete ──────────────────────────────────────────────────
    static void copy(const std::filesystem::path& source,
                     const std::filesystem::path& dest,
                     bool overwrite = false);
    static void move(const std::filesystem::path& source,
                     const std::filesystem::path& dest,
                     bool overwrite = false);
    static void remove(const std::filesystem::path& path, bool missingOk = false);

    // ── Directory enumeration ─────────────────────────────────────────────────
    static std::vector<std::filesystem::path> listDirectory(
        const std::filesystem::path& dir);
    static std::vector<std::filesystem::path> glob(const std::filesystem::path& dir,
                                                    const std::string& pattern);
    static std::vector<std::filesystem::path> rglob(const std::filesystem::path& dir,
                                                     const std::string& pattern);
    static std::vector<std::filesystem::path> filterByExtension(
        const std::vector<std::filesystem::path>& files,
        const std::string& extension);
};
