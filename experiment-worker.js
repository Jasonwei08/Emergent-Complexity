import createLifeModule from "./life_engine.js";
// Register the listener before awaiting Wasm so the first browser message is not lost.
let modulePromise;
self.onmessage = async ({ data: { config: c, rules, controls = [] } }) => {
  try {
    const Life = await (modulePromise ??= createLifeModule());
    const total = (rules.length+controls.length) * (c.densities?.length || 1) * c.trials * c.noiseLevels.length;
    for (const rule of [...rules,...controls])
      for (const density of c.densities || [c.density])
      for (const noise of c.noiseLevels)
        for (let trial = 0; trial < c.trials; trial++) {
          // Same initial and noise seeds across rules/levels: paired comparisons.
          const seed = (c.seed + Math.imul(trial + 1, 2654435761) + Math.imul(Math.round(density*1000000),2246822519)) >>> 0;
          const result = JSON.parse(
            Life.runTrialAll(
              rule,
              c.states,
              noise,
              c.cols,
              c.rows,
              density,
              c.steps,
              seed,
              Boolean(c.allStates),
            ),
          );
          result.trial = trial; result.control=controls.includes(rule); result.ruleIndex=rules.indexOf(rule);
          self.postMessage({ result, total });
        }
    self.postMessage({ done: true });
  } catch (error) {
    self.postMessage({ error: String(error) });
  }
};
