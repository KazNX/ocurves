//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef DOCGROUPS_H_
#define DOCGROUPS_H_

#include "ocurvesconfig.h"

// This header defines various doxygen groups.

/// @defgroup api API Reference Documentation
/// This section holds the API reference documentation.
/// @{

/// @defgroup ui User Interface
/// Various user interface components and controls.

/// @defgroup data Data Model
/// Contains classes which underpin the Open Curves data model.

/// @defgroup realtime Real Time Plots
/// Contains classes supporting the real-time plotting system.
///
/// This system is an alpha level feature.

/// @defgroup gen Plot Loaders and Generators
/// This section contains various plot data sources. Each source derives
/// the @c PlotGenerator and is designed for background thread operation.
/// Each data series is loaded into a @c PlotInstance.

/// @namespace ocurves
/// A namespace containing various utility functions and definitions.

/// @}

/// @ingroup api
/// @defgroup alloctrack

#endif // DOCGROUPS_H_
