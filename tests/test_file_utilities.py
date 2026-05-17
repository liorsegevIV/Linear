import json
import pytest
from pathlib import Path

from file_utilities import FileUtilities


# ── Path helpers ──────────────────────────────────────────────────────────────


class TestJoin:
    def test_two_parts(self):
        assert FileUtilities.join("a", "b") == Path("a/b")

    def test_three_parts(self):
        assert FileUtilities.join("a", "b", "c") == Path("a/b/c")

    def test_single_part(self):
        assert FileUtilities.join("a") == Path("a")

    def test_no_parts_raises(self):
        with pytest.raises(ValueError):
            FileUtilities.join()

    def test_accepts_path_objects(self, tmp_path):
        result = FileUtilities.join(tmp_path, "sub")
        assert result == tmp_path / "sub"


class TestNormalize:
    def test_resolves_dotdot(self, tmp_path):
        result = FileUtilities.normalize(tmp_path / ".." / tmp_path.name)
        assert result == tmp_path.resolve()

    def test_absolute_stays_absolute(self, tmp_path):
        result = FileUtilities.normalize(tmp_path)
        assert result.is_absolute()


class TestExists:
    def test_existing_dir(self, tmp_path):
        assert FileUtilities.exists(tmp_path) is True

    def test_existing_file(self, tmp_path):
        p = tmp_path / "f.txt"
        p.write_text("x")
        assert FileUtilities.exists(p) is True

    def test_missing(self, tmp_path):
        assert FileUtilities.exists(tmp_path / "nope") is False


class TestEnsureDirectory:
    def test_creates_nested(self, tmp_path):
        target = tmp_path / "a" / "b" / "c"
        result = FileUtilities.ensure_directory(target)
        assert result.is_dir()

    def test_idempotent(self, tmp_path):
        FileUtilities.ensure_directory(tmp_path)
        FileUtilities.ensure_directory(tmp_path)
        assert tmp_path.is_dir()

    def test_returns_path(self, tmp_path):
        target = tmp_path / "new"
        result = FileUtilities.ensure_directory(target)
        assert result == target


# ── Read / write helpers ──────────────────────────────────────────────────────


class TestReadWriteText:
    def test_round_trip(self, tmp_path):
        p = tmp_path / "hello.txt"
        FileUtilities.write_text(p, "hello world")
        assert FileUtilities.read_text(p) == "hello world"

    def test_custom_encoding(self, tmp_path):
        p = tmp_path / "latin.txt"
        FileUtilities.write_text(p, "café", encoding="latin-1")
        assert FileUtilities.read_text(p, encoding="latin-1") == "café"

    def test_read_missing_raises(self, tmp_path):
        with pytest.raises(OSError):
            FileUtilities.read_text(tmp_path / "missing.txt")

    def test_overwrite(self, tmp_path):
        p = tmp_path / "f.txt"
        FileUtilities.write_text(p, "old")
        FileUtilities.write_text(p, "new")
        assert FileUtilities.read_text(p) == "new"


class TestReadWriteBytes:
    def test_round_trip(self, tmp_path):
        p = tmp_path / "data.bin"
        FileUtilities.write_bytes(p, b"\x00\x01\x02\xff")
        assert FileUtilities.read_bytes(p) == b"\x00\x01\x02\xff"

    def test_read_missing_raises(self, tmp_path):
        with pytest.raises(OSError):
            FileUtilities.read_bytes(tmp_path / "missing.bin")


class TestReadWriteJson:
    def test_round_trip_dict(self, tmp_path):
        p = tmp_path / "data.json"
        data = {"key": "value", "num": 42, "list": [1, 2, 3]}
        FileUtilities.write_json(p, data)
        assert FileUtilities.read_json(p) == data

    def test_round_trip_list(self, tmp_path):
        p = tmp_path / "arr.json"
        FileUtilities.write_json(p, [1, 2, 3])
        assert FileUtilities.read_json(p) == [1, 2, 3]

    def test_default_indent(self, tmp_path):
        p = tmp_path / "data.json"
        FileUtilities.write_json(p, {"a": 1})
        assert "\n" in p.read_text()

    def test_read_invalid_json_raises(self, tmp_path):
        p = tmp_path / "bad.json"
        p.write_text("not json")
        with pytest.raises(json.JSONDecodeError):
            FileUtilities.read_json(p)

    def test_read_missing_raises(self, tmp_path):
        with pytest.raises(OSError):
            FileUtilities.read_json(tmp_path / "missing.json")

    def test_write_non_serialisable_raises(self, tmp_path):
        p = tmp_path / "data.json"
        with pytest.raises(TypeError):
            FileUtilities.write_json(p, object())


# ── Copy / move / delete ──────────────────────────────────────────────────────


