#!/usr/bin/env python3
"""Reject stale pre-Veometri product identifiers in project-owned text files."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
STALE = (
    "MeshSculpt", "Mesh Sculpt", "Mesh Sculpt Tool", "mesh_sculpt", "mesh-sculpt",
    "MESH_SCULPT_", "namespace mesh_sculpt", "mesh_sculpt::",
    "include/mesh_sculpt", "share/mesh-sculpt",
)
TEXT_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".cmake", ".json", ".md", ".py", ".txt", ".yml", ".yaml"}
SKIP_PARTS = {".git", "build", "_deps", "__pycache__"}
violations = []
for path in sorted(ROOT.rglob("*")):
    if not path.is_file() or any(part in SKIP_PARTS or part.startswith("build-") for part in path.parts):
        continue
    if path.name != "CMakeLists.txt" and path.suffix.lower() not in TEXT_SUFFIXES:
        continue
    relative = path.relative_to(ROOT).as_posix()
    # The audit's own pattern declarations are the sole intentional old-name references.
    if relative == "scripts/check_product_identity.py":
        continue
    for number, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
        if any(token in line for token in STALE):
            violations.append(f"{relative}:{number}: {line.strip()}")
if violations:
    print("Stale product identities:")
    print("\n".join(violations))
    sys.exit(1)
print(f"Product identity check passed: {ROOT}")
