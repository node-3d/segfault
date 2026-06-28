import fs from 'node:fs/promises';
import { ensuredir } from '@node-3d/addon-tools';
import { setLogPath, causeSegfault } from '@node-3d/segfault';

await ensuredir('c:/tmp');
await fs.writeFile('c:/tmp/segfault.txt', '');

setLogPath('c:/tmp/segfault.txt');
causeSegfault();
