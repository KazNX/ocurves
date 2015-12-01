//
// author Kazys Stepanas
//
// Copyright (c) 2012 CSIRO
//
#include "debugbreak.h"

#include <signal.h>

void debugBreak()
{
  raise(SIGINT);
}
