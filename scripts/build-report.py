"""Stage the supplied report archive without regenerating its edited content."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def stage(archive):
    destination = ROOT / 'build' / 'report'
    with zipfile.ZipFile(archive) as bundle:
        files = {}
        for entry in bundle.infolist():
            path = PurePosixPath(entry.filename)
            if (path.is_absolute() or '..' in path.parts or
                    '\\' in entry.filename or ':' in entry.filename or
                    not path.parts or path.parts[0] != 'report'):
                raise ValueError(f'Unexpected archive path: {entry.filename}')
            if entry.is_dir():
                continue
            relative = path.relative_to('report').as_posix()
            if relative in files:
                raise ValueError(f'Duplicate archive path: {entry.filename}')
            files[relative] = bundle.read(entry)
    for required in ('main.tex', 'main.pdf'):
        if required not in files:
            raise ValueError(f'Archive is missing report/{required}')
    if not files['main.pdf'].startswith(b'%PDF-'):
        raise ValueError('main.pdf is not a PDF')
    source = '\n'.join(data.decode('utf-8-sig') for name, data in files.items()
                       if name.endswith('.tex'))
    for name in re.findall(r'\\(?:input|include)\{([^}]+)\}', source):
        name = name if name.endswith('.tex') else name + '.tex'
        if name not in files:
            raise ValueError(f'Missing LaTeX input: {name}')
    for name in re.findall(r'\\includegraphics(?:\[[^]]*\])?\{([^}]+)\}', source):
        if not any(name + ext in files for ext in ('', '.pdf', '.png', '.jpg')):
            raise ValueError(f'Missing figure: {name}')
    refs = set(re.findall(r'\\ref\{([^}]+)\}', source))
    labels = set(re.findall(r'\\label\{([^}]+)\}', source))
    if refs - labels:
        raise ValueError(f'Unresolved references: {sorted(refs - labels)}')
    destination.mkdir(parents=True, exist_ok=True)
    for name, data in files.items():
        target = destination / name
        if not target.resolve().is_relative_to(destination.resolve()):
            raise ValueError(f'Output escapes report directory: {name}')
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    manifest = {
        'archive': str(archive.resolve()),
        'sha256': hashlib.sha256(archive.read_bytes()).hexdigest(),
        'files': {name: hashlib.sha256(data).hexdigest() for name, data in sorted(files.items())},
        'referencesChecked': len(refs),
    }
    (ROOT / 'build' / 'report-manifest.json').write_text(
        json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print(f'Staged {len(files)} files in {destination}; {len(refs)} references resolve.')
    if re.search(r'\\cite\w*\{', source) and not re.search(
            r'\\(?:bibliography|begin\{thebibliography\})', source):
        print('Source warning: the supplied report has citations but no bibliography declaration.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', nargs='?', type=Path, default=ROOT / 'Emergent_Complexity.zip')
    stage(parser.parse_args().archive)
