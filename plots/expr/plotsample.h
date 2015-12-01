//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef __PLOTSAMPLE_H_
#define __PLOTSAMPLE_H_

#include "plotsconfig.h"

#include "plotexpression.h"

class PlotInstanceSampler;

class QTextStream;

/// @ingroup expr
/// Details how an expression is referencing a plot sample.
///
/// Used by @c PlotSample to manage the curve and source references
/// The ID structure contains the referenced name, the quote character it was
/// defined with (' or ") and whether the name represents a regular expression.
struct PlotSampleId
{
  QString name; ///< The plot name or regular expression.
  char quote;   ///< The quote char parsed to define the ID.
  bool regex;   ///< True if @c name is a regular expression.

  /// Constructor.
  /// @param name The sample name or expression.
  /// @param quote The quote char: generally ' or " or the null character.
  /// @param regex True if @p name is a regular expression.
  inline PlotSampleId(const QString &name = QString(), char quote = '\0', bool regex = false)
    : name(name), quote(quote), regex(regex) { fixQuote(); }

  /// Constructor for unquoted references.
  /// @param name The sample name or expression.
  /// @param regex True if @p name is a regular expression.
  inline PlotSampleId(const QString &name, bool regex)
    : name(name), quote('\0'), regex(regex) { fixQuote(); }

  /// Copy constructor.
  /// @param other The object to copy.
  inline PlotSampleId(const PlotSampleId &other)
    : name(other.name), quote(other.quote), regex(other.regex) { }

  /// Bool operator: true if name is set.
  /// @return True if @c name is not empty.
  inline operator bool() const { return !name.isEmpty(); }

  /// Negation operator: true if not set.
  /// @return True if @c name is empty.
  inline bool operator !() const { return name.isEmpty(); }

  /// Ensures the @c quote character is set for regular expressions, defaulting to (').
  inline void fixQuote()
  {
    if (!quote && regex)
    {
      quote = '\'';
    }
  }
};

/// @ingroup expr
/// Stream operator for writing a @c PlotSampleId to @c QTextStream.
///
/// The @p sid is written only if it is not empty. The quote character wraps
/// the name appropriately and an 'r' is added as a prefix if the ID is a
/// regular expression.
///
/// @return The @p stream object after writing.
QTextStream &operator << (QTextStream &stream, const PlotSampleId &sid);

/// @ingroup expr
/// This expression class binds and references an existing curve.
///
/// This is the primary data source in the @c PlotExpression trees, where most
/// other expressions modify existing expressions. This class binds and samples
/// a @c PlotInstance.
///
/// The @c bind() method is implemented to bind the next appropriate @c PlotInstance
/// and report its domain in a @c PlotExpressionBindDomain. Repeated binding is
/// supported through the @c PlotBindTracker. The @c sample() method then returns
/// data sampled from the currently bound @c PlotInstance object.
///
/// The binding logic may match just the curve name or the curve and source names.
/// Source names will only match file sources. The binding logic may optionally
/// use regular expression binding for either name or source. The minimal
/// binding identification requires just the curve name match.
///
/// Uses @c PlotInstanceSampler for sampling.
///
/// A @c PlotSample object can only be bound to a single @c PlotInstance at a time.
class PlotSample : public PlotExpression
{
public:
  /// Create a @c PlotSample which matches just a curve name in binding.
  /// @param curveName The curve name to match.
  /// @param curveRegularExpression True if @p curveName is a regular expression.
  PlotSample(const QString &curveName, bool curveRegularExpression = false);

  /// Create a @c PlotSample which matches a curve name and file name in binding.
  /// @param curveName The curve name to match.
  /// @param fileName The source name to match against available file sources.
  /// @param curveRegularExpression True if @p curveName is a regular expression.
  /// @param fileRegularExpression True if @p fileName is a regular expression.
  PlotSample(const QString &curveName, const QString &fileName, bool curveRegularExpression = false, bool fileRegularExpression = false);

  /// Create a @c PlotSample which matches a curve name and file name in binding.
  /// @param curveId The curve ID to match.
  /// @param fileId The source ID to match against available file sources.
  PlotSample(const PlotSampleId &curveId, const PlotSampleId &fileId = PlotSampleId());

  /// Copy constructor, including copying the current binding.
  /// @param other The object to copy.
  PlotSample(const PlotSample &other);

  /// Destructor.
  ~PlotSample();

  /// Called to generate a sample at @p sampleTime.
  ///
  /// The implementation is optimised for sequential sampling and provides linearly
  /// interpolated points between true sample points.
  ///
  /// @param sampleTime The time to sample the expression at.
  /// @return The calculated sample at @p sampleTime.
  virtual double sample(double sampleTime) const;

  /// Attempts to bind to a curve mathcing @c curveName().
  ///
  /// The binding supports regular expressions in both source and curve names, or exact matches,
  /// with the source name optional. In either case, only @c PlotSource::File type sources are
  /// accepted with sources of all other types ignored.
  ///
  /// Repeated bindings are managed via the @p bindTracker.
  ///
  /// @return True on successful binding. Do not call @c sample() unless binding
  /// succeeds.
  virtual BindResult bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker,
                          PlotExpressionBindDomain &info, bool repeatLastBinding = false);

  /// Releases the current binding (if any).
  virtual void unbind();

  /// Creates a deep copy of this object.
  /// @return A deep copy.
  virtual PlotExpression *clone() const;

private:
  /// Converts back to the generating expression string.
  virtual QString stringExpression() const;

  /// Generates a bind name for the given @p plot, to be used as the display name.
  ///
  /// Generated as follows:
  /// - \<plot.name()\> when there is no source specified.
  /// - \<plot.source().name()\>|\<plot.name()\> when a source has been specified.
  ///
  /// @param plot The curve to make a name for.
  /// @return A display name for @p plot.
  QString makeBoundName(const PlotInstance &plot) const;

  // Binding support.

  /// Checks if @p name matches the @p searchName or @p re object.
  /// @param name The name to check.
  /// @param searchName The name to check against (exact match). Ignored if @p re is non-null.
  /// @param re The regular expression to check.
  /// @return True if @p searchName and @p re are null/empty, otherwise true when @p name matches
  ///   either @p re or @p searchName.
  static bool nameMatch(const QString &name, const QString &searchName, const QRegExp *re = nullptr);

  PlotSampleId _curveId;    ///< Curve name matching ID.
  PlotSampleId _fileId;     ///< File source name matching ID.
  QString _boundName; ///< Bound curve name (for RegEx match).
  PlotInstanceSampler *_sampler;  ///< Bound data set.
  mutable unsigned int _previousSample; ///< Last sample index.
};

#endif // __PLOTSAMPLE_H_
