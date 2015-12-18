//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTSOURCE_H_
#define PLOTSOURCE_H_

#include "plotsconfig.h"

#include "refcountobject.h"
#include "refcountptr.h"

#include <QString>
#include <QVector>

class PlotInstance;
class QMutex;

/// @ingroup plot
/// Provides details about a source from which @c PlotInstance objects have been generated.
///
/// The source is shared among @c PlotInstance objects generated from the same source
/// and the source knows of its plots. The source is reference counted because of its
/// shared nature and should always be wrapped in a @c Ptr.
///
/// The source also holds shared timing details such as time scale and column for the curves.
class PlotSource : public plotutil::RefCountObject<PlotSource>
{
  /// @cond Doxygen_Exclude
  friend plotutil::RefCountObject<PlotSource>;
  /// @endcond Doxygen_Exclude

public:
  /// Reference counted pointer reference.
  typedef plotutil::RefCountPtr<PlotSource> Ptr;

  /// Defines where the plot was loaded from.
  ///
  /// This lists the built-in sources.
  enum Type
  {
    /// No source: invalid.
    None,
    /// Loaded from a file.
    File,
    /// A generated expression with no other source.
    /// Expressions generally share their source with the original plot referenced by the expression.
    Expression,
    /// A real time plot over a serial, TCP or UDP connection.
    RealTime,

    /// User type values should start here.
    User = 64
  };

  /// Create a @c PlotSource of the given type and full name.
  ///
  /// The @c name() is set to match @p fullName. Use @c deriveName() to
  /// create a shorter @c name().
  ///
  /// @param sourceType The source type. Nominally a value from @c Type, though user types may be specified.
  /// @param fullName The full name for the source. E.g., the generating file name or expression.
  /// @see @c lengthenName().
  /// @param curveCount Optional specification for the expected number of curves associated with
  ///   this source. Used in array reservations.
  PlotSource(int sourceType, const QString &fullName, unsigned curveCount = 0);

  /// Simple destructor.
  ~PlotSource();

  /// Identify the source type.
  /// @return The source @c Type.
  inline unsigned type() const { return _type; }

  /// Access the short name. Used in short display context.
  /// @return The short name.
  inline QString name() const { return _name; }

  /// Set the short name.
  /// @param name The new name.
  inline void setName(QString name) { _name = name; }

  /// Access the full source name.
  /// @return The long source name.
  inline QString fullName() const { return _fullName; }

  /// Set the full source name (e.g., file name). Should be unique.
  /// Does not affect the short @c name().
  ///
  /// Follow with a call to @c deriveName() to set the @c name() based on the full name.
  /// @param name The new name.
  inline void setFullName(QString name) { _fullName = name; }

  /// Returns the index of the curve used to generate timestamps for all other columns.
  /// This is a 1 based index.
  ///
  /// A zero indicates no time column.
  inline unsigned timeColumn() const { return _timeColumn; }

  /// Set the 1-based time column index or zero for none.
  /// @param index The new column index.
  void setTimeColumn(unsigned index) { _timeColumn = index; }

  /// Returns the time scaling applied to curves using this source.
  /// @return The time scale multiplier.
  inline double timeScale() const { return _timeScale; }

  /// Returns the time scaling applied to curves using this source.
  /// @param scale The new time scale multiplier. Must not be zero.
  inline void setTimeScale(double scale) { _timeScale = scale; }

  /// Returns the time base for the source. This is considered time zero. Scaling is applied afterwards.
  /// @return The time base.
  inline double timeBase() const { return _timeBase; }
  /// Sets the time base for the source.
  /// @param time The base time (origin).
  inline void setTimeBase(double time) { _timeBase = time; }

  /// Request the first time value from the time column.
  /// @return The first value in the time column. Zero with no such column.
  double firstTime() const;

  /// Returns the number of curves associated with the source.
  unsigned curveCount() const;

  /// Request a curve at the given index.
  /// @param index The curve index. Should be [0, @c curveCount()), but out of range returns null.
  /// @return The curve at @p index, or null when out of bounds.
  PlotInstance *curve(unsigned index) const;

  /// Returns the time column curve (if any).
  /// @return The time column curve, or null if there is no such curve.
  PlotInstance *timeColumnCurve() const;

  /// Add a curve to this source. Internal use only.
  /// @param curve The curve to add.
  void addCurve(PlotInstance *curve);

  /// Remove a curve to this source. Internal use only.
  /// @param curve The curve to remove.
  void removeCurve(const PlotInstance *curve);

  /// Set the @c name() based on the current @c fullName().
  ///
  /// This treats @c fullName() as a file name, and extracts the
  /// file name without path or extension. Both slashes are accepted
  /// as delimiters.
  void deriveName();

  /// Lengthen the short @c name() to attempt to disambiguate.
  ///
  /// This method is designed to help distinguish situations where a source files share a name,
  /// but lie in different directories. Normally, the short name for both will be the same.
  /// By calling @c lengthenName(), the parent directory is added to the @c name().
  ///
  /// Successfull name lengthening requires that the @c fullName() is delimited by a directory
  /// separator character: either '/' or '\\' (both are accepted on all platforms). The name is
  /// lengthened by adding more delimited tokens to the short name. The following example
  /// continuing lengthens an example source name until it can no longer be lengthened.
  ///
  /// Initial @c name()  | Lengthened @c name()   | @c fullName()
  /// ------------------ | ---------------------- | ------------------------------------
  /// data               | dir-a/data             | C:\\temp\\dir-a\data.txt
  /// dir-a/data         | temp/dir-a/data        | C:\\temp\\dir-a\data.txt
  /// temp/dir-a/data    | C:/temp/dir-a/data     | C:\\temp\\dir-a\data.txt
  /// NYI:C:/temp/dir-a/data | C:/temp/dir-a/data.txt | C:\\temp\\dir-a\data.txt
  ///
  /// This function was intended for use when there were sources with the same @c name(),
  /// This may be revisited in future.
  ///
  /// @return True if the @c name() has been successfully lengthened. Otherwise it is
  ///   unchanged.
  bool lengthenName();

private:
  QString _name;        ///< A brief name.
  QString _fullName;    ///< A long, qualified name.
  int _type;            ///< Source type id.
  unsigned _timeColumn; ///< Time column 1-based index.
  double _timeScale;    ///< Time scale multiplier (after @c _timeBase shift)
  double _timeBase;     ///< Considered time zero (before scaling).
  QMutex *_curvesMutex; ///< To support expression generation modifying @c _curves.
  QVector<PlotInstance *> _curves;  ///< Curve list.
};

#endif // PLOTSOURCE_H_
