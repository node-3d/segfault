import assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { promisify } from 'node:util';
import { exec as execCallback } from 'node:child_process';
import type { ExecException } from 'node:child_process';

const exec = promisify(execCallback);
const isWindows = process.platform === 'win32';

type ExecFailure = ExecException & {
	stdout?: string | Buffer;
	stderr?: string | Buffer;
};

const stringifyOutput = (value: string | Buffer | undefined) => {
	if (Buffer.isBuffer(value)) {
		return value.toString('utf8');
	}
	return value ?? '';
};

const formatOutput = (label: string, value: string) => `${label}:\n${value || '<empty>'}`;

const formatExecFailure = (command: string, error: unknown) => {
	if (!(error instanceof Error)) {
		return [`command: ${command}`, `thrown: ${String(error)}`].join('\n');
	}

	const failure = error as ExecFailure;
	const code = failure.code === undefined ? '<unset>' : String(failure.code);
	const signal =
		failure.signal === undefined || failure.signal === null ? '<unset>' : failure.signal;

	return [
		`command: ${command}`,
		`message: ${failure.message}`,
		`code: ${code}`,
		`signal: ${signal}`,
		formatOutput('stdout', stringifyOutput(failure.stdout)),
		formatOutput('stderr', stringifyOutput(failure.stderr)),
	].join('\n');
};

const runAndGetError = async (name: string) => {
	const command = `node --input-type=module -e "import('./ts/index.ts').then((m) => m.${name}())"`;
	try {
		const { stderr, stdout } = await exec(command);
		return [
			`command: ${command}`,
			'status: exited normally',
			formatOutput('stdout', stdout),
			formatOutput('stderr', stderr),
		].join('\n');
	} catch (error) {
		return formatExecFailure(command, error);
	}
};

const assertIncludes = (response: string, expected: string, action: string) => {
	assert.ok(
		response.includes(expected),
		[`Expected ${action} output to include "${expected}".`, response].join('\n\n'),
	);
};

describe('Exceptions', () => {
	it('reports segfaults', async () => {
		const response = await runAndGetError('causeSegfault');
		const exceptionName = isWindows ? 'ACCESS_VIOLATION' : 'SIGSEGV';
		assertIncludes(response, exceptionName, 'causeSegfault');
	});

	// On Unix, the stacktrace is empty sometimes.
	if (isWindows) {
		it('shows symbol names in stacktrace', async () => {
			const response = await runAndGetError('causeSegfault');
			assert.match(response, /segfault::causeSegfault/u, response);
		});

		it('shows module names in stacktrace', async () => {
			const response = await runAndGetError('causeSegfault');
			assertIncludes(response, '[node.exe]', 'causeSegfault');
			assertIncludes(response, '[segfault.node]', 'causeSegfault');
		});
	}

	// On ARM this fails.
	if (['win32', 'linux'].includes(process.platform)) {
		it('reports divisions by zero (int)', async () => {
			const response = await runAndGetError('causeDivisionInt');
			const exceptionName = isWindows ? 'INT_DIVIDE_BY_ZERO' : 'SIGFPE';
			assertIncludes(response, exceptionName, 'causeDivisionInt');
		});
	}

	// On Unix, this hangs for some reason.
	if (isWindows) {
		it('reports stack overflows', async () => {
			const response = await runAndGetError('causeOverflow');
			const exceptionName = isWindows ? 'STACK_OVERFLOW' : 'SIGSEGV';
			assertIncludes(response, exceptionName, 'causeOverflow');
		});
	}

	it('reports illegal operations', async () => {
		const response = await runAndGetError('causeIllegal');
		const exceptionName = isWindows ? 'ILLEGAL_INSTRUCTION' : 'SIGILL';
		assertIncludes(response, exceptionName, 'causeIllegal');
	});
});
