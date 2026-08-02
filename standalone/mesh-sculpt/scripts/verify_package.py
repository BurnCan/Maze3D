#!/usr/bin/env python3
import argparse, pathlib, subprocess, tarfile, zipfile, sys
p=argparse.ArgumentParser(); p.add_argument('build'); p.add_argument('--config',default='Release'); a=p.parse_args(); build=pathlib.Path(a.build).resolve()
subprocess.run(['cpack','-C',a.config],cwd=build,check=True)
archives=list(build.glob('MeshSculpt-*.tar.gz'))+list(build.glob('MeshSculpt-*.zip'))
if not archives: print('No archive packages generated'); sys.exit(1)
for archive in archives:
 names=tarfile.open(archive).getnames() if archive.name.endswith('.tar.gz') else zipfile.ZipFile(archive).namelist()
 required=('mesh_sculpt','basic.vert','README.md','LICENSE_STATUS.md','THIRD_PARTY_NOTICES.md')
 missing=[item for item in required if not any(item in name for name in names)]
 if missing: print(archive, 'missing', missing); sys.exit(1)
 print(archive.name, *names, sep='\n  ')
