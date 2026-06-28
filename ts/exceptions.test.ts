import assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { promisify } from 'node:util';
import { exec as execCallback } from 'node:child_process';
import { getPlatform } from '@node-3d/addon-tools';

const exec = promisify(execCallback);

const runAndGetError = async (name: string) => {
	let response = '';
	try {
		const { stderr, stdout } = await exec(
			`node --input-type=module -e "import('./dist/index.js').then((m) => m.${name}())"`,
		);
		response = stderr + stdout;
	} catch (error) {
		response = (error as Error).message;
	}
	return response;
};

describe('Exceptions', () => {
	it('reports segfaults', async () => {
		const response = await runAndGetError('causeSegfault');
		const exceptionName = getPlatform() === 'windows' ? 'ACCESS_VIOLATION' : 'SIGSEGV';
		assert.ok(response.includes(exceptionName));
	});

	// On Unix, the stacktrace is empty sometimes.
	if (['windows'].includes(getPlatform())) {
		it('shows symbol names in stacktrace', async () => {
			const response = await runAndGetError('causeSegfault');
			assert.match(response, /segfault::causeSegfault/u);
		});

		it('shows module names in stacktrace', async () => {
			const response = await runAndGetError('causeSegfault');
			assert.ok(response.includes('[node.exe]'));
			assert.ok(response.includes('[segfault.node]'));
		});
	}

	// On ARM this fails.
	if (['windows', 'linux'].includes(getPlatform())) {
		it('reports divisions by zero (int)', async () => {
			const response = await runAndGetError('causeDivisionInt');
			const exceptionName = getPlatform() === 'windows' ? 'INT_DIVIDE_BY_ZERO' : 'SIGFPE';
			assert.ok(response.includes(exceptionName));
		});
	}

	// On Unix, this hangs for some reason.
	if (['windows'].includes(getPlatform())) {
		it('reports stack overflows', async () => {
			const response = await runAndGetError('causeOverflow');
			const exceptionName = getPlatform() === 'windows' ? 'STACK_OVERFLOW' : 'SIGSEGV';
			assert.ok(response.includes(exceptionName));
		});
	}

	it('reports illegal operations', async () => {
		const response = await runAndGetError('causeIllegal');
		const exceptionName = getPlatform() === 'windows' ? 'ILLEGAL_INSTRUCTION' : 'SIGILL';
		assert.ok(response.includes(exceptionName));
	});
});
