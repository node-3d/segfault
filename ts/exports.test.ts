import assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { getPlatform } from '@node-3d/addon-tools';
import * as Segfault from './index.ts';

const signalsWindows = [
	'EXCEPTION_ALL',
	'EXCEPTION_ACCESS_VIOLATION',
	'EXCEPTION_DATATYPE_MISALIGNMENT',
	'EXCEPTION_BREAKPOINT',
	'EXCEPTION_SINGLE_STEP',
	'EXCEPTION_ARRAY_BOUNDS_EXCEEDED',
	'EXCEPTION_FLT_DENORMAL_OPERAND',
	'EXCEPTION_FLT_DIVIDE_BY_ZERO',
	'EXCEPTION_FLT_INEXACT_RESULT',
	'EXCEPTION_FLT_INVALID_OPERATION',
	'EXCEPTION_FLT_OVERFLOW',
	'EXCEPTION_FLT_STACK_CHECK',
	'EXCEPTION_FLT_UNDERFLOW',
	'EXCEPTION_INT_DIVIDE_BY_ZERO',
	'EXCEPTION_INT_OVERFLOW',
	'EXCEPTION_PRIV_INSTRUCTION',
	'EXCEPTION_IN_PAGE_ERROR',
	'EXCEPTION_ILLEGAL_INSTRUCTION',
	'EXCEPTION_NONCONTINUABLE_EXCEPTION',
	'EXCEPTION_STACK_OVERFLOW',
	'EXCEPTION_INVALID_DISPOSITION',
	'EXCEPTION_GUARD_PAGE',
	'EXCEPTION_INVALID_HANDLE',
	'STATUS_STACK_BUFFER_OVERRUN',
];

const signalsUnix = [
	'SIGINT',
	'SIGILL',
	'SIGABRT',
	'SIGFPE',
	'SIGSEGV',
	'SIGTERM',
	'SIGHUP',
	'SIGQUIT',
	'SIGTRAP',
	'SIGBUS',
	'SIGKILL',
	'SIGUSR1',
	'SIGUSR2',
	'SIGPIPE',
	'SIGALRM',
	'SIGCHLD',
	'SIGCONT',
	'SIGSTOP',
	'SIGTSTP',
	'SIGTTIN',
	'SIGTTOU',
	'SIGURG',
	'SIGXCPU',
	'SIGXFSZ',
	'SIGVTALRM',
	'SIGPROF',
	'SIGWINCH',
	'SIGSYS',
];

describe('Segfault', () => {
	it('exports functions', () => {
		assert.strictEqual(typeof Segfault.causeSegfault, 'function');
		assert.strictEqual(typeof Segfault.causeDivisionInt, 'function');
		assert.strictEqual(typeof Segfault.causeOverflow, 'function');
		assert.strictEqual(typeof Segfault.causeIllegal, 'function');
		assert.strictEqual(typeof Segfault.setSignal, 'function');
		assert.strictEqual(typeof Segfault.setLogPath, 'function');
	});

	for (const name of getPlatform() === 'windows' ? signalsWindows : signalsUnix) {
		it(`contains the \`${name}\` constant`, () => {
			assert.strictEqual(typeof Segfault[name as keyof typeof Segfault], 'number');
		});
	}
});
