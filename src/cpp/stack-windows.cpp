#include <tchar.h>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>
#include <dbghelp.h>
#include <tlhelp32.h>
#include <stdlib.h>
#include <psapi.h>

#include "stack-windows.hpp"


#pragma comment(lib, "version.lib") // for "VerQueryValue"


namespace segfault {

constexpr size_t COUNT_PSAPI_MODULES = 4096;
constexpr size_t SIZE_BUF_PSAPI_MODULES = COUNT_PSAPI_MODULES * sizeof(HMODULE);
constexpr size_t SIZE_BUF_NAME = 4096;
constexpr char dbgToolPath[] = "\\Debugging Tools for Windows 64-Bit\\dbghelp.dll";
constexpr size_t dbgToolPathSize = sizeof(dbgToolPath);
constexpr size_t SIZE_MAX_PROGRAM_FILES = SIZE_BUF_NAME - dbgToolPathSize;
constexpr int USED_CONTEXT_FLAGS = CONTEXT_FULL;

HMODULE bufPsapiModules[COUNT_PSAPI_MODULES];
char bufFileName[SIZE_BUF_NAME];
char bufBaseName[SIZE_BUF_NAME];
char bufSymbolPath[SIZE_BUF_NAME];
char bufDbgToolPath[SIZE_BUF_NAME];

const char *toolDllNames[] = { "kernel32.dll", "tlhelp32.dll" };

struct IMAGEHLP_SYMBOL64_EXT : SYMBOL_INFO {
	char __ext[SIZE_BUF_NAME];
};

IMAGEHLP_SYMBOL64_EXT bufSymbol;
IMAGEHLP_MODULE64 bufModule;
MODULEINFO bufModuleInfo;

using PtrLibrary = std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&::FreeLibrary)>;

using TSymCleanup = decltype(&::SymCleanup);
using TSymFunctionTableAccess64 = decltype(&::SymFunctionTableAccess64);
using TSymGetLineFromAddr64 = decltype(&::SymGetLineFromAddr64);
using TSymGetModuleBase64 = decltype(&::SymGetModuleBase64);
using TSymGetModuleInfo = decltype(&::SymGetModuleInfo);
using TSymGetOptions = decltype(&::SymGetOptions);
using TSymFromAddr = decltype(&::SymFromAddr);
using TSymInitialize = decltype(&::SymInitialize);
using TSymLoadModuleEx = decltype(&::SymLoadModuleEx);
using TSymSetOptions = decltype(&::SymSetOptions);
using TStackWalk64 = decltype(&::StackWalk64);
using TUnDecorateSymbolName = decltype(&::UnDecorateSymbolName);
using TSymGetSearchPath = decltype(&::SymGetSearchPath);
using TCreateToolhelp32Snapshot = decltype(&::CreateToolhelp32Snapshot);
using TModule32First = decltype(&::Module32First);
using TModule32Next = decltype(&::Module32Next);
using TEnumProcessModules = decltype(&::EnumProcessModules);
using TGetModuleFileNameExA = decltype(&::GetModuleFileNameExA);
using TGetModuleBaseNameA = decltype(&::GetModuleBaseNameA);
using TGetModuleInformation = decltype(&::GetModuleInformation);

// Entry for each Callstack-Entry
struct CallstackEntry {
	DWORD64 offset; // if 0, we have no valid entry
	char funcName[SIZE_BUF_NAME];
	DWORD lineNumber;
	char sourceName[SIZE_BUF_NAME];
	char moduleName[SIZE_BUF_NAME];
};


TSymCleanup pSymCleanup = nullptr;
TSymFunctionTableAccess64 pSymFunctionTableAccess64 = nullptr;
TSymGetLineFromAddr64 pSymGetLineFromAddr64 = nullptr;
TSymGetModuleBase64 pSymGetModuleBase64 = nullptr;
TSymGetModuleInfo pSymGetModuleInfo = nullptr;
TSymGetOptions pSymGetOptions = nullptr;
TSymFromAddr pSymGetSymFromAddr64 = nullptr;
TSymInitialize pSymInitialize = nullptr;
TSymLoadModuleEx pSymLoadModuleEx = nullptr;
TSymSetOptions pSymSetOptions = nullptr;
TStackWalk64 pStackWalk64 = nullptr;
TUnDecorateSymbolName pUnDecorateSymbolName = nullptr;
TSymGetSearchPath pSymGetSearchPath = nullptr;

