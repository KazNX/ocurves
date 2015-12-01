//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef CURVES_H_
#define CURVES_H_

#include "ocurvesconfig.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QVariant>

class PlotExpression;
class PlotInstance;
class PlotSource;
class QMutex;
class QwtPlotCurve;

typedef QHash<QString, QVariant> VariantMap;

/// @ingroup data
/// The data model for loaded plots and curves.
///
/// Curves are added in the loading state via @c newCurve() and are considered
/// to be loading. Loading curves are expected to be populated in the background
/// and data are migrated via @c PlotInstance::migrateBuffer().
///
/// Once loaded, a curve no longer has data migrated.
///
/// The @c Curves object is designed to be thread safe to support background loading.
/// This includes delayed destruction of @c PlotInstance objects as they may be
/// accessed from different threads.
class Curves : public QObject
{
  Q_OBJECT
public:
  /// A thread safe capture of a list of curves internal to a @c Curves object.
  ///
  /// This provides a way to access one of the lists of @c PlotInstance objects
  /// in a @c Curves instance while maintaining thread safety. The curve list is
  /// locked while this object exists. Thus it is critical that the @c CurveList
  /// object go out of scope before attempting to access another @c CurveList.
  class CurveList
  {
  public:
    /// The iterator type.
    typedef QList<PlotInstance *>::iterator iterator;

    /// The const_iterator type.
    typedef QList<PlotInstance *>::const_iterator const_iterator;

    /// Constructor: internal use only.
    /// @param mutex The mutex to lock.
    /// @param curves The curves list.
    CurveList(QMutex &mutex, const QList<PlotInstance *> &curves);

    /// Destructor: releases the mutex.
    ~CurveList();

    /// Releases the curve list and lock allowing other threads to access the data. The object becomes invalid for further use.
    void release();

    /// Begin iteration of the curves. Must not be released.
    /// @return The iterator to the first item.
    inline const_iterator begin() const { return _curves->begin(); }

    /// End iterator. Must not be released.
    /// @return The iterator after the last item.
    inline const_iterator end() const { return _curves->end(); }

    /// Direct access to the underlying list (thread-safe).
    ///
    /// There is no guarantee the list items will remain valid.
    ///
    /// @return The underlying curve list
    const QList<PlotInstance *> &list() const { return *_curves; }

  private:
    QMutex *_mutex; ///< Mutex locked while this object persists and is not released.
    const QList<PlotInstance *> *_curves; ///< The underlying list.
  };

  /// Create a curves data model.
  /// @param parent The owning object.
  Curves(QObject *parent = nullptr);

  /// Destructor. Cleans up all owned curves.
  ~Curves();

  /// Retrieve the complete list of curves, loading or loaded.
  /// @return The list of curves.
  CurveList curves() const;

  /// Retrieve the complete list of loading curves.
  /// @return The list of loading curves.
  CurveList loadingCurves() const;

  /// Retrieve the complete list of real-time curves.
  /// @return The list of real-time curves.
  CurveList realTimeCurves() const;

  /// Remove all the curves which where generated from @p expression.
  /// @param expression The expression to look for.
  /// @return The number of curves removed.
  unsigned removeUsingExpression(const PlotExpression *expression);

  /// Enumerates the source files from which curves have been loaded.
  void enumerateFileSources(QStringList &filePaths) const;

  /// Enumerates all sources of the requested @p type.
  /// @param sources List to append to.
  /// @param type Source type to match.
  void enumerateSources(QList<PlotSource *> &sources, unsigned type) const;

  /// Retrieve the curve properties map for pending curves.
  ///
  /// This is as a direct consequence of supporting the potential loading delay in
  /// @c bookmarks::restoreBookmark(). See that function for details.
  /// @return The current properties map.
  inline const VariantMap &curvePropertiesMap() const { return _curvePropertiesMap; }

