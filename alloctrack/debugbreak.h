//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#ifndef __DEBUGBREAK_
#define __DEBUGBREAK_

#include "alloctrackconfig.h"

/// Utility function that breaks execution into the debugger in some fashion.
/// This may be non-recoverable, but ideally is a break into the debugger.
void ALLOCTRACKAPI debugBreak();

#endif /// __DEBUGBREAK_