static inline PtrLibrary _makePtrModule(const char *name) {
	return PtrLibrary(LoadLibraryA(name), FreeLibrary);
}


template <class T> static inline T _getProc(HMODULE library, const char *name) {
	return reinterpret_cast<T>(GetProcAddress(library, name));
}


static inline bool _bufferModuleInfo(HANDLE hProcess, DWORD64 baseAddr, IMAGEHLP_MODULE64 *pModuleInfo) {
	pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
	return TRUE == pSymGetModuleInfo(hProcess, baseAddr, pModuleInfo);
}


static inline bool _cutLastPathSegment(char *path) {
	int len = strlen(path);
	char *p = (path + len - 2);

	while (p > path) {
		// locate the rightmost path separator
		if (*p == '\\' || *p == '/') {
			*p = '\0';
			break;
		}
		p--;
	}

	return p != path;
}

static inline bool _getLastPathSegment(char *path) {
	int len = strlen(path);
	char *p = (path + len - 2);
	int count = 1;

	while (p > path) {
		// locate the rightmost path separator
		if (*p == '\\' || *p == '/') {
			p++;
			break;
		}
		p--;
		count++;
	}

	if (p == path) {
		return false;
	}

	for (int i = 0; i < count; i++) {
		path[i] = p[i];
	}
	path[count] = '\0';

	return true;
}


static inline void
_loadModule(HANDLE hProcess, const char *img, const char *mod, DWORD64 baseAddr, DWORD size) {
	DWORD result = ERROR_SUCCESS;

	if (!pSymLoadModuleEx(hProcess, 0, img, mod, baseAddr, size, nullptr, 0)) {
		GetLastError();
	}
}

static inline bool loadPsapi(
    HMODULE psapi,
    TEnumProcessModules *pEnumProcessModules,
    TGetModuleFileNameExA *pGetModuleFileNameExA,
    TGetModuleBaseNameA *pGetModuleBaseNameA,
    TGetModuleInformation *pGetModuleInformation
) {
	*pEnumProcessModules = _getProc<TEnumProcessModules>(psapi, "EnumProcessModules");
	*pGetModuleFileNameExA = _getProc<TGetModuleFileNameExA>(psapi, "GetModuleFileNameExA");
	*pGetModuleBaseNameA = _getProc<TGetModuleBaseNameA>(psapi, "GetModuleBaseNameA");
	*pGetModuleInformation = _getProc<TGetModuleInformation>(psapi, "GetModuleInformation");
	return (*pEnumProcessModules && *pGetModuleFileNameExA && *pGetModuleBaseNameA && *pGetModuleInformation);
}


static inline bool _loadModulesWithPsapi(HANDLE hProcess) {
	TEnumProcessModules pEnumProcessModules;
	TGetModuleFileNameExA pGetModuleFileNameExA;
	TGetModuleBaseNameA pGetModuleBaseNameA;
	TGetModuleInformation pGetModuleInformation;

	PtrLibrary ptrPsapi = _makePtrModule("psapi.dll");
	if (!ptrPsapi.get() ||
	    !loadPsapi(
	        ptrPsapi.get(),
	        &pEnumProcessModules,
	        &pGetModuleFileNameExA,
	        &pGetModuleBaseNameA,
	        &pGetModuleInformation
	    )) {
		return false;
	}

	DWORD cbNeeded;
	if (!pEnumProcessModules(hProcess, bufPsapiModules, SIZE_BUF_PSAPI_MODULES, &cbNeeded)) {
		return false;
	}

	if (cbNeeded > SIZE_BUF_PSAPI_MODULES) {
		return false;
	}

	int countModulesFinal = cbNeeded / sizeof(HMODULE);

	int cnt = 0;
	for (int i = 0; i < countModulesFinal; i++) {
		pGetModuleInformation(hProcess, bufPsapiModules[i], &bufModuleInfo, sizeof bufModuleInfo);
		bufFileName[0] = 0;
		pGetModuleFileNameExA(hProcess, bufPsapiModules[i], bufFileName, SIZE_BUF_NAME);
		bufBaseName[0] = 0;
		pGetModuleBaseNameA(hProcess, bufPsapiModules[i], bufBaseName, SIZE_BUF_NAME);

		_loadModule(
		    hProcess,
		    bufFileName,
		    bufBaseName,
		    reinterpret_cast<DWORD64>(bufModuleInfo.lpBaseOfDll),
		    bufModuleInfo.SizeOfImage
		);

		cnt++;
	}

	return cnt != 0;
}


