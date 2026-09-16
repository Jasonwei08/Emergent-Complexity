import json,sys,math,statistics,collections
from pathlib import Path
sys.path.insert(0,str(Path('build/report-tools').resolve()))
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
ROOT=Path('report');FIG=ROOT/'figures';FIG.mkdir(parents=True,exist_ok=True)
s=json.loads(Path('results/v2/summary.json').read_text())
plt.rcParams.update({'font.size':9,'axes.spines.top':False,'axes.spines.right':False,'savefig.dpi':180})
def tex(v): return str(v).replace('_',r'\_').replace('%',r'\%').replace('&',r'\&')
def table(headers,rows,caption,label):
 return '\\begin{table}[htbp]\n\\centering\\small\n\\begin{tabular}{'+('l'+'r'*(len(headers)-1))+'}\n\\toprule\n'+' & '.join(headers)+r' \\'+'\n\\midrule\n'+'\n'.join(' & '.join(tex(v) for v in row)+r' \\' for row in rows)+'\n\\bottomrule\n\\end{tabular}\n\\caption{'+caption+'}\\label{'+label+'}\n\\end{table}\n'
def fig(name,caption,label): return '\\begin{figure}[htbp]\n\\centering\\includegraphics[width=.98\\linewidth]{figures/'+name+'}\n\\caption{'+caption+'}\\label{'+label+'}\n\\end{figure}\n'
def write(name,text): (ROOT/name).write_text(text,encoding='ascii')
def counts(rows):return dict(collections.Counter(r['category'] for r in rows))
cats=counts(s['sample']['rows'])
text='The completed v2 campaign contains 2880 trials: 1500 random-sample trials, 30 separate controls, 450 binary follow-up/noise trials, and 900 Generations trials. The respective measured wall runtimes were '+', '.join(f"{s[k]['runtimeSeconds']:.1f} s" for k in ['sample','controls','detail','generations'])+'. Runtime is environment-dependent and excludes report generation. All used master seed 2026 and reached 500 updates.\n'
text+=table(['Category','Trials','Percent'],[(k,v,f'{v/15:.2f}') for k,v in cats.items()],'Random sample only; 100 rules times three densities times five trials. Extinction, growth and unclassified counts were zero. Labels follow the stated priority.','tab:sample')
text+='No sampled trial was classified extinct. This sample-specific absence does not establish persistence for other seeds, sizes or rules. '+str(sum(len(r['categories'])>1 for r in s['ranked']))+' sampled rules had more than one observed category. Spatial expansion was not detected by the conservative bounding-box criterion; random initial grids already occupied most of the available spatial extent.\n'
fig1,ax=plt.subplots(figsize=(7,3.2));ax.barh(list(cats),list(cats.values()),color='#527d68');ax.set_xlabel('Number of trials (sample only, n=1500)');fig1.tight_layout();fig1.savefig(FIG/'sample-categories.png');plt.close(fig1)
text+=fig('sample-categories.png','Measured classification distribution. Categories describe trajectories, not intrinsic rule classes.','fig:categories')
rows=[]
for rule in ['B2/S','B3/S23']:
 for c in [3,5,8]:
  for init in ['active-only','all-states']:
   rr=[r for r in s['generations']['rows'] if r['rule']==rule and r['states']==c and r['initialization']==init and r['noise']==0]
   rows.append([rule,c,init,f"{statistics.mean(r['finalDensity'] for r in rr):.4f}",f"{statistics.mean(r['finalOccupied'] for r in rr):.4f}",sum(r['category']=='extinction' for r in rr)])
text+=table(['Rule','C','Initializer','Active','Nonzero','Extinct / 15'],rows,'Zero-noise Generations outcomes pooled equally across the three densities (five trials per density). Full per-density outcomes remain in the JSON.','tab:generations')
text+='The active and total nonzero columns separate activity from refractory occupancy. Differences between C values and B/S rules concern state dynamics, not merely color. The pooled results should be read alongside the density-specific noise plots; initialization changes expected starting active density.\n'
write('measured-results.tex',text)
sel=s['selected'];rank=s['ranked'][0];rr=[r for r in s['detail']['rows'] if r['rule']==sel and r['noise']==0]
text=f'The highest-ranked sampled candidate was {sel}, with interest score {rank["score"]:.4f}. Its original 15 sample trials had the distribution '+', '.join(f'{v} {k}' for k,v in rank['categories'].items())+'. It was selected for mixed observed behavior and nontrivial recurrence, not biological resemblance. B0 also allows spontaneous activation from empty regions.\n'
rows=[]
for d in [.1,.3,.5]:
 group=[r for r in rr if r['initialDensity']==d];rows.append([d,f"{statistics.mean(r['finalDensity'] for r in group):.4f}",f"{min(r['finalDensity'] for r in group):.4f}--{max(r['finalDensity'] for r in group):.4f}",'; '.join(f'{v} '+('persistent' if k=='bounded persistent activity' else k) for k,v in counts(group).items())])
