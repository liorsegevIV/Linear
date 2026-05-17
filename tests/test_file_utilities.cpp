#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "FileUtilities.h"

namespace fs = std::filesystem;

// ── Fixture ───────────────────────────────────────────────────────────────────

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() /
               ("fu_test_" + std::to_string(
                                 std::hash<std::thread::id>{}(std::this_thread::get_id()) ^
                                 static_cast<size_t>(
                                     std::chrono::steady_clock::now().time_since_epoch().count())));
        fs::create_directories(path);
    }

    ~TempDir() { fs::remove_all(path); }

    fs::path operator/(const std::string& name) const { return path / name; }
};

// ── Path helpers ──────────────────────────────────────────────────────────────

TEST_CASE("join combines base and relative path", "[path]") {
    const fs::path result = FileUtilities::join("/foo/bar", "baz.txt");
    REQUIRE(result == fs::path("/foo/bar/baz.txt"));
}

TEST_CASE("join with nested relative path", "[path]") {
    const fs::path result = FileUtilities::join("/foo", "bar/baz");
    REQUIRE(result == fs::path("/foo/bar/baz"));
}

TEST_CASE("normalize resolves dots", "[path]") {
    const fs::path result = FileUtilities::normalize("/foo/../foo/./bar");
    REQUIRE(result == fs::path("/foo/bar"));
}

TEST_CASE("exists returns true for an existing file", "[path]") {
    TempDir tmp;
    const fs::path f = tmp / "exists.txt";
    std::ofstream(f) << "hi";
    REQUIRE(FileUtilities::exists(f));
}

TEST_CASE("exists returns false for a non-existent path", "[path]") {
    REQUIRE_FALSE(FileUtilities::exists("/tmp/fu_no_such_file_xyz"));
}

TEST_CASE("ensureDirectory creates nested directories", "[path]") {
    TempDir tmp;
    const fs::path nested = tmp.path / "a" / "b" / "c";
    FileUtilities::ensureDirectory(nested);
    REQUIRE(fs::is_directory(nested));
}

TEST_CASE("ensureDirectory is idempotent", "[path]") {
    TempDir tmp;
    FileUtilities::ensureDirectory(tmp.path);
    REQUIRE_NOTHROW(FileUtilities::ensureDirectory(tmp.path));
}

// ── readText / writeText ──────────────────────────────────────────────────────

TEST_CASE("writeText then readText round-trips content", "[text]") {
    TempDir tmp;
    const fs::path f = tmp / "hello.txt";
    FileUtilities::writeText(f, "Hello, world!");
    REQUIRE(FileUtilities::readText(f) == "Hello, world!");
}

TEST_CASE("writeText overwrites existing file by default", "[text]") {
    TempDir tmp;
    const fs::path f = tmp / "over.txt";
    FileUtilities::writeText(f, "first");
    FileUtilities::writeText(f, "second");
    REQUIRE(FileUtilities::readText(f) == "second");
}

TEST_CASE("writeText throws when overwrite=false and file exists", "[text]") {
    TempDir tmp;
    const fs::path f = tmp / "noover.txt";
    FileUtilities::writeText(f, "original");
    REQUIRE_THROWS_AS(FileUtilities::writeText(f, "new", false), std::runtime_error);
}

TEST_CASE("readText throws for non-existent file", "[text]") {
    REQUIRE_THROWS_AS(FileUtilities::readText("/tmp/fu_ghost_xyz.txt"),
                      std::runtime_error);
}

TEST_CASE("writeText preserves empty string", "[text]") {
    TempDir tmp;
    const fs::path f = tmp / "empty.txt";
    FileUtilities::writeText(f, "");
    REQUIRE(FileUtilities::readText(f) == "");
}

TEST_CASE("writeText preserves multi-line content", "[text]") {
    TempDir tmp;
    const fs::path f = tmp / "multi.txt";
    const std::string content = "line1\nline2\nline3\n";
    FileUtilities::writeText(f, content);
    REQUIRE(FileUtilities::readText(f) == content);
}

// ── readBytes / writeBytes ────────────────────────────────────────────────────

TEST_CASE("writeBytes then readBytes round-trips binary data", "[binary]") {
    TempDir tmp;
    const fs::path f = tmp / "data.bin";
    const std::vector<std::uint8_t> data = {0x00, 0xFF, 0x7F, 0x80, 0x01};
    FileUtilities::writeBytes(f, data);
    REQUIRE(FileUtilities::readBytes(f) == data);
}

TEST_CASE("writeBytes throws when overwrite=false and file exists", "[binary]") {
    TempDir tmp;
    const fs::path f = tmp / "bin_noover.bin";
    FileUtilities::writeBytes(f, {0x01});
    REQUIRE_THROWS_AS(FileUtilities::writeBytes(f, {0x02}, false), std::runtime_error);
}

TEST_CASE("readBytes throws for non-existent file", "[binary]") {
    REQUIRE_THROWS_AS(FileUtilities::readBytes("/tmp/fu_ghost_bin_xyz.bin"),
                      std::runtime_error);
}