static inline bool _loadModulesWithTh32(HANDLE hProcess, DWORD pid) {
	MODULEENTRY32 me;
	me.dwSize = sizeof(me);

	for (auto name : toolDllNames) {
		PtrLibrary ptrToolHelp = _makePtrModule(name);
		if (!ptrToolHelp.get()) {
			continue;
		}

		TCreateToolhelp32Snapshot pCreateToolhelp32Snapshot =
		    _getProc<TCreateToolhelp32Snapshot>(ptrToolHelp.get(), "CreateToolhelp32Snapshot");
		TModule32First pModule32First = _getProc<TModule32First>(ptrToolHelp.get(), "Module32First");
		TModule32Next pModule32Next = _getProc<TModule32Next>(ptrToolHelp.get(), "Module32Next");

		if (!pCreateToolhelp32Snapshot || !pModule32First || !pModule32Next) {
			continue;
		}

		HANDLE hSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		if (hSnap == reinterpret_cast<HANDLE>(-1)) {
			return false;
		}

		bool keepGoing = !!pModule32First(hSnap, &me);
		int cnt = 0;
		while (keepGoing) {
			_loadModule(
			    hProcess, me.szExePath, me.szModule, reinterpret_cast<DWORD64>(me.modBaseAddr), me.modBaseSize
			);
			cnt++;
			keepGoing = !!pModule32Next(hSnap, &me);
		}

		CloseHandle(hSnap);
		return cnt > 0;
	}

	return false;
}


// Dynamically load the Entry-Points for dbghelp.dll:
// Try to load the dbghelp.dll from the "Debugging Tools for Windows"
// Otherwise try to load any available dbghelp.dll library
static inline PtrLibrary _loadDbgHelpDll() {
	DWORD writtenCount = GetEnvironmentVariable("ProgramFiles", bufDbgToolPath, 4096);

	if (writtenCount > 0 && writtenCount < SIZE_MAX_PROGRAM_FILES) {
		strcat_s(bufDbgToolPath, dbgToolPath);
		if (GetFileAttributes(bufDbgToolPath) != INVALID_FILE_ATTRIBUTES) {
			PtrLibrary ptrModule = _makePtrModule(bufDbgToolPath);
			if (ptrModule.get()) {
				return ptrModule;
			}
		}
	}

	return _makePtrModule("dbghelp.dll");
}


static inline bool _loadDbgFuncs(HMODULE dbgHelpDll) {
	pSymInitialize = _getProc<TSymInitialize>(dbgHelpDll, "SymInitialize");
	pSymCleanup = _getProc<TSymCleanup>(dbgHelpDll, "SymCleanup");
	pStackWalk64 = _getProc<TStackWalk64>(dbgHelpDll, "StackWalk64");
	pSymGetOptions = _getProc<TSymGetOptions>(dbgHelpDll, "SymGetOptions");
	pSymSetOptions = _getProc<TSymSetOptions>(dbgHelpDll, "SymSetOptions");
	pSymFunctionTableAccess64 = _getProc<TSymFunctionTableAccess64>(dbgHelpDll, "SymFunctionTableAccess64");
	pSymGetLineFromAddr64 = _getProc<TSymGetLineFromAddr64>(dbgHelpDll, "SymGetLineFromAddr64");
	pSymGetModuleBase64 = _getProc<TSymGetModuleBase64>(dbgHelpDll, "SymGetModuleBase64");
	pSymGetModuleInfo = _getProc<TSymGetModuleInfo>(dbgHelpDll, "SymGetModuleInfo");
	pSymGetSymFromAddr64 = _getProc<TSymFromAddr>(dbgHelpDll, "SymFromAddr");
	pUnDecorateSymbolName = _getProc<TUnDecorateSymbolName>(dbgHelpDll, "UnDecorateSymbolName");
	pSymLoadModuleEx = _getProc<TSymLoadModuleEx>(dbgHelpDll, "SymLoadModuleEx");
	pSymGetSearchPath = _getProc<TSymGetSearchPath>(dbgHelpDll, "SymGetSearchPath");

	return (
	    pSymCleanup && pSymFunctionTableAccess64 && pSymGetModuleBase64 && pSymGetModuleInfo &&
	    pSymGetOptions && pSymGetSymFromAddr64 && pSymInitialize && pSymSetOptions && pStackWalk64 &&
	    pUnDecorateSymbolName && pSymLoadModuleEx
	);
}