text+=table(['Density','Mean final','Final range','Observed categories'],rows,'Selected-rule zero-noise follow-up: ten trials per density. The category distribution reports both common and exceptional outcomes.','tab:selected')
text+='The additional trials give '+', '.join(f'{v} {k}' for k,v in counts(rr).items())+'. Terminal periods across these runs were '+', '.join(str(v) for v in sorted(set(r['terminalPeriod'] for r in rr)))+'. Period zero means no recurrence in the 16-step terminal window. Exact repeating grids support a cyclic structure claim for the deterministic recurrent trials; no translating object or reproduction claim follows.\n'
# Images from exact saved initial/final cells and activity traces.
fig2,axs=plt.subplots(2,3,figsize=(7.2,4.7))
for j,d in enumerate([.1,.3,.5]):
 r=json.loads(Path(f'results/v2/replay-{d}-0.json').read_text())['results'][0]
 for row,key in enumerate(['initial','final']):
  grid=np.array(r[key]).reshape(64,64)>0;axs[row,j].imshow(grid,cmap='Greys',vmin=0,vmax=1,interpolation='nearest');axs[row,j].set_title(f'd={d}, t={0 if row==0 else 500}');axs[row,j].axis('off')
fig2.tight_layout();fig2.savefig(FIG/'selected-grids.png');plt.close(fig2)
text+=fig('selected-grids.png',f'Actual {sel} semantic grids, trial 0 at each density, p=0. Top: initialization; bottom: generation 500. Exact derived seeds are in the replay files.','fig:selected')
text+=r'\IfFileExists{figures/application-selected-100.png}{\begin{figure}[htbp]\centering\includegraphics[width=.9\linewidth]{figures/application-selected-100.png}\caption{Working application screenshot after 100 exact replay steps of the selected sampled rule, density 0.30, trial 0, zero noise.}\label{fig:application}\end{figure}}{}'+'\n'
write('selected-results.tex',text)
# Repeated noise statistics, by density; error bars are across trials, not proof of threshold.
fig3,axs=plt.subplots(1,3,figsize=(8,3.2),sharey=True)
levels=[0,.00001,.0001,.001,.01]
for ax,rule in zip(axs,[sel,'B3/S23','B36/S23']):
 for d in [.1,.3,.5]:
  groups=[[r['finalDensity'] for r in s['detail']['rows'] if r['rule']==rule and r['initialDensity']==d and r['noise']==p] for p in levels]
  ax.errorbar(range(5),[statistics.mean(g) for g in groups],yerr=[statistics.stdev(g) for g in groups],marker='o',markersize=3,label=f'd={d}',capsize=2)
 ax.set_title(rule);ax.set_xticks(range(5),['0','1e-5','1e-4','1e-3','1e-2'],rotation=40);ax.set_xlabel('p');ax.set_ylim(-.03,1.03)
axs[0].set_ylabel('Final active density');axs[-1].legend(fontsize=7);fig3.tight_layout();fig3.savefig(FIG/'binary-noise.png');plt.close(fig3)
text=fig('binary-noise.png','Binary repeated-trial noise response. Points are means of ten trials per density and p; error bars are one sample standard deviation. Discrete levels are equally spaced for legibility.','fig:noise')
rows=[]
for p in levels:
 group=[r for r in s['detail']['rows'] if r['rule']==sel and r['noise']==p]
 rows.append([f'{p:g}',f"{statistics.mean(r['finalDensity'] for r in group):.4f}",f"{statistics.stdev(r['finalDensity'] for r in group):.4f}",f"{statistics.mean(r['finalDistance'] for r in group):.4f}",sum(r['finalDensity']>0 for r in group)])
text+=table(['p','Mean density','SD','Distance','Active / 30'],rows,'Selected-rule noise outcomes pooled across densities. SD includes between-density variation; the plot separates densities. Distance is final disagreement from the matched deterministic grid.','tab:noise')
fig4,axs=plt.subplots(2,3,figsize=(8,5),sharey=True)
for i,rule in enumerate(['B2/S','B3/S23']):
 for j,c in enumerate([3,5,8]):
  ax=axs[i,j]
  for init,style in [('active-only','-'),('all-states','--')]:
   for d,color in zip([.1,.3,.5],['#3274a1','#e1812c','#3a923a']):
    groups=[[r['finalDensity'] for r in s['generations']['rows'] if r['rule']==rule and r['states']==c and r['initialization']==init and r['initialDensity']==d and r['noise']==p] for p in levels]
    ax.plot(range(5),[statistics.mean(g) for g in groups],style,color=color,label=f'{d}, {init}')
  ax.set_title(f'{rule}/C{c}');ax.set_xticks(range(5),['0','1e-5','1e-4','1e-3','1e-2'],rotation=40)
  if j==0:ax.set_ylabel('Final active density')
