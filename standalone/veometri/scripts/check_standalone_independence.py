#!/usr/bin/env python3
"""Reject source/build coupling outside the standalone project."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXTENSIONS = {'.cpp', '.h', '.hpp', '.c', '.cmake', '.md', '.json', '.yml', '.yaml', '.sh', '.ps1'}
PATTERNS = [r'maze_engine', r'MAZE3D_', r'\$\{CMAKE_SOURCE_DIR\}/(?:engine|app|tools)',
            r'\.\./(?:\.\./)?(?:engine|app|tools)/', r'tools/veometri/']
# Exact documentation/transition artifacts only; executable build configuration is never exempted.
EXEMPT = {
    ('README.md', 'BurnCan/Maze3D'),
    ('docs/REPOSITORY_MIGRATION.md', 'Maze3D'),
    ('ci/veometri-ci.yml', 'cmake -S standalone/veometri'),
}
violations = []
for path in sorted(ROOT.rglob('*')):
    if not path.is_file() or any(part in {'.git', 'build', '_deps', '__pycache__'} for part in path.parts):
        continue
    if path.name != 'CMakeLists.txt' and path.suffix.lower() not in EXTENSIONS:
        continue
    relative = path.relative_to(ROOT).as_posix()
    if relative == 'scripts/check_standalone_independence.py':
        continue
    for number, line in enumerate(path.read_text(encoding='utf-8', errors='replace').splitlines(), 1):
        if any(relative == file and token in line for file, token in EXEMPT):
            continue
        for pattern in PATTERNS:
            if re.search(pattern, line, re.IGNORECASE if pattern == 'maze_engine' else 0):
                violations.append(f'{relative}:{number}: {line.strip()}')
if violations:
    print('Standalone independence violations:')
    print('\n'.join(violations))
    sys.exit(1)
print(f'Standalone independence check passed: {ROOT}')