class TestCopy:
    def test_copies_content(self, tmp_path):
        src = tmp_path / "src.txt"
        dst = tmp_path / "dst.txt"
        src.write_text("hello")
        FileUtilities.copy(src, dst)
        assert dst.read_text() == "hello"
        assert src.exists()

    def test_returns_destination(self, tmp_path):
        src = tmp_path / "src.txt"
        dst = tmp_path / "dst.txt"
        src.write_text("x")
        result = FileUtilities.copy(src, dst)
        assert result == dst

    def test_no_overwrite_raises(self, tmp_path):
        src, dst = tmp_path / "src.txt", tmp_path / "dst.txt"
        src.write_text("src")
        dst.write_text("dst")
        with pytest.raises(FileExistsError):
            FileUtilities.copy(src, dst, overwrite=False)

    def test_overwrite_replaces(self, tmp_path):
        src, dst = tmp_path / "src.txt", tmp_path / "dst.txt"
        src.write_text("new")
        dst.write_text("old")
        FileUtilities.copy(src, dst, overwrite=True)
        assert dst.read_text() == "new"


class TestMove:
    def test_moves_and_removes_source(self, tmp_path):
        src = tmp_path / "src.txt"
        dst = tmp_path / "dst.txt"
        src.write_text("content")
        FileUtilities.move(src, dst)
        assert dst.read_text() == "content"
        assert not src.exists()

    def test_returns_destination(self, tmp_path):
        src, dst = tmp_path / "src.txt", tmp_path / "dst.txt"
        src.write_text("x")
        result = FileUtilities.move(src, dst)
        assert result == dst

    def test_no_overwrite_raises(self, tmp_path):
        src, dst = tmp_path / "src.txt", tmp_path / "dst.txt"
        src.write_text("src")
        dst.write_text("dst")
        with pytest.raises(FileExistsError):
            FileUtilities.move(src, dst, overwrite=False)

    def test_overwrite_replaces(self, tmp_path):
        src, dst = tmp_path / "src.txt", tmp_path / "dst.txt"
        src.write_text("new")
        dst.write_text("old")
        FileUtilities.move(src, dst, overwrite=True)
        assert dst.read_text() == "new"
        assert not src.exists()


class TestDelete:
    def test_deletes_file(self, tmp_path):
        p = tmp_path / "file.txt"
        p.write_text("x")
        FileUtilities.delete(p)
        assert not p.exists()

    def test_deletes_directory_tree(self, tmp_path):
        d = tmp_path / "subdir"
        d.mkdir()
        (d / "child.txt").write_text("x")
        FileUtilities.delete(d)
        assert not d.exists()

    def test_missing_ok_true(self, tmp_path):
        FileUtilities.delete(tmp_path / "ghost.txt", missing_ok=True)

    def test_missing_ok_false_raises(self, tmp_path):
        with pytest.raises(OSError):
            FileUtilities.delete(tmp_path / "ghost.txt", missing_ok=False)


# ── Enumerate / glob / filter ─────────────────────────────────────────────────


class TestListDirectory:
    def test_lists_contents(self, tmp_path):
        (tmp_path / "a.txt").write_text("")
        (tmp_path / "b.txt").write_text("")
        names = [p.name for p in FileUtilities.list_directory(tmp_path)]
        assert "a.txt" in names
        assert "b.txt" in names

    def test_sorted_order(self, tmp_path):
        for name in ["c.txt", "a.txt", "b.txt"]:
            (tmp_path / name).write_text("")
        names = [p.name for p in FileUtilities.list_directory(tmp_path)]
        assert names == sorted(names)

    def test_missing_directory_raises(self, tmp_path):
        with pytest.raises(OSError):
            FileUtilities.list_directory(tmp_path / "nonexistent")


class TestGlob:
    def test_matches_pattern(self, tmp_path):
        (tmp_path / "a.txt").write_text("")
        (tmp_path / "b.py").write_text("")
        result = FileUtilities.glob(tmp_path, "*.txt")
        assert len(result) == 1
        assert result[0].name == "a.txt"

    def test_empty_when_no_match(self, tmp_path):
        assert FileUtilities.glob(tmp_path, "*.xyz") == []


class TestRglob:
    def test_recurses(self, tmp_path):
        sub = tmp_path / "sub"
        sub.mkdir()
        (tmp_path / "a.txt").write_text("")
        (sub / "b.txt").write_text("")
        result = FileUtilities.rglob(tmp_path, "*.txt")
        assert len(result) == 2

    def test_no_match_returns_empty(self, tmp_path):
        assert FileUtilities.rglob(tmp_path, "*.xyz") == []


class TestFilterFiles:
    def test_filters_by_extension_with_dot(self, tmp_path):
        paths = [tmp_path / "a.txt", tmp_path / "b.py", tmp_path / "c.txt"]
        for p in paths:
            p.write_text("")
        result = FileUtilities.filter_files(paths, ".txt")
        assert {p.name for p in result} == {"a.txt", "c.txt"}

    def test_filters_by_extension_without_dot(self, tmp_path):
        paths = [tmp_path / "a.py", tmp_path / "b.txt"]
        for p in paths:
            p.write_text("")
        result = FileUtilities.filter_files(paths, "py")
        assert len(result) == 1
        assert result[0].name == "a.py"

    def test_no_extension_returns_all_files(self, tmp_path):
        paths = [tmp_path / "a.txt", tmp_path / "b.py"]
        for p in paths:
            p.write_text("")
        result = FileUtilities.filter_files(paths)
        assert len(result) == 2

    def test_excludes_directories(self, tmp_path):
        d = tmp_path / "subdir"
        d.mkdir()
        f = tmp_path / "file.txt"
        f.write_text("")
        result = FileUtilities.filter_files([d, f])
        assert result == [f]
