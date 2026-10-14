"""Flash only explicit firmware regions, preserving NVS and PHY. No whole-chip erase."""
import argparse, json, hashlib, subprocess, sys
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('--port',required=True)
parser.add_argument('--check',action='store_true',help='Validate images and print the planned offsets without accessing a device')
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
manifest=json.loads((root/'firmware-images/manifest.json').read_text())
files=[]
for image in manifest['images']:
    target=(root/'firmware-images'/image['file']).resolve()
    if not target.is_relative_to((root/'firmware-images').resolve()):raise SystemExit('Image path is outside the package')
    if hashlib.sha256(target.read_bytes()).hexdigest()!=image['sha256']:raise SystemExit('Image checksum mismatch')
    files += [image['offset'],str(target)]
command=[sys.executable,'-m','esptool','--chip',manifest['chip'],'--port',args.port,'--no-stub','--before','default-reset','--after','hard-reset',
         'write-flash','--flash-mode','dio','--flash-freq','80m','--flash-size','16MB',*files]
if args.check:
    print('Verified four images; NVS 0x9000-0xefff and PHY 0xf000-0xffff are not written.')
    print(' '.join(command));sys.exit(0)
subprocess.run(command,check=True)
