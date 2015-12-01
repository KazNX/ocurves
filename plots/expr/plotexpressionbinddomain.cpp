//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotexpressionbinddomain.h"

#include <algorithm>

void domainUnion(PlotExpressionBindDomain &result, const PlotExpressionBindDomain &a, const PlotExpressionBindDomain &b)
{
  if (a.isUnbounded())
  {
    result = b;
    return;
  }
  if (b.isUnbounded())
  {
    result = a;
    return;
  }

  result.minSet = a.minSet || b.minSet;
  if (a.minSet && b.minSet)
  {
    result.domainMin = std::min(a.domainMin, b.domainMin);
  }
  else if (a.minSet)
  {
    result.domainMin = a.domainMin;
  }
  else if (b.minSet)
  {
    result.domainMin = b.domainMin;
  }


  result.maxSet = a.maxSet || b.maxSet;
  if (a.maxSet && b.maxSet)
  {
    result.domainMax = std::max(a.domainMax, b.domainMax);
  }
  else if (a.maxSet)
  {
    result.domainMax = a.domainMax;
  }
  else if (b.maxSet)
  {
    result.domainMax = b.domainMax;
  }

  if (a.sampleDelta > 0 && b.sampleDelta > 0)
  {
    result.sampleDelta = std::min(a.sampleDelta, b.sampleDelta);
  }
  else if (a.sampleDelta > 0)
  {
    result.sampleDelta = a.sampleDelta;
  }
  else if (b.sampleDelta > 0)
  {
    result.sampleDelta = b.sampleDelta;
  }
  else if (result.minSet && result.maxSet)
  {
    result.sampleDelta = result.domainMax - result.domainMin;
  }
  else
  {
    result.sampleDelta = 1;
  }

  result.sampleCount = 1u + std::max<size_t>(size_t((result.domainMax - result.domainMin) / result.sampleDelta), 1u);
}
