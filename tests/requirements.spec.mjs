import {test,expect} from '@playwright/test';
import {mkdirSync,readFileSync} from 'node:fs';
test('new presets, all-state initialization and saved experiment import',async({page,baseURL},testInfo)=>{
 await page.goto(baseURL);await page.locator('#generations-life').click();
 await expect(page.locator('#states')).toHaveValue('5');
 await page.locator('#initialization').selectOption('all');await page.locator('#random').click();
 await page.locator('#grid').focus();await page.keyboard.press('ArrowRight');
 await page.locator('#highlife').click();await expect(page.locator('#states')).toHaveValue('2');
 await page.locator('#rule-text').fill('B2/S/C3');await page.locator('#rule-form button').click();
 await expect(page.locator('#states')).toHaveValue('3');
 const summary=JSON.parse(readFileSync('results/v2/summary.json','utf8'));
 await page.locator('#import-experiment').setInputFiles('results/v2/replay-0.3-0.json');
 await expect(page.locator('#experiment-status')).toContainText('Opened 1 saved trials');
 await page.locator('#replay').click();await expect(page.locator('#rule-text')).toHaveValue(summary.selected);
 if(testInfo.project.name==='repository-subpath') {
  mkdirSync('report/figures',{recursive:true});
  await page.locator('#grid').screenshot({path:'report/figures/application-selected-initial.png'});
  for(let i=0;i<100;i++)await page.locator('#step').click();
  await page.locator('#grid').screenshot({path:'report/figures/application-selected-100.png'});
  await page.screenshot({path:'report/figures/application.png',fullPage:true});
 }
});
