# Segfault handler for Node.js

This is a part of [Node3D](https://github.com/node-3d) project.

[![NPM](https://badge.fury.io/js/@node-3d%2Fsegfault.svg)](https://badge.fury.io/js/@node-3d/segfault)
[![Lint](https://github.com/node-3d/segfault/actions/workflows/lint.yml/badge.svg)](https://github.com/node-3d/segfault/actions/workflows/lint.yml)
[![Test](https://github.com/node-3d/segfault/actions/workflows/test.yml/badge.svg)](https://github.com/node-3d/segfault/actions/workflows/test.yml)

```
npm install @node-3d/segfault
```

## Binary Provenance

Release binary archives for this package are published by public GitHub Actions
workflows.
When a release asset includes a GitHub Artifact Attestation, you can verify that
the downloaded archive was published by the visible workflow in this repository:

```console
gh release download <tag> -R node-3d/segfault -p <platform>.gz
gh attestation verify <platform>.gz -R node-3d/segfault
```

The attestation links the archive to the repository, workflow, commit, and
release build identity. It does not replace review of the source code, workflow,
or third-party library contents.

This module report debug information on process crash (and some other events).
It doesn't affect the performance while the program is running normally.

The module prints to both
**STDERR** and to the **"segfault.log"** file (if it exists inside **"working directory"**).
If there is no such file, it
**won't be created**, so it is up to you if the log-file is needed.

> Note: this **addon uses N-API**, and therefore is ABI-compatible across different
Node.js versions. Addon binaries are precompiled and **there is no compilation**
step during the `npm install` command.

A zero-setup is available: just import the module and it comes pre-equipped with several
signal listeners enabled by default.

```js
import '@node-3d/segfault';
```

> Note: if your project tree contains multiple versions of this module, the first one imported
will seize `global['@node-3d/segfault']`. The rest of them will only re-export `global['@node-3d/segfault']`
and **WILL NOT** import their own **binaries**.

---

If you want to use a custom location instead of "**segfault.log**", use:

```js
import { setLogPath } from '@node-3d/segfault';

setLogPath('C:/my/log/file.txt');
```

**You must create this file first, it must be there when the crash happens.**

## API

Configuration:

* `setLogPath(path | null | '')` - set the log file path. Passing `null` or `''`
  resets it to `segfault.log`.
* `setSignal(signal, enabled)` - enable or disable a supported signal/exception handler.
  Passing `null` is safe and has no effect, which is useful for cross-platform code.

Demo crash helpers:

* `causeSegfault()`
* `causeDivisionInt()`
* `causeOverflow()`
* `causeIllegal()`

Constants are platform-specific. Windows exception constants are `null` on Unix-like systems,
and Unix signal constants are `null` on Windows.


## Configuring Signals

As listed below, some signals (platform specific) are enabled by default. But they can be
enabled/disabled manually:

Example:

```js
import {
	setSignal,
	EXCEPTION_ACCESS_VIOLATION, SIGSEGV,
	EXCEPTION_BREAKPOINT, SIGTRAP,
	EXCEPTION_ALL,
} from '@node-3d/segfault';

setSignal(EXCEPTION_ACCESS_VIOLATION, false);
setSignal(SIGSEGV, false);

setSignal(EXCEPTION_BREAKPOINT, true);
setSignal(SIGTRAP, true);

// This enables capturing every signal on Windows.
setSignal(EXCEPTION_ALL, true);
```

On **Windows**, all the **Unix** signals are `null`, and the opposite is true.
Passing `null` as the first parameter to `setSignal` **has no effect and is safe**.


## Demo Methods

These are be helpful to see how the signals are reported and if the log files are being written properly.

* `causeSegfault` - Causes a memory access violation.
* `causeDivisionInt` - Divides an integer by zero.
* `causeOverflow` - Runs infinite recursion (stack overflow).
* `causeIllegal` - Raises an "illegal instruction" exception.

Example:

```js
import { causeSegfault } from '@node-3d/segfault';

causeSegfault();
```


## Windows Signals

| Signal                             | Enabled | Description                                        |
| :---                               | :---:   | :---                                               |
| EXCEPTION_ALL                      | no      | All of the below.                                  |
| EXCEPTION_ACCESS_VIOLATION         | yes     | Memory access was denied.                          |
| EXCEPTION_ARRAY_BOUNDS_EXCEEDED    | yes     | Array was accessed with an illegal index.          |
| EXCEPTION_INT_DIVIDE_BY_ZERO       | yes     | Attempt to divide by an integer divisor of 0.      |
| EXCEPTION_ILLEGAL_INSTRUCTION      | yes     | Attempt to execute an illegal instruction.         |
| EXCEPTION_NONCONTINUABLE_EXCEPTION | yes     | Can't continue after an exception.                 |
| EXCEPTION_STACK_OVERFLOW           | yes     | The thread used up its stack.                      |
| EXCEPTION_INVALID_HANDLE           | yes     | An invalid handle was specified.                   |
| EXCEPTION_FLT_DIVIDE_BY_ZERO       | no      | Attempt to divide by a float divisor of 0.f.       |
| EXCEPTION_DATATYPE_MISALIGNMENT    | no      | A datatype misalignment was detected.              |
| EXCEPTION_BREAKPOINT               | no      | A Breakpoint has been reached.                     |
| EXCEPTION_SINGLE_STEP              | no      | Continue single-stepping execution.                |
| EXCEPTION_FLT_DENORMAL_OPERAND     | no      | One of the operands is denormal.                   |
| EXCEPTION_FLT_INEXACT_RESULT       | no      | The result cannot be represented exactly.          |
| EXCEPTION_FLT_INVALID_OPERATION    | no      | Floating-point invalid operation.                  |
| EXCEPTION_FLT_OVERFLOW             | no      | The exponent of operation is too large.            |
| EXCEPTION_FLT_STACK_CHECK          | no      | The Stack gone bad after a float operation.        |
| EXCEPTION_FLT_UNDERFLOW            | no      | The exponent of operation is too low.              |
| EXCEPTION_INT_OVERFLOW             | no      | The result of operation is too large.              |
| EXCEPTION_PRIV_INSTRUCTION         | no      | Operation is not allowed in current mode.          |
| EXCEPTION_IN_PAGE_ERROR            | no      | Can't access a memory page.                        |
| EXCEPTION_INVALID_DISPOSITION      | no      | Invalid disposition returned.                      |
| EXCEPTION_GUARD_PAGE               | no      | Accessing PAGE_GUARD-allocated modifier.           |


## Unix Signals

| Signal     | Handling | Enabled | Description                                        |
| :---       | :---:    | :---:   | :---                                               |
| SIGABRT    |    A     | yes     | Process abort signal.                              |
| SIGFPE     |    A     | yes     | Erroneous arithmetic operation.                    |
| SIGSEGV    |    A     | yes     | Invalid memory reference.                          |
| SIGILL     |    A     | yes     | Illegal instruction.                               |
| SIGBUS     |    A     | yes     | Access to an undefined portion of a memory object. |
| SIGTERM    |    T     | no      | Termination signal.                                |
| SIGINT     |    T     | no      | Terminal interrupt signal.                         |
| SIGALRM    |    T     | no      | Alarm clock.                                       |
| SIGCHLD    |    I     | no      | Child process terminated, stopped, or continued.   |
| SIGCONT    |    C     | no      | Continue executing, if stopped.                    |
| SIGHUP     |    T     | no      | Hangup.                                            |
| SIGKILL    |    T     | no      | Kill (cannot be caught or ignored).                |
| SIGPIPE    |    T     | no      | Write on a pipe with no one to read it.            |
| SIGQUIT    |    A     | no      | Terminal quit signal.                              |
| SIGSTOP    |    S     | no      | Stop executing (cannot be caught or ignored).      |
| SIGTSTP    |    S     | no      | Terminal stop signal.                              |
| SIGTTIN    |    S     | no      | Background process attempting read.                |
| SIGTTOU    |    S     | no      | Background process attempting write.               |
| SIGUSR1    |    T     | no      | User-defined signal 1.                             |
| SIGUSR2    |    T     | no      | User-defined signal 2.                             |
| SIGPROF    |    T     | no      | Profiling timer expired.                           |
| SIGSYS     |    A     | no      | Bad system call.                                   |
| SIGTRAP    |    A     | no      | Trace/breakpoint trap.                             |
| SIGURG     |    I     | no      | High bandwidth data is available at a socket.      |
| SIGVTALRM  |    T     | no      | Virtual timer expired.                             |
| SIGXCPU    |    A     | no      | CPU time limit exceeded.                           |
| SIGXFSZ    |    A     | no      | File size limit exceeded.                          |

* `T` - Abnormal termination of the process.
* `A` - Abnormal termination of the process with additional actions.
* `I` - Ignore the signal.
* `S` - Stop the process.
* `C` - Continue the process, if it is stopped; otherwise, ignore the signal.


## Legal notice

This package is derived from [segfault-handler](https://github.com/ddopson/node-segfault-handler).
The original licensing rules apply, see `LICENSE_node-segfault-handler`.

Also this project uses [callstack walker](https://github.com/JochenKalmbach/StackWalker)
which is licensed under BSD-2 Clause.

The rest of this package (the newly introduced files) is licensed under MIT.
