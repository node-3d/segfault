import assert from 'node:assert/strict';
import { describe, it, after } from 'node:test';
import fs from 'node:fs/promises';
import { promisify } from 'node:util';
import { exec as execCallback } from 'node:child_process';
import { exists } from '@node-3d/addon-tools';

const exec = promisify(execCallback);

const PATH_STD = `${import.meta.dirname.replaceAll('\\', '/')}/../segfault.log`;
const PATH_CUSTOM = `${import.meta.dirname.replaceAll('\\', '/')}/../custom.log`;

const rm = async (path: string) => {
	await fs.rm(path, { force: true, maxRetries: 20, retryDelay: 100 });
};

const causeSegfault = async (isCustom = false) => {
	try {
		if (isCustom) {
			await exec([
				'node --input-type=module -e',
				`"import('./dist/index.js').then((m) => { m.setLogPath('${PATH_CUSTOM}'); m.causeSegfault(); })"`,
			].join(' '));
			return;
		}
		await exec('node --input-type=module -e "import(\'./dist/index.js\').then((m) => m.causeSegfault())"');
	} catch {
		// The subprocess is expected to crash.
	}
};

describe('Logging', async () => {
	await rm(PATH_STD);
	await rm(PATH_CUSTOM);

	after(async () => {
		await rm(PATH_STD);
		await rm(PATH_CUSTOM);
	});

	await it('does not write log if there is no file', async () => {
		await causeSegfault();

		const isLogWritten = await exists(PATH_STD);
		assert.ok(!isLogWritten);
	});

	it('writes standard log if it exists', async () => {
		await fs.writeFile(PATH_STD, 'test\n');
		await causeSegfault();

		const content = await fs.readFile(PATH_STD, 'utf8');
		assert.ok(content.length > 10);
	});

	it('does not write custom log if there is no file', async () => {
		await rm(PATH_STD);
		await causeSegfault(true);

		const isLogWritten = await exists(PATH_STD);
		const isLogWrittenCustom = await exists(PATH_CUSTOM);
		assert.ok(!isLogWritten);
		assert.ok(!isLogWrittenCustom);
	});

	it('writes custom log if it exists', async () => {
		await rm(PATH_STD);
		await fs.writeFile(PATH_CUSTOM, 'test\n');
		await causeSegfault(true);

		const isLogWritten = await exists(PATH_STD);
		assert.ok(!isLogWritten);

		const content = await fs.readFile(PATH_CUSTOM, 'utf8');
		assert.ok(content.length > 10);
	});
});
