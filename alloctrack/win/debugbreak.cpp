//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#include "debugbreak.h"

#include <intrin.h>

void debugBreak()
{
  __debugbreak();
}

