//
// @author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef OCURVESUTIL_H_
#define OCURVESUTIL_H_

#include "ocurvesconfig.h"

#include <QString>

/// @ingroup ui
/// Utility functions for Open Curves.
namespace ocutil
{
  /// Generates a string from the version numbers in ocurvesver.h .
  /// The generated string is derived from:
  /// - Major, minor and patch numbers.
  /// - Beta or release candidate build number if enabled.
  /// - Build number of set to display the build number.
  /// @return The formated version string.
  QString versionString();
}

#endif // OCURVESUTIL_H_