static inline std::string _buildSymbolsPath() {
	bufSymbolPath[SIZE_BUF_NAME - 1] = 0;

	std::stringstream ss;
	ss << ".;";

	if (GetModuleFileNameA(nullptr, bufSymbolPath, SIZE_BUF_NAME - 1) > 0) {
		_cutLastPathSegment(bufSymbolPath);
		ss << bufSymbolPath << ";";
	}

	if (GetCurrentDirectoryA(SIZE_BUF_NAME - 1, bufSymbolPath) > 0) {
		ss << bufSymbolPath << ";";
	}

	if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", bufSymbolPath, SIZE_BUF_NAME - 1) > 0) {
		ss << bufSymbolPath << ";";
	}

	if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", bufSymbolPath, SIZE_BUF_NAME - 1) > 0) {
		ss << bufSymbolPath << ";";
	}

	if (GetEnvironmentVariableA("SYSTEMROOT", bufSymbolPath, SIZE_BUF_NAME - 1) > 0) {
		ss << bufSymbolPath << ";";
		ss << bufSymbolPath << "\\system32;";
	}

	ss << "SRV*";
	if (GetEnvironmentVariableA("SYSTEMDRIVE", bufSymbolPath, SIZE_BUF_NAME - 1) > 0) {
		ss << bufSymbolPath;
	} else {
		ss << "c:";
	}
	ss << "\\websymbols*http://msdl.microsoft.com/download/symbols;";

	return ss.str();
}


static inline void _init(HANDLE hProcess, HMODULE dbgHelpDll) {
	std::string symbolsPath = _buildSymbolsPath();

	if (!pSymInitialize(hProcess, symbolsPath.c_str(), FALSE)) {
		GetLastError();
	}

	DWORD symOptions = pSymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES;
	symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
	symOptions = pSymSetOptions(symOptions);
}


static inline void initEntityStrings(CallstackEntry *csEntry) {
	csEntry->funcName[0] = 0;
	csEntry->sourceName[0] = 0;
	csEntry->lineNumber = 0;
	csEntry->moduleName[0] = 0;
}


static inline void _handleOutput(std::ofstream &outfile, const std::string &text) {
	std::cerr << text << std::endl;

	if (outfile.is_open()) {
		outfile << text << std::endl;

		if (outfile.bad()) {
			std::cerr << "SegfaultHandler: Error writing to file." << std::endl;
		}
	}
}


static inline void _composeStackEntry(std::ofstream &outfile, CallstackEntry *entry) {
	if (!entry->funcName[0] && !entry->sourceName[0] && !entry->moduleName[0]) {
		return;
	}

	std::stringstream ss;

	if (entry->moduleName[0]) {
		ss << "[" << entry->moduleName << "]";
	}

	if (entry->sourceName[0]) {
		ss << " " << entry->sourceName << " (" << entry->lineNumber << ")";
	}

	if (entry->funcName[0]) {
		ss << ": " << entry->funcName;
	}

	_handleOutput(outfile, ss.str());
}


static BOOL __stdcall myReadProcMem(
    HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead
) {
	SIZE_T st;
	BOOL bRet = ReadProcessMemory(hProcess, (LPVOID)qwBaseAddress, lpBuffer, nSize, &st);
	*lpNumberOfBytesRead = static_cast<DWORD>(st);
	return bRet;
}


static inline CONTEXT _getValidContext() {
	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = USED_CONTEXT_FLAGS;
	RtlCaptureContext(&ctx);
	return ctx;
}


