#!/usr/bin/env python3
"""Copy, configure, build, and test the project away from its source tree."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--generator')
parser.add_argument('--config', default='Debug')
parser.add_argument('--keep-temp', action='store_true')
parser.add_argument('--build-parallel', nargs='?', const='')
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
temp = Path(tempfile.mkdtemp(prefix='veometri-clean-'))
project = temp / 'project'
def ignored(_path, names):
    reject = {'.git', '.idea', '.vscode', '__pycache__', 'CMakeFiles', '_deps'}
    return {name for name in names if name in reject or name.startswith('build') or name.endswith(('.pyc', '.meshgeo.tmp'))}
def run(command):
    print('+', ' '.join(command), flush=True)
    subprocess.run(command, check=True)
try:
    shutil.copytree(root, project, ignore=ignored)
    configure = ['cmake', '-S', str(project), '-B', str(temp / 'build'), '-DBUILD_TESTING=ON']
    if args.generator: configure += ['-G', args.generator]
    run(configure)
    build = ['cmake', '--build', str(temp / 'build'), '--config', args.config]
    if args.build_parallel is not None: build += ['--parallel'] + ([args.build_parallel] if args.build_parallel else [])
    run(build)
    run(['ctest', '--test-dir', str(temp / 'build'), '-C', args.config, '--output-on-failure'])
    print('Clean-copy verification passed.')
except subprocess.CalledProcessError as error:
    print(f'Clean-copy verification failed with exit code {error.returncode}; temporary tree: {temp}')
    raise SystemExit(error.returncode)
finally:
    if args.keep_temp: print(f'Kept temporary tree: {temp}')
    else: shutil.rmtree(temp, ignore_errors=True)