axs[0,2].legend(fontsize=6);fig4.tight_layout();fig4.savefig(FIG/'generations-noise.png');plt.close(fig4)
text+=fig('generations-noise.png','Generations response: means of five trials per point. Colors denote initial nonzero density; solid lines use active-only initialization and dashed lines all-states. Raw trial variation and per-state populations are exported. Lines guide comparison and do not locate a critical probability.','fig:multi-noise')
# actual quantitative differences
for rule in ['B2/S','B3/S23']:
 group=[r for r in s['generations']['rows'] if r['rule']==rule]
 zero=[r for r in group if r['noise']==0];hi=[r for r in group if r['noise']==.01]
 text+=f'For {rule} pooled across C, initialization and density, the mean final active density was {statistics.mean(r["finalDensity"] for r in zero):.4f} at p=0 and {statistics.mean(r["finalDensity"] for r in hi):.4f} at p=0.01. Zero-noise active extinction occurred in {sum(r["category"]=="extinction" for r in zero)} of {len(zero)} runs. Pooling summarizes this design only; it does not isolate a causal C effect.\n'
write('noise-results.tex',text)
# validate every full export, paired grids, per-state conservation and mask encoding
validated=0
for name in ['sampled','controls','detail','generations']:
 data=json.loads(Path(f'results/v2/{name}.json').read_text());assert data['complete']
 for r in data['results']:
  assert r['schema']=='life-trial-v2' and r['boundary']=='toroidal' and r['version']==2
  assert r['elapsedGenerations']==500 and r['extinct']==(r['category']=='extinction')
  for key in ['density','occupied','change','distance','populations','births','deaths','hashes','spread','movement']:assert len(r[key])==501
  assert all(len(p)==r['states'] and sum(p)==4096 for p in r['populations'])
  assert all(p[1]/4096==d for p,d in zip(r['populations'],r['density']))
  b,sv=r['rule'].split('/');assert sum(1<<int(v) for v in b[1:])==r['birthMask'];assert sum(1<<int(v) for v in sv[1:])==r['survivalMask']
  assert len(r['initial'])==len(r['final'])==4096
  validated+=1
 if name=='sampled':
  paired={}
  for r in data['results']:
   key=(r['initialDensity'],r['trial']);assert paired.setdefault(key,r['initial'])==r['initial']
print(f'Generated measured LaTeX sections; validated {validated} complete trial schemas and paired sampled grids.')

# Separate every semantic population in actual multi-state traces.
data=json.loads(Path('results/v2/generations.json').read_text())
fig5,axes=plt.subplots(1,2,figsize=(8,3.1))
for ax,rule in zip(axes,['B2/S','B3/S23']):
 r=next(r for r in data['results'] if r['rule']==rule and r['states']==5 and r['initialization']=='active-only' and r['initialDensity']==.3 and r['noise']==0 and r['trial']==0)
 population=np.array(r['populations'])/4096
 for state in range(5):ax.plot(range(501),population[:,state],label=f'state {state}',linestyle=['-','-','--',':','-.'][state])
 ax.set_title(f'{rule}/C5; seed {r["seed"]}');ax.set_xlabel('Generation');ax.set_ylim(0,1)
axes[0].set_ylabel('State population / 4096');axes[1].legend(fontsize=7);fig5.tight_layout();fig5.savefig(FIG/'state-populations.png');plt.close(fig5)
with (ROOT/'measured-results.tex').open('a',encoding='ascii') as f:
 f.write(fig('state-populations.png','Measured per-state density histories for trial 0, nonzero density 0.30, active-only initialization and zero noise. Dead state 0 is included; these traces distinguish active persistence from refractory decay.','fig:state-populations'))
 for rule in ['B2/S','B3/S23']:
  observed=counts([r for r in s['generations']['rows'] if r['rule']==rule and r['noise']==0])
  f.write('For zero-noise '+rule+' across C and initializers, the measured categories were '+', '.join(f'{v} {k}' for k,v in observed.items())+'.\n')
 f.write('Exact fixed points and recurrences support grid-level stable or cyclic structure claims. No traveling or wave-like object was established by object tracking; refractory population changes alone are not counted as emergent structure.\n')