TEST_CASE("writeBytes handles empty buffer", "[binary]") {
    TempDir tmp;
    const fs::path f = tmp / "empty.bin";
    FileUtilities::writeBytes(f, {});
    REQUIRE(FileUtilities::readBytes(f).empty());
}

// ── readJson / writeJson ──────────────────────────────────────────────────────

TEST_CASE("writeJson then readJson round-trips JSON object", "[json]") {
    TempDir tmp;
    const fs::path f = tmp / "data.json";
    const nlohmann::json j = {{"key", "value"}, {"num", 42}, {"flag", true}};
    FileUtilities::writeJson(f, j);
    const nlohmann::json loaded = FileUtilities::readJson(f);
    REQUIRE(loaded == j);
}

TEST_CASE("writeJson then readJson round-trips JSON array", "[json]") {
    TempDir tmp;
    const fs::path f = tmp / "arr.json";
    const nlohmann::json j = nlohmann::json::array({1, 2, 3});
    FileUtilities::writeJson(f, j);
    REQUIRE(FileUtilities::readJson(f) == j);
}

TEST_CASE("readJson throws on malformed JSON", "[json]") {
    TempDir tmp;
    const fs::path f = tmp / "bad.json";
    FileUtilities::writeText(f, "{not valid json}");
    REQUIRE_THROWS_AS(FileUtilities::readJson(f), std::runtime_error);
}

TEST_CASE("writeJson throws when overwrite=false and file exists", "[json]") {
    TempDir tmp;
    const fs::path f = tmp / "j_noover.json";
    FileUtilities::writeJson(f, {{"a", 1}});
    REQUIRE_THROWS_AS(FileUtilities::writeJson(f, {{"b", 2}}, false),
                      std::runtime_error);
}

// ── copy ─────────────────────────────────────────────────────────────────────

TEST_CASE("copy duplicates a file", "[copy]") {
    TempDir tmp;
    const fs::path src = tmp / "src.txt";
    const fs::path dst = tmp / "dst.txt";
    FileUtilities::writeText(src, "content");
    FileUtilities::copy(src, dst);
    REQUIRE(FileUtilities::readText(dst) == "content");
    REQUIRE(FileUtilities::exists(src));
}

TEST_CASE("copy throws when dest exists and overwrite=false", "[copy]") {
    TempDir tmp;
    const fs::path src = tmp / "src.txt";
    const fs::path dst = tmp / "dst.txt";
    FileUtilities::writeText(src, "src");
    FileUtilities::writeText(dst, "dst");
    REQUIRE_THROWS_AS(FileUtilities::copy(src, dst, false), std::runtime_error);
}

TEST_CASE("copy overwrites dest when overwrite=true", "[copy]") {
    TempDir tmp;
    const fs::path src = tmp / "src.txt";
    const fs::path dst = tmp / "dst.txt";
    FileUtilities::writeText(src, "new");
    FileUtilities::writeText(dst, "old");
    FileUtilities::copy(src, dst, true);
    REQUIRE(FileUtilities::readText(dst) == "new");
}

// ── move ─────────────────────────────────────────────────────────────────────

TEST_CASE("move relocates a file", "[move]") {
    TempDir tmp;
    const fs::path src = tmp / "src.txt";
    const fs::path dst = tmp / "dst.txt";
    FileUtilities::writeText(src, "moveme");
    FileUtilities::move(src, dst);
    REQUIRE(FileUtilities::readText(dst) == "moveme");
    REQUIRE_FALSE(FileUtilities::exists(src));
}

TEST_CASE("move throws when dest exists and overwrite=false", "[move]") {
    TempDir tmp;
    const fs::path src = tmp / "src.txt";
    const fs::path dst = tmp / "dst.txt";
    FileUtilities::writeText(src, "src");
    FileUtilities::writeText(dst, "dst");
    REQUIRE_THROWS_AS(FileUtilities::move(src, dst, false), std::runtime_error);
}

TEST_CASE("move overwrites dest when overwrite=true", "[move]") {
    TempDir tmp;
    const fs::path src = tmp / "src.txt";
    const fs::path dst = tmp / "dst.txt";
    FileUtilities::writeText(src, "new");
    FileUtilities::writeText(dst, "old");
    FileUtilities::move(src, dst, true);
    REQUIRE(FileUtilities::readText(dst) == "new");
    REQUIRE_FALSE(FileUtilities::exists(src));
}

// ── remove ────────────────────────────────────────────────────────────────────

TEST_CASE("remove deletes an existing file", "[remove]") {
    TempDir tmp;
    const fs::path f = tmp / "del.txt";
    FileUtilities::writeText(f, "bye");
    FileUtilities::remove(f);
    REQUIRE_FALSE(FileUtilities::exists(f));
}

TEST_CASE("remove deletes a directory tree", "[remove]") {
    TempDir tmp;
    const fs::path dir = tmp.path / "subdir";
    fs::create_directories(dir / "nested");
    FileUtilities::remove(dir);
    REQUIRE_FALSE(FileUtilities::exists(dir));
}

