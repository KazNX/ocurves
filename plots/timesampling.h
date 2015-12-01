//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef TIMESAMPLING_H_
#define TIMESAMPLING_H_

/// @ingroup plot
/// Defines how to sample a plot file.
struct TimeSampling
{
  unsigned column;  ///< Index of the time column: 1-based, zero for none.
  double base;      ///< Base time, subtracted from the time value before scaling.
  double scale;     ///< Scale time values by this (even with no time column).
  unsigned flags;   ///< Additional flags (for generation).
};

#endif // TIMESAMPLING_H_