  /// Set the curve properties map. Affects existing and future curves.
  ///
  /// This is as a direct consequence of supporting the potential loading delay in
  /// @c bookmarks::restoreBookmark(). See that function for details.
  /// @param map The properties map.
  void setCurvePropertiesMap(VariantMap &map);

public slots:
  /// Add a new curve to the list of curves. The curve is considered to be in the loading state.
  ///
  /// The curve is added to the @c curves() and loadingCurves() lists. This call
  /// must be followed by either by a call to @c curveComplete() or to
  /// @c removeCurve().
  void newCurve(PlotInstance *curve);

  /// Signals successful completion of @c curve. It is no longer in the loading state.
  void completeLoading(PlotInstance *curve);

  /// Removes and deletes @c curve.
  /// @param curve The curve to remove.
  /// @return True if the curve was tracked and has been removed.
  bool removeCurve(const PlotInstance *curve);

  /// Invalidates @p curve by raising the @c curveDataChanged() signal.
  /// @param curve The curve to invalidate.
  void invalidate(const PlotInstance *curve);

  /// Invalidates @p source by raising the @c curveSourceDataChanged() signal.
  ///
  /// May also invalidate the assocaited @c PlotInstance objects by calling
  /// @c invalidate(const PlotInstance *) for each one.
  /// @param source The source to invalidate. Exits if null.
  /// @param invalidateCurves True to invalidate each associated @c PlotInstance.
  void invalidate(const PlotSource *source, bool invalidateCurves = false);

  /// Clears all curve data.
  void clearCurves();

  /// Migrates data from the back buffer of loading curves into the main display buffer.
  ///
  /// Invokes @c PlotInstance::migrateBuffer() for each loading curve.
  ///
  /// @return True if some data have been migrated, false when there is nothing to
  ///   migrate.
  bool migrateLoadingData();

public:
  /// Checks if the given curve is loading.
  /// @param curve The curve of interest. Handles null.
  /// @return True if @p curve is loading.
  bool isLoading(const PlotInstance *curve) const;

signals:
  /// Signals a curve is added (in the loading state).
  /// @param curve Curve of interest.
  void curveAdded(PlotInstance *curve);

  /// Signals that loading of a curve has finished.
  /// @param curve Curve of interest.
  void curveComplete(PlotInstance *curve);

  /// Signals that a curve has changed its data content.
  /// @param curve Curve of interest.
  void curveDataChanged(const PlotInstance *curve);

  /// Signals removal of a curve (and deletion).
  /// @param curve The removed and deleted curve.
  void curveRemoved(const PlotInstance *curve);

  /// Signals that a @c PlotSource has changed its data content - generally it's timing data.
  /// @param source Plot source of interest.
  void sourceDataChanged(const PlotSource *source);

  /// Signals all curve data has been cleared.
  void curvesCleared();

  /// Signals that all loading curves have completed loading.
  void loadingComplete();

private:
  /// Restore @p curve properties form the properties map.
  /// @param curve The curve to restore.
  /// @return True if @c curve is modified as a result.
  bool restoreProperties(PlotInstance &curve) const;

  /// Adds the given curve to death row for deletion and triggers an event to
  /// clear death row.
  /// @param curve The curve to delete.
  void addToDeathRow(const PlotInstance *curve);

private slots:
  /// Deletes any objects on death row.
  void clearDeathRow();

private:
  QList<PlotInstance *> _curves;          ///< All curves.
  QList<PlotInstance *> _loadingCurves;   ///< Curves which are loading.
  QList<PlotInstance *> _realTimeCurves;  ///< Real time plots, which never complete unless stopped.
  QList<PlotInstance *> _completedCurves; ///< Curves finished loading, awaiting notification on the main thread. Shares the loadingMutex
  QList<const PlotInstance *> _deathRow;  ///< Death row list. Cleaned up in @c
  mutable QMutex *_curvesMutex;           ///< Mutex for @c _curves.
  mutable QMutex *_loadingMutex;          ///< Mutex for @c _loadingCurves.
  mutable QMutex *_realTimeMutex;         ///< Mutex for @c _realTimeCurves.
  mutable QMutex *_deathRowMutex;         ///< Mutex for @c _deathRow.
  VariantMap _curvePropertiesMap;         ///< Curve properties may. See @c bookmarks::restoreBookmark().
};

#endif // CURVES_H_
