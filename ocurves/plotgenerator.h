//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTGENERATOR_H_
#define PLOTGENERATOR_H_

#include "ocurvesconfig.h"

#include <QThread>

#include <QDir>
#include <QList>
#include <QRgb>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QPointF>

class QMutex;
class QwtPlotCurve;
class QwtPointSeriesData;
class PlotExpression;

class Curves;
class FileHandle;
class PlotFile;
class PlotExpression;
class PlotInstance;

/// @ingroup gen
/// A curve generation thread. This is the base class for threads capable of
/// generating @c PlotInstance data. For example, generator may loads from file.
///
/// Each curve series is added to the @c Curves model and new data are pushed
/// into the  back buffer of the @c PlotInstance. The main thread must periodically
/// call @c Curves::migrateLoadingData() to move data to the main buffer and
/// to complete data loading.
///
/// A generator should operate under the following guidelines in order to maintain
/// thread safety:
/// - Generate a @c PlotSource for each new data source, which may generate
///   multiple @c PlotInstance objects. The source is shared between curves.
///   The source binds the timing details of the curves.
/// - Call @c Curves::newCurve() for each new @c PlotInstance it creates.
///   This is thread safe and can be called as each is curve created.
/// - Start loading or generating data, calling @c PlotInstance::addPoint()
///   for each new datum or use @c PlotInstance::addPoints() (threadsafe).
/// - Call @c Curves::completeLoading() for each @c PlotInstance once done.
/// - If the generator needs to dispose of a curve, it must do so by calling
///   @c Curves::removeCurve(), which will manage deletion in a threadsafe manner.
/// - A generator must constantly check the value of @p _abortFlag, and immediately
///   quit the generation loop if it is set. After aborting, the generator
///   must call @c Curves::removeCurve() for any incomplete curves or
///   @c Curves::completeLoading() for completed curves. The former is acceptable for
///   completed curves as well.
///
/// A generator should also effect the following signals so that progress may be
/// correctly monitored and reported on.
/// - Signal @c overallProgress() to report coarse level progress. For example, this
///   may be emitted for each file in a set of files to load. It is best to signal
///   this event as soon as possible with a zero @c current value in order to report
///   the total number of items to be processed.
/// - Signal @c itemName() if possible to publish the name of the item currently
///   being loaded. For example the file name (short version). This may be displayed in
///   the UI.
/// - Signal @c itemProgress() to report the progress through the current item.
///   For example, this may be repeatedly signalled while processing a file
///   to report how far through the file the generator has progressed. The event
///   should raised judiciously so as to avoid overwhelming the message system.
/// - Signal @c beginNewCurves() just prior to starting to add the @c PlotInstance
///   objects from a single @c PlotSource.
/// - Signal @c endNewCurves() once all curves from a @c PlotSource are added.
/// - Signal @c loadComplete() once the generator has finished.
///
/// Note the generator relinquishes ownership of a @c PlotInstance as soon as it
/// passes the curve to @c Curves::newCurve(). The @c PlotSource should be kept
/// wrapped in a @c PlotSource::Ptr to maintain thread safety.
///
/// @par Note
/// The terms <em>generator</em> and <em>loader</em> may be used interchangeably
/// as are <em>curve</em> and <em>plot</em>.
class PlotGenerator : public QThread
{
  Q_OBJECT
public:
  /// Plot generation control flags. Must be set before execution.
  enum ControlFlag
  {
    None = 0,
    /// Use relative time based on the first sample. Affects the
    /// @c PlotSource objects created by this generator.
    RelativeTime = (1 << 0)
  };

  /// Instantiate the generator thread to load data into @c curves.
  /// @param curves The data model to load into.
  PlotGenerator(Curves *curves);

  /// Destructor. The thread should generally be stopped before destructing.
  virtual ~PlotGenerator();

  /// Access the curves model to load data into.
  inline const Curves *curves() const { return _curves; }

  /// Set the expected time column for all curves created by this generator.
  /// This value is written to @c PlotSource objects created by this generator.
  ///
  /// Specific implementations may support alternative time specifications.
  /// @param number The index of the time column, or zero for none.
  void setTimeColumn(uint number);

  /// The time column value in used. See @c setTimeColumn().
  /// @return The one-based time column index, or zero for none.
  uint timeColumn() const;

  /// Set the scale factor applied to all curves created by this generator.
  /// This value is written to @c PlotSource objects created by this generator.
  ///
  /// Specific implementations may support alternative time specifications.
  /// @param scale The time scaling value. Must not be zero.
  void setTimeScale(double scale);

  /// Access the time scale used by this generator.
  /// @return The time scale affecting generated curves.
  double timeScale() const;

  /// Sets @c ControlFlag values for this generator.
  /// @param flags The flag to set. Replaces existing flags.
  void setControlFlags(uint flags);

  /// Set the state of subset of @c ControlFlag values without affecting other flags.
  /// @param flags The @c ControlFlag values to modify (may be just one).
  ///     Values not set are left unmodified.
  /// @param on True to enable @p flags, false to disable.
  void setControlFlags(uint flags, bool on);

  /// Access the current @c ControlFlag values.
  uint controlFlags() const;

  /// Call to request the generator terminate operation.
  void quit() { _abortFlag = true; }

  /// True if this is a file loader.
  /// @todo Make this less specific. Given the Q_OBJECT macro, this
  /// has become somewhat redundant in favour of a @c qobject_cast().
  virtual inline bool isFileLoad() const { return false; }

public slots:
  /// Request loading abort.
  void abortLoad();

signals:
  /// Signal the current item name. For UI display.
  /// @param name The name of the current @c PlotSource.
  void itemName(const QString &name);

  /// Update progress through the current item.
  /// @param current Current progress, ranging from 0 to 1000.
  void itemProgress(int current);

  /// This signal reports overall progress.
  ///
  /// Logically, this signal reports progress through the overall number of items
  /// to be loaded or processed. For example in the case of loading files,
  /// the @p total reflects the total number of files to load, while the
  /// @p current progress reflects the number of files which have been loaded.
  /// The file loader actually reports part progress through each file by setting
  /// the total item count to the number of files by 100.
  ///
  /// @param current Current item count, with optional part progress.
  /// @param total Total item count.
  void overallProgress(int current, int total);

  /// Begin the addition of new curves via @c Curves::newCurve()
  /// Block notification occurs at a per @c PlotSource level.
  /// @todo Review the utility of this event as it came about before the @c Curves class.
  void beginNewCurves();

  /// End notification of new curves via @c Curves::newCurve().
  /// @todo Review the utility of this event as it came about before the @c Curves class.
  void endNewCurves();

  /// Notify completion of curve generation.
  /// @param curveCount The number of curves loaded.
  void loadComplete(int curveCount);

protected:
  Curves *_curves;                      ///< Curves data model.
  QMutex *_dataMutex;                   ///< Data mutex.
  uint _controlFlags;                   ///< @c ControlFlag values affecting generation.
  uint _timeColumn;                     ///< Index of the time column, zero for none.
  double _timeScale;                    ///< Time scaling factor.
  bool _abortFlag;                      ///< Abort requested?
};

#endif // PLOTGENERATOR_H_
