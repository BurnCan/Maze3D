#!/usr/bin/env python3
import argparse, pathlib, subprocess, sys
p=argparse.ArgumentParser(); p.add_argument('--clang-format', default='clang-format'); a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
files=[str(x) for folder in ('src','include','tests') for x in (root/folder).rglob('*') if x.suffix in ('.cpp','.h','.hpp','.c')]
result=subprocess.run([a.clang_format, '--dry-run', '--Werror', *files])
sys.exit(result.returncode)
