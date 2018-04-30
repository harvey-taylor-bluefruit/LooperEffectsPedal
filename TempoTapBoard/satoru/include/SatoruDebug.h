// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef SATORU_DEBUG_H
#define SATORU_DEBUG_H

#if defined(SATORU_DEBUG) && !defined(NDEBUG)

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
namespace satoru { namespace debug {
// User supplied print method.
void Print(const char*);

static const size_t StackSafetyLimitBytes = 200U;
} }
#else
void SatoruDebug_Print(const char *);
#endif

#endif

#endif