TEST_CASE("remove throws for missing file when missingOk=false", "[remove]") {
    REQUIRE_THROWS_AS(FileUtilities::remove("/tmp/fu_no_exist_xyz", false),
                      std::runtime_error);
}

TEST_CASE("remove is silent for missing file when missingOk=true", "[remove]") {
    REQUIRE_NOTHROW(FileUtilities::remove("/tmp/fu_no_exist_xyz", true));
}

// ── listDirectory ─────────────────────────────────────────────────────────────

TEST_CASE("listDirectory returns sorted entries", "[dir]") {
    TempDir tmp;
    FileUtilities::writeText(tmp / "b.txt", "");
    FileUtilities::writeText(tmp / "a.txt", "");
    FileUtilities::writeText(tmp / "c.txt", "");
    const auto entries = FileUtilities::listDirectory(tmp.path);
    REQUIRE(entries.size() == 3);
    REQUIRE(entries[0].filename() == "a.txt");
    REQUIRE(entries[1].filename() == "b.txt");
    REQUIRE(entries[2].filename() == "c.txt");
}

TEST_CASE("listDirectory on empty directory returns empty vector", "[dir]") {
    TempDir tmp;
    const fs::path empty = tmp.path / "empty";
    fs::create_directory(empty);
    REQUIRE(FileUtilities::listDirectory(empty).empty());
}

TEST_CASE("listDirectory throws for non-directory path", "[dir]") {
    TempDir tmp;
    const fs::path f = tmp / "file.txt";
    FileUtilities::writeText(f, "x");
    REQUIRE_THROWS_AS(FileUtilities::listDirectory(f), std::runtime_error);
}

// ── glob ─────────────────────────────────────────────────────────────────────

TEST_CASE("glob matches files by wildcard", "[glob]") {
    TempDir tmp;
    FileUtilities::writeText(tmp / "foo.cpp", "");
    FileUtilities::writeText(tmp / "bar.cpp", "");
    FileUtilities::writeText(tmp / "baz.txt", "");
    const auto results = FileUtilities::glob(tmp.path, "*.cpp");
    REQUIRE(results.size() == 2);
}

TEST_CASE("glob with ? wildcard matches single character", "[glob]") {
    TempDir tmp;
    FileUtilities::writeText(tmp / "a1.txt", "");
    FileUtilities::writeText(tmp / "a2.txt", "");
    FileUtilities::writeText(tmp / "ab.txt", "");
    const auto results = FileUtilities::glob(tmp.path, "a?.txt");
    REQUIRE(results.size() == 3);
}

TEST_CASE("glob returns empty when nothing matches", "[glob]") {
    TempDir tmp;
    FileUtilities::writeText(tmp / "foo.txt", "");
    REQUIRE(FileUtilities::glob(tmp.path, "*.cpp").empty());
}

TEST_CASE("glob throws for non-directory path", "[glob]") {
    TempDir tmp;
    const fs::path f = tmp / "file.txt";
    FileUtilities::writeText(f, "x");
    REQUIRE_THROWS_AS(FileUtilities::glob(f, "*"), std::runtime_error);
}

// ── rglob ─────────────────────────────────────────────────────────────────────

TEST_CASE("rglob finds files in subdirectories", "[rglob]") {
    TempDir tmp;
    const fs::path sub = tmp.path / "sub";
    fs::create_directory(sub);
    FileUtilities::writeText(tmp / "root.cpp", "");
    FileUtilities::writeText(sub / "deep.cpp", "");
    FileUtilities::writeText(sub / "other.txt", "");
    const auto results = FileUtilities::rglob(tmp.path, "*.cpp");
    REQUIRE(results.size() == 2);
}

TEST_CASE("rglob throws for non-directory path", "[rglob]") {
    TempDir tmp;
    const fs::path f = tmp / "file.txt";
    FileUtilities::writeText(f, "x");
    REQUIRE_THROWS_AS(FileUtilities::rglob(f, "*"), std::runtime_error);
}

// ── filterByExtension ─────────────────────────────────────────────────────────

TEST_CASE("filterByExtension filters by extension with leading dot", "[filter]") {
    const std::vector<fs::path> files = {"a.cpp", "b.h", "c.cpp", "d.txt"};
    const auto result = FileUtilities::filterByExtension(files, ".cpp");
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == "a.cpp");
    REQUIRE(result[1] == "c.cpp");
}

TEST_CASE("filterByExtension normalises extension without leading dot", "[filter]") {
    const std::vector<fs::path> files = {"a.cpp", "b.h", "c.cpp"};
    const auto result = FileUtilities::filterByExtension(files, "cpp");
    REQUIRE(result.size() == 2);
}

TEST_CASE("filterByExtension returns empty when no match", "[filter]") {
    const std::vector<fs::path> files = {"a.cpp", "b.cpp"};
    REQUIRE(FileUtilities::filterByExtension(files, ".h").empty());
}

TEST_CASE("filterByExtension on empty input returns empty", "[filter]") {
    REQUIRE(FileUtilities::filterByExtension({}, ".cpp").empty());
}
