//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef DOCGLOSSARY_H_
#define DOCGLOSSARY_H_

// Please keep the glossary table in alphabetical order.

/// @page glossary Glossary
/// This page contains a glossary of terms used in Open Curves.
///
/// Term        | Meaning
/// ----------- | -------------------------------------------------------------
/// curve       | A plottable data series object. Also referred to as a 'plot'.
/// column      | Consecutive curves from a single source may be referred to as columns. This is derived from the way a text file source can be considered as a table, with each line a new row and each row containing multiple columns of data.
/// expression  | A set of operations used to generate new data from existing data.
/// generator   | An object which creates curves. Also 'loader'.
/// plot        | An alias for 'curve'.
/// loader      | An alias for 'generator'.
/// source      | Identifies where a set of curves came from; e.g., file, serial connection, expressions. A single source will normally generate multiple curves.
/// time column | The column in a data source which is used to derive a time value for other curves from the same source. Uses one-based indexing, allowing a zero value to indicate a lack of a time column.

#endif // DOCGLOSSARY_H_
