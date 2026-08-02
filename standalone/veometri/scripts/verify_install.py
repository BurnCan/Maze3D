#!/usr/bin/env python3
import argparse, pathlib, subprocess, tempfile, shutil, sys
p=argparse.ArgumentParser(); p.add_argument('build'); p.add_argument('--config', default='Release'); a=p.parse_args()
prefix=pathlib.Path(tempfile.mkdtemp(prefix='veometri-install-'))
try:
 subprocess.run(['cmake','--install',a.build,'--prefix',str(prefix),'--config',a.config],check=True)
 files=[x.relative_to(prefix).as_posix() for x in prefix.rglob('*') if x.is_file()]
 required=['basic.vert','basic.frag','README.md','LICENSE_STATUS.md','THIRD_PARTY_NOTICES.md']
 missing=[x for x in required if not any(f.endswith(x) for f in files)]
 if not any(pathlib.Path(f).name in ('veometri','veometri.exe') for f in files): missing.append('veometri executable')
 if missing: print('Missing installed files:', ', '.join(missing)); sys.exit(1)
 source=str(pathlib.Path(__file__).resolve().parents[1])
 for f in files:
  path=prefix/f
  if path.suffix in ('.txt','.md','.cmake','.json') and source in path.read_text(errors='ignore'):
   print('Source path embedded in',f); sys.exit(1)
 print('Install verification passed:', *files, sep='\n  ')
finally: shutil.rmtree(prefix,ignore_errors=True)
