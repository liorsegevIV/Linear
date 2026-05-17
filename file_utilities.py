import json
import logging
import shutil
from pathlib import Path
from typing import List, Optional, Union

logger = logging.getLogger(__name__)

PathLike = Union[str, Path]


class FileUtilities:
    """Centralized file operations: paths, read/write, copy/move/delete, and directory enumeration."""

    # ── Path helpers ──────────────────────────────────────────────────────────

    @staticmethod
    def join(*parts: PathLike) -> Path:
        if not parts:
            raise ValueError("join requires at least one path part")
        return Path(parts[0]).joinpath(*parts[1:])

    @staticmethod
    def normalize(path: PathLike) -> Path:
        return Path(path).resolve()

    @staticmethod
    def exists(path: PathLike) -> bool:
        return Path(path).exists()

    @staticmethod
    def ensure_directory(path: PathLike) -> Path:
        p = Path(path)
        p.mkdir(parents=True, exist_ok=True)
        logger.debug("Ensured directory: %s", p)
        return p

    # ── Read / write helpers ──────────────────────────────────────────────────

    @staticmethod
    def read_text(path: PathLike, encoding: str = "utf-8") -> str:
        p = Path(path)
        try:
            return p.read_text(encoding=encoding)
        except OSError as exc:
            logger.error("Failed to read text file %s: %s", p, exc)
            raise

    @staticmethod
    def write_text(path: PathLike, content: str, encoding: str = "utf-8") -> None:
        p = Path(path)
        try:
            p.write_text(content, encoding=encoding)
            logger.debug("Wrote text file: %s", p)
        except OSError as exc:
            logger.error("Failed to write text file %s: %s", p, exc)
            raise

    @staticmethod
    def read_bytes(path: PathLike) -> bytes:
        p = Path(path)
        try:
            return p.read_bytes()
        except OSError as exc:
            logger.error("Failed to read binary file %s: %s", p, exc)
            raise

    @staticmethod
    def write_bytes(path: PathLike, content: bytes) -> None:
        p = Path(path)
        try:
            p.write_bytes(content)
            logger.debug("Wrote binary file: %s", p)
        except OSError as exc:
            logger.error("Failed to write binary file %s: %s", p, exc)
            raise

    @staticmethod
    def read_json(path: PathLike) -> object:
        p = Path(path)
        try:
            return json.loads(p.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            logger.error("Failed to read JSON file %s: %s", p, exc)
            raise

    @staticmethod
    def write_json(path: PathLike, data: object, indent: int = 2) -> None:
        p = Path(path)
        try:
            p.write_text(json.dumps(data, indent=indent), encoding="utf-8")
            logger.debug("Wrote JSON file: %s", p)
        except (OSError, TypeError) as exc:
            logger.error("Failed to write JSON file %s: %s", p, exc)
            raise

    # ── Copy / move / delete ──────────────────────────────────────────────────

    @staticmethod
    def copy(src: PathLike, dst: PathLike, overwrite: bool = False) -> Path:
        src_path, dst_path = Path(src), Path(dst)
        if dst_path.exists() and not overwrite:
            raise FileExistsError(f"Destination already exists: {dst_path}")
        try:
            shutil.copy2(src_path, dst_path)
            logger.debug("Copied %s -> %s", src_path, dst_path)
            return dst_path
        except OSError as exc:
            logger.error("Failed to copy %s -> %s: %s", src_path, dst_path, exc)
            raise

    @staticmethod
    def move(src: PathLike, dst: PathLike, overwrite: bool = False) -> Path:
        src_path, dst_path = Path(src), Path(dst)
        if dst_path.exists() and not overwrite:
            raise FileExistsError(f"Destination already exists: {dst_path}")
        try:
            shutil.move(str(src_path), str(dst_path))
            logger.debug("Moved %s -> %s", src_path, dst_path)
            return Path(dst_path)
        except OSError as exc:
            logger.error("Failed to move %s -> %s: %s", src_path, dst_path, exc)
            raise

    @staticmethod
    def delete(path: PathLike, missing_ok: bool = False) -> None:
        p = Path(path)
        try:
            if p.is_dir():
                shutil.rmtree(p)
                logger.debug("Deleted directory: %s", p)
            else:
                p.unlink(missing_ok=missing_ok)
                logger.debug("Deleted file: %s", p)
        except OSError as exc:
            logger.error("Failed to delete %s: %s", p, exc)
            raise

    # ── Enumerate / glob / filter ─────────────────────────────────────────────

    @staticmethod
    def list_directory(path: PathLike) -> List[Path]:
        p = Path(path)
        try:
            return sorted(p.iterdir())
        except OSError as exc:
            logger.error("Failed to list directory %s: %s", p, exc)
            raise

    @staticmethod
    def glob(path: PathLike, pattern: str) -> List[Path]:
        p = Path(path)
        try:
            return sorted(p.glob(pattern))
        except OSError as exc:
            logger.error("Failed to glob %s with pattern '%s': %s", p, pattern, exc)
            raise

    @staticmethod
    def rglob(path: PathLike, pattern: str) -> List[Path]:
        p = Path(path)
        try:
            return sorted(p.rglob(pattern))
        except OSError as exc:
            logger.error("Failed to rglob %s with pattern '%s': %s", p, pattern, exc)
            raise

    @staticmethod
    def filter_files(paths: List[Path], extension: Optional[str] = None) -> List[Path]:
        result = [p for p in paths if p.is_file()]
        if extension is not None:
            ext = extension if extension.startswith(".") else f".{extension}"
            result = [p for p in result if p.suffix == ext]
        return result