static inline void _iterateFrames(std::ofstream &outfile) {
	CONTEXT ctx = _getValidContext();

	// init STACKFRAME for first call
	STACKFRAME64 s;
	memset(&s, 0, sizeof(s));
	DWORD imageType;
#if defined(_M_ARM64) || defined(__aarch64__)
	imageType = IMAGE_FILE_MACHINE_ARM64;
	s.AddrPC.Offset = ctx.Pc;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrFrame.Offset = ctx.Fp;
	s.AddrFrame.Mode = AddrModeFlat;
	s.AddrStack.Offset = ctx.Sp;
	s.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_X64) || defined(__x86_64__)
	imageType = IMAGE_FILE_MACHINE_AMD64;
	s.AddrPC.Offset = ctx.Rip;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrFrame.Offset = ctx.Rsp;
	s.AddrFrame.Mode = AddrModeFlat;
	s.AddrStack.Offset = ctx.Rsp;
	s.AddrStack.Mode = AddrModeFlat;
#else
#error Unsupported Windows architecture
#endif

	CallstackEntry csEntry;

	memset(&bufModule, 0, sizeof(bufModule));
	bufModule.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

	IMAGEHLP_LINE64 Line;
	memset(&Line, 0, sizeof(Line));
	Line.SizeOfStruct = sizeof(Line);

	HANDLE hThread = GetCurrentThread();
	HANDLE hProcess = GetCurrentProcess();

	memset(&bufSymbol, 0, sizeof(IMAGEHLP_SYMBOL64_EXT));
	bufSymbol.SizeOfStruct = sizeof(SYMBOL_INFO);
	bufSymbol.MaxNameLen = SIZE_BUF_NAME;

	for (int frameNum = 0;; ++frameNum) {
		// get next stack frame (StackWalk64(), SymFunctionTableAccess64(), SymGetModuleBase64())
		// if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
		// assume that either you are done, or that the stack is so hosed that the next
		// deeper frame could not be found.
		if (!pStackWalk64(
		        imageType,
		        hProcess,
		        hThread,
		        &s,
		        &ctx,
		        myReadProcMem,
		        pSymFunctionTableAccess64,
		        pSymGetModuleBase64,
		        nullptr
		    )) {
			GetLastError();
			break;
		}

		csEntry.offset = s.AddrPC.Offset;
		initEntityStrings(&csEntry);

		if (s.AddrPC.Offset == s.AddrReturn.Offset) {
			GetLastError();
			break;
		}

		if (s.AddrPC.Offset) {
			if (pSymGetSymFromAddr64(hProcess, s.AddrPC.Offset, nullptr, &bufSymbol)) {
				pUnDecorateSymbolName(bufSymbol.Name, csEntry.funcName, SIZE_BUF_NAME, UNDNAME_COMPLETE);
				if (!csEntry.funcName[0]) {
					pUnDecorateSymbolName(bufSymbol.Name, csEntry.funcName, SIZE_BUF_NAME, UNDNAME_NAME_ONLY);
					if (!csEntry.funcName[0]) {
						strcpy_s(csEntry.funcName, bufSymbol.Name);
					}
				}
			} else {
				GetLastError();
			}

			DWORD offsetFromLine;
			if (pSymGetLineFromAddr64(hProcess, s.AddrPC.Offset, &offsetFromLine, &Line)) {
				csEntry.lineNumber = Line.LineNumber;
				strcpy_s(csEntry.sourceName, Line.FileName);
				_getLastPathSegment(csEntry.sourceName);
			} else {
				GetLastError();
			}

			if (_bufferModuleInfo(hProcess, s.AddrPC.Offset, &bufModule)) {
				strcpy_s(csEntry.moduleName, bufModule.LoadedImageName);
				_getLastPathSegment(csEntry.moduleName);
			} else {
				GetLastError();
			}
		}

		_composeStackEntry(outfile, &csEntry);

		if (!s.AddrReturn.Offset) {
			SetLastError(ERROR_SUCCESS);
			break;
		}
	}
}


DBG_EXPORT void showCallstack(std::ofstream &outfile) {
	PtrLibrary ptrDbgHelp = _loadDbgHelpDll();
	if (!ptrDbgHelp.get()) {
		return;
	}

	HANDLE hProcess = GetCurrentProcess();

	if (!_loadDbgFuncs(ptrDbgHelp.get())) {
		return;
	}

	_init(hProcess, ptrDbgHelp.get());

	DWORD dwProcessId = GetCurrentProcessId();
	if (!_loadModulesWithTh32(hProcess, dwProcessId)) {
		if (!_loadModulesWithPsapi(hProcess)) {
			return;
		}
	}

	_iterateFrames(outfile);

	if (pSymCleanup) {
		pSymCleanup(hProcess);
	}
}

} // namespace segfault
