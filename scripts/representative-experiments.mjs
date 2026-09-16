import {mkdirSync,writeFileSync,openSync,writeSync,closeSync} from 'node:fs';
import createLifeModule from '../life_engine.js';
const Life=await createLifeModule();
mkdirSync('results/v2',{recursive:true});
const master=2026, densities=[.1,.3,.5], noiseLevels=[0,.00001,.0001,.001,.01];
const seedFor=(density,trial)=>(master+Math.imul(trial+1,2654435761)+Math.imul(Math.round(density*1000000),2246822519))>>>0;
const compact=r=>Object.fromEntries(Object.entries(r).filter(([k,v])=>!Array.isArray(v)));
function batch(name,specs,trials){
 const config={masterSeed:master,cols:64,rows:64,steps:500,trials,densities,boundary:'toroidal',seedDerivation:'(master + (trial+1)*2654435761 + round(density*1e6)*2246822519) modulo 2^32; rule index intentionally excluded for paired grids',noiseModel:'post-update binary flip; Generations uniformly different state',initialization:'density is nonzero probability; all-states distributes over 1..C-1',measurementInterval:1};
 const fd=openSync(`results/v2/${name}.json`,'w');writeSync(fd,JSON.stringify({schema:'life-batch-v2',engine:'life-lab-2',config,specs}).slice(0,-1)+',"results":[');
 const rows=[];let first=true;const start=Date.now();
 for(const [index,spec] of specs.entries()){
  for(const density of densities)for(const noise of spec.noiseLevels||[0])for(let trial=0;trial<trials;trial++){
   const seed=seedFor(density,trial);const r=JSON.parse(Life.runTrialAll(spec.rule,spec.states||2,noise,64,64,density,500,seed,!!spec.allStates));
   Object.assign(r,{trial,ruleIndex:index,control:!!spec.control});
   if(!first)writeSync(fd,',');first=false;writeSync(fd,JSON.stringify(r));
   const row=compact(r);row.finalOccupied=r.occupied.at(-1);row.finalDistance=r.distance.at(-1);row.finalPopulations=r.populations.at(-1);rows.push(row);
   if(name==='detail' && index===0 && noise===0 && trial<3) writeFileSync(`results/v2/replay-${density}-${trial}.json`,JSON.stringify({schema:'life-batch-v2',config,results:[r],rules:[r.rule],complete:true}));
  }
  if(index%10===0) console.log(`${name}: ${index+1}/${specs.length} specifications, ${rows.length} trials, ${((Date.now()-start)/1000).toFixed(1)}s`);
 }
 writeSync(fd,'],"complete":true}');closeSync(fd);return {config,rows,runtimeSeconds:(Date.now()-start)/1000};
}
const rules=Life.sampleRules(master);writeFileSync('results/v2/sampled-rules.json',JSON.stringify({masterSeed:master,rules},null,2));
const sample=batch('sampled',rules.map(rule=>({rule})),5);
const controls=batch('controls',['B3/S23','B36/S23'].filter(r=>!rules.includes(r)).map(rule=>({rule,control:true})),5);
const ranked=rules.map(rule=>{
 const rows=sample.rows.filter(r=>r.rule===rule),categories={};rows.forEach(r=>categories[r.category]=(categories[r.category]||0)+1);
 const mean=f=>rows.reduce((s,r)=>s+f(r),0)/rows.length;
 const score=mean(r=>(r.finalDensity>0&&r.finalDensity<.9?1:0)+(r.terminalPeriod>2?1:0)+Math.max(0,r.spatialGrowth)+Math.min(1,r.mobility*20)+4*r.tailChange*(1-r.tailChange))+(Object.keys(categories).length-1)/6;
 return {rule,score,categories,dominantObservedCategory:Object.entries(categories).sort((a,b)=>b[1]-a[1])[0][0]};
}).sort((a,b)=>b.score-a.score||a.rule.localeCompare(b.rule));
const selected=ranked[0].rule;
const detail=batch('detail',[selected,'B3/S23','B36/S23'].map(rule=>({rule,noiseLevels})),10);
const generations=batch('generations',['B2/S','B3/S23'].flatMap(rule=>[3,5,8].flatMap(states=>[false,true].map(allStates=>({rule,states,allStates,noiseLevels})))),5);
const summary={schema:'life-summary-v2',masterSeed:master,selected,interestFormula:'mean(persistence without >=.9 saturation + period>2 + positive spatialGrowth + min(1,20*mobility) + 4*a*(1-a)) + (categoryCount-1)/6',ranked,sample,controls,detail,generations};
writeFileSync('results/v2/summary.json',JSON.stringify(summary,null,2));
console.log(JSON.stringify({selected,trials:[sample,controls,detail,generations].map(b=>b.rows.length),seconds:[sample,controls,detail,generations].map(b=>b.runtimeSeconds)}));
