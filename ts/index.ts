import { native } from './native.ts';

export type SignalId = number | null;

/**
 * Produce a segfault.
 *
 * Issues an actual segfault by accessing unavailable memory.
 */
export const causeSegfault = native.causeSegfault;

/**
 * Divides an integer number by zero.
 */
export const causeDivisionInt = native.causeDivisionInt;

/**
 * Overflows the program stack.
 */
export const causeOverflow = native.causeOverflow;

/**
 * Executes an illegal instruction.
 */
export const causeIllegal = native.causeIllegal;

/**
 * Enable or disable a signal handler.
 *
 * Passing `null` as the signal ID has no effect and is safe.
 */
export const setSignal = native.setSignal;

/**
 * Change the path of the `segfault.log` location.
 *
 * You can set it to `"C:/my.log"` or another existing file path. The addon
 * does not create the file automatically. Passing `null` or `""` resets the
 * path to `segfault.log`.
 */
export const setLogPath = native.setLogPath;

/**
 * Windows OS constant. `null` on Unix-like platforms.
 *
 * Captures all supported Windows exceptions.
 */
export const EXCEPTION_ALL: SignalId = native.EXCEPTION_ALL;

/**
 * Windows OS constant. `null` on Unix-like platforms.
 */
export const EXCEPTION_ACCESS_VIOLATION: SignalId = native.EXCEPTION_ACCESS_VIOLATION;
export const EXCEPTION_DATATYPE_MISALIGNMENT: SignalId = native.EXCEPTION_DATATYPE_MISALIGNMENT;
export const EXCEPTION_BREAKPOINT: SignalId = native.EXCEPTION_BREAKPOINT;
export const EXCEPTION_SINGLE_STEP: SignalId = native.EXCEPTION_SINGLE_STEP;
export const EXCEPTION_ARRAY_BOUNDS_EXCEEDED: SignalId = native.EXCEPTION_ARRAY_BOUNDS_EXCEEDED;
export const EXCEPTION_FLT_DENORMAL_OPERAND: SignalId = native.EXCEPTION_FLT_DENORMAL_OPERAND;
export const EXCEPTION_FLT_DIVIDE_BY_ZERO: SignalId = native.EXCEPTION_FLT_DIVIDE_BY_ZERO;
export const EXCEPTION_FLT_INEXACT_RESULT: SignalId = native.EXCEPTION_FLT_INEXACT_RESULT;
export const EXCEPTION_FLT_INVALID_OPERATION: SignalId = native.EXCEPTION_FLT_INVALID_OPERATION;
export const EXCEPTION_FLT_OVERFLOW: SignalId = native.EXCEPTION_FLT_OVERFLOW;
export const EXCEPTION_FLT_STACK_CHECK: SignalId = native.EXCEPTION_FLT_STACK_CHECK;
export const EXCEPTION_FLT_UNDERFLOW: SignalId = native.EXCEPTION_FLT_UNDERFLOW;
export const EXCEPTION_INT_DIVIDE_BY_ZERO: SignalId = native.EXCEPTION_INT_DIVIDE_BY_ZERO;
export const EXCEPTION_INT_OVERFLOW: SignalId = native.EXCEPTION_INT_OVERFLOW;
export const EXCEPTION_PRIV_INSTRUCTION: SignalId = native.EXCEPTION_PRIV_INSTRUCTION;
export const EXCEPTION_IN_PAGE_ERROR: SignalId = native.EXCEPTION_IN_PAGE_ERROR;
export const EXCEPTION_ILLEGAL_INSTRUCTION: SignalId = native.EXCEPTION_ILLEGAL_INSTRUCTION;
export const EXCEPTION_NONCONTINUABLE_EXCEPTION: SignalId = native.EXCEPTION_NONCONTINUABLE_EXCEPTION;
export const EXCEPTION_STACK_OVERFLOW: SignalId = native.EXCEPTION_STACK_OVERFLOW;
export const EXCEPTION_INVALID_DISPOSITION: SignalId = native.EXCEPTION_INVALID_DISPOSITION;
export const EXCEPTION_GUARD_PAGE: SignalId = native.EXCEPTION_GUARD_PAGE;
export const EXCEPTION_INVALID_HANDLE: SignalId = native.EXCEPTION_INVALID_HANDLE;
export const STATUS_STACK_BUFFER_OVERRUN: SignalId = native.STATUS_STACK_BUFFER_OVERRUN;

/**
 * Linux/Unix OS signal constant. `null` on Windows.
 */
export const SIGABRT: SignalId = native.SIGABRT;
export const SIGFPE: SignalId = native.SIGFPE;
export const SIGSEGV: SignalId = native.SIGSEGV;
export const SIGTERM: SignalId = native.SIGTERM;
export const SIGILL: SignalId = native.SIGILL;
export const SIGINT: SignalId = native.SIGINT;
export const SIGALRM: SignalId = native.SIGALRM;
export const SIGBUS: SignalId = native.SIGBUS;
export const SIGCHLD: SignalId = native.SIGCHLD;
export const SIGCONT: SignalId = native.SIGCONT;
export const SIGHUP: SignalId = native.SIGHUP;
export const SIGKILL: SignalId = native.SIGKILL;
export const SIGPIPE: SignalId = native.SIGPIPE;
export const SIGQUIT: SignalId = native.SIGQUIT;
export const SIGSTOP: SignalId = native.SIGSTOP;
export const SIGTSTP: SignalId = native.SIGTSTP;
export const SIGTTIN: SignalId = native.SIGTTIN;
export const SIGTTOU: SignalId = native.SIGTTOU;
export const SIGUSR1: SignalId = native.SIGUSR1;
export const SIGUSR2: SignalId = native.SIGUSR2;
export const SIGPROF: SignalId = native.SIGPROF;
export const SIGSYS: SignalId = native.SIGSYS;
export const SIGTRAP: SignalId = native.SIGTRAP;
export const SIGURG: SignalId = native.SIGURG;
export const SIGVTALRM: SignalId = native.SIGVTALRM;
export const SIGWINCH: SignalId = native.SIGWINCH;
export const SIGXCPU: SignalId = native.SIGXCPU;
export const SIGXFSZ: SignalId = native.SIGXFSZ;
