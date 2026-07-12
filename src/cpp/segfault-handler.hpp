#pragma once

#include <addon-tools.hpp>


namespace segfault {
DBG_EXPORT void init();

DBG_EXPORT JS_METHOD(causeSegfault);
DBG_EXPORT JS_METHOD(causeDivisionInt);
DBG_EXPORT JS_METHOD(causeOverflow);
DBG_EXPORT JS_METHOD(causeIllegal);
DBG_EXPORT JS_METHOD(setSignal);
DBG_EXPORT JS_METHOD(setLogPath);
} // namespace segfault
