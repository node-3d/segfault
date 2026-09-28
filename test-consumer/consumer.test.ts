import assert from 'node:assert/strict';
import test from 'node:test';

import { EXCEPTION_ALL, setLogPath, setSignal } from '@node-3d/segfault';

test('loads the native addon through the public entry point', () => {
	assert.equal(typeof setSignal, 'function');
	setLogPath(null);
	setSignal(EXCEPTION_ALL, false);
});
