// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef CRITICAL_H
#define CRITICAL_H

#ifdef __cplusplus
extern "C" {
#endif

void Critical_DisallowInterrupts(void);
void Critical_AllowInterrupts(void);

#ifdef __cplusplus
}
#endif

#endif
