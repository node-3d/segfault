import { createRequire } from 'node:module';
import { getBin } from '@node-3d/addon-tools';

type NativeConstant = number | null;

type TNative = {
	causeSegfault: () => void;
	causeDivisionInt: () => void;
	causeOverflow: () => void;
	causeIllegal: () => void;
	setSignal: (signalId: number | null, value: boolean) => void;
	setLogPath: (path: string | null) => void;
	readonly EXCEPTION_ALL: NativeConstant;
	readonly EXCEPTION_ACCESS_VIOLATION: NativeConstant;
	readonly EXCEPTION_DATATYPE_MISALIGNMENT: NativeConstant;
	readonly EXCEPTION_BREAKPOINT: NativeConstant;
	readonly EXCEPTION_SINGLE_STEP: NativeConstant;
	readonly EXCEPTION_ARRAY_BOUNDS_EXCEEDED: NativeConstant;
	readonly EXCEPTION_FLT_DENORMAL_OPERAND: NativeConstant;
	readonly EXCEPTION_FLT_DIVIDE_BY_ZERO: NativeConstant;
	readonly EXCEPTION_FLT_INEXACT_RESULT: NativeConstant;
	readonly EXCEPTION_FLT_INVALID_OPERATION: NativeConstant;
	readonly EXCEPTION_FLT_OVERFLOW: NativeConstant;
	readonly EXCEPTION_FLT_STACK_CHECK: NativeConstant;
	readonly EXCEPTION_FLT_UNDERFLOW: NativeConstant;
	readonly EXCEPTION_INT_DIVIDE_BY_ZERO: NativeConstant;
	readonly EXCEPTION_INT_OVERFLOW: NativeConstant;
	readonly EXCEPTION_PRIV_INSTRUCTION: NativeConstant;
	readonly EXCEPTION_IN_PAGE_ERROR: NativeConstant;
	readonly EXCEPTION_ILLEGAL_INSTRUCTION: NativeConstant;
	readonly EXCEPTION_NONCONTINUABLE_EXCEPTION: NativeConstant;
	readonly EXCEPTION_STACK_OVERFLOW: NativeConstant;
	readonly EXCEPTION_INVALID_DISPOSITION: NativeConstant;
	readonly EXCEPTION_GUARD_PAGE: NativeConstant;
	readonly EXCEPTION_INVALID_HANDLE: NativeConstant;
	readonly STATUS_STACK_BUFFER_OVERRUN: NativeConstant;
	readonly SIGABRT: NativeConstant;
	readonly SIGFPE: NativeConstant;
	readonly SIGSEGV: NativeConstant;
	readonly SIGTERM: NativeConstant;
	readonly SIGILL: NativeConstant;
	readonly SIGINT: NativeConstant;
	readonly SIGALRM: NativeConstant;
	readonly SIGBUS: NativeConstant;
	readonly SIGCHLD: NativeConstant;
	readonly SIGCONT: NativeConstant;
	readonly SIGHUP: NativeConstant;
	readonly SIGKILL: NativeConstant;
	readonly SIGPIPE: NativeConstant;
	readonly SIGQUIT: NativeConstant;
	readonly SIGSTOP: NativeConstant;
	readonly SIGTSTP: NativeConstant;
	readonly SIGTTIN: NativeConstant;
	readonly SIGTTOU: NativeConstant;
	readonly SIGUSR1: NativeConstant;
	readonly SIGUSR2: NativeConstant;
	readonly SIGPROF: NativeConstant;
	readonly SIGSYS: NativeConstant;
	readonly SIGTRAP: NativeConstant;
	readonly SIGURG: NativeConstant;
	readonly SIGVTALRM: NativeConstant;
	readonly SIGWINCH: NativeConstant;
	readonly SIGXCPU: NativeConstant;
	readonly SIGXFSZ: NativeConstant;
};

const loadAddon = createRequire(import.meta.url);
const globalStore = globalThis as typeof globalThis & {
	'@node-3d/segfault'?: TNative;
};

export const native = (
	globalStore['@node-3d/segfault'] ?? loadAddon(`../${getBin()}/segfault.node`)
) as TNative;

globalStore['@node-3d/segfault'] ??= native;
