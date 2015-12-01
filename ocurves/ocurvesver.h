//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef OCURVESVER_H
#define OCURVESVER_H

#define OCURVES_MAJOR_VER 0
#define OCURVES_MINOR_VER 9
#define OCURVES_PATCH_VER 0

// Release candidate or beta display string.
//#define OCURVES_BETA_RC "RC"
//#define OCURVES_BETA_RC "Beta"
// Beta or release candidate version number. Only displayed if OCURVES_BETA_RC is defined.
#define OCURVES_BETA_RC_VER 1

// Build number should be incremented every for every stable build in the master branch.
#define OCURVES_BUILD_NUMBER 63

// Show the OCURVES_BUILD_NUMBER in the UI?
// This should be disabled for full releases including beta and RC releases, but enabled
// for all other builds in the master branch.
#define OCURVES_DISPLAY_BUILD 1

#endif  // OCURVESVER_H
