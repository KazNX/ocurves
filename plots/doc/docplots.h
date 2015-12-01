//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef DOCPLOTS_H_
#define DOCPLOTS_H_

/// @ingroup api
/// @defgroup plot Plot Support
/// This module supports the plot or curve data model.
///
/// Each curve is represented by a @c PlotInstance. The @c PlotSource and
/// @c PlotInstnaceSampler provide additional meta data and sampling support
/// for each plot. Meanwhile, the @c PlotExpression and derivations support
/// generating plots based on existing plots using a format expression syntax.
///
/// The expressions are parsed using Bison and Flex, but the generated files
/// are provided for platforms where these are unavailable.

/// @ingroup api
/// @defgroup expr Plot Expressions
/// This module contains an expression parser and tree generation for creating
/// data sets derived from existing data sets. For example, a new curve
/// can be generted by the expression : 'a - b', creating a data series from
/// the difference between a and b.

#endif // DOCPLOTS_H_
