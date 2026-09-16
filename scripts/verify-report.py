import sys,re,json,argparse
from pathlib import Path
sys.path.insert(0,str(Path('build/report-tools').resolve()))
sys.path.insert(0,str(Path('build/report-python').resolve()))
import pymupdf as fitz
from PIL import Image,ImageOps,ImageDraw
parser=argparse.ArgumentParser(description='Render and verify a staged or source report.')
parser.add_argument('--report-dir',type=Path,default=Path('build/report') if Path('build/report/main.pdf').exists() else Path('report'))
parser.add_argument('--output-dir',type=Path,default=Path('build/report-review'))
args=parser.parse_args()
root=args.report_dir;out=args.output_dir;out.mkdir(parents=True,exist_ok=True)
doc=fitz.open(root/'main.pdf');thumbs=[];texts=[]
for i,page in enumerate(doc):
 pix=page.get_pixmap(matrix=fitz.Matrix(1.2,1.2));pix.save(out/f'page-{i+1}.png')
 im=Image.open(out/f'page-{i+1}.png');im.thumbnail((306,400));thumb=Image.new('RGB',(326,430),'#d9d9d9');thumb.paste(im,((326-im.width)//2,20));ImageDraw.Draw(thumb).text((10,410),str(i+1),fill='black');thumbs.append(thumb);texts.append(page.get_text())
for start in range(0,len(thumbs),6):
 sheet=Image.new('RGB',(978,860),'white')
 for j,im in enumerate(thumbs[start:start+6]):sheet.paste(im,((j%3)*326,(j//3)*430))
 sheet.save(out/f'contact-{start//6+1}.png')
source='\n'.join(p.read_text(encoding='utf-8') for p in root.glob('*.tex'))
refs=set(re.findall(r'\\ref\{([^}]+)\}',source));labels=set(re.findall(r'\\label\{([^}]+)\}',source));assert refs<=labels,refs-labels
for file in re.findall(r'\\includegraphics(?:\[[^]]*\])?\{([^}]+)\}',source):assert (root/file).exists(),file
assert not re.search(r'\\textbf\{TODO:',source)
assert all(text.strip() for text in texts)
title=re.search(r'\\title\{([^}]+)\}',(root/'main.tex').read_text(encoding='utf-8'))
normalize=lambda value: re.sub(r'[^a-z0-9]','',value.lower())
title_matches=not title or normalize(title.group(1)) in normalize(texts[0])
if not title_matches:
 print('Source warning: the supplied PDF title differs from main.tex; recompile to publish the edited report.')
(out/'verification.json').write_text(json.dumps({'pages':len(doc),'figures':len(re.findall(r'\\includegraphics',source)),'referencesChecked':len(refs),'placeholders':0,'allPagesHaveText':True,'pdfTitleMatchesSource':title_matches},indent=2))
print(f'{len(doc)} pages rendered; {len(refs)} figure/table references resolve; no TODO placeholders.')
