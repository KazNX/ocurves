//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef BOOKMARKS_H_
#define BOOKMARKS_H_

#include "ocurvesconfig.h"

#include <QStringList>

#include "curves.h"

class OCurvesUI;
class QSettings;

/// @ingroup data
/// Utility functions dealing with the bookmarking system.
namespace bookmarks
{
  /// Bookmark the current @c OCurvesUI state.
  ///
  /// The bookmark is stored in @p settings under the given @p id and @p name.
  /// The bookmark is essentially a snapshot of the current state, with limited exclusions
  /// on the currently loaded sources.
  ///
  /// In general terms, the bookmark stores the following settings:
  /// - The UI state and layout, including split views and zoom levels.
  /// - The current expressions.
  /// - The currently selected sources and plots.
  ///
  /// The bookmark also stores the list of selected file sources to ensure they are loaded when the
  /// bookmark is restored. This may optionally include all sources, not just selected ones,
  /// when @p includeInactiveSources is true.
  ///
  /// @param settings The settings object to store the bookmark in.
  /// @param id The bookmark ID. The number is arbitrary, though low, sequential numbers are expected
  ///     and zero is generally used for the last session.
  /// @param name The name of the bookmark, used in the UI display.
  /// @param ui The application.
  /// @param includeInactiveSources True to include unselected file sources when restoring.
  void setBookmark(QSettings &settings, unsigned id, const QString &name, OCurvesUI *ui, bool includeInactiveSources = false);

  /// @overload
  void setBookmark(QSettings &settings, unsigned id, OCurvesUI *ui, bool includeInactiveSources = false);

  /// Attempts to restore a bookmark.
  ///
  /// This restores all settings stored in the bookmark. Any file sources which aren't
  /// currently loaded are queued for loading.
  ///
  /// The bookmark may also contain settings which relate to specific @c PlotInstance curves.
  /// These are not loaded directly as there is no guarantee that the curve exists yet. Restoring
  /// the bookmark may queue loading of such curves. To this end, such @c PlotInstance data are
  /// instead loaded into the @p curveDataMap (if provided). Settings are keyed as follows:
  /// - {source-name}|{curve-name}|{property}
  ///
  /// The following @c PlotInstance properties may be retrieved from @c curveDataMap:
  /// - colour - a @c QRgb integer presenting the curve colour. Only present if it requires
  ///   an explicit colour.
  /// - style : the @c PlotInstance::style()
  /// - width : the @c PlotInstance::width()
  /// - symbol : the @c PlotInstance::symbol()
  /// - symbol-size : the @c PlotInstance::symbolSize()
  /// - filter-inf : the @c PlotInstance::filterInf()
  /// - filter-nan : the @c PlotInstance::filterNaN()
  ///
  /// @param settings The settings object to restore the bookmark from.
  /// @param id The ID of the bookmark to restore.
  /// @param ui The application.
  /// @param curveDataMap Optional map for @c PlotInstance curve data. See description.
  /// @return True if the bookmark ID is valid and is being restored.
  bool restoreBookmak(QSettings &settings, unsigned id, OCurvesUI *ui, VariantMap *curveDataMap = nullptr);

  /// Clears a bookmark.
  /// @param settings The settings object to restore the bookmark from.
  /// @param id The ID of the bookmark of interest.
  /// @return True if the bookmark ID was valid.
  bool clearBookmark(QSettings &settings, unsigned id);

  /// Check if bookmark with the given ID exists and is set.
  /// @param settings The settings object to restore the bookmark from.
  /// @param id The ID of the bookmark of interest.
  bool exists(QSettings &settings, unsigned id);

  /// Fetch the name of a bookmark if it exists.
  /// @param[out] name Set to the bookmark name. May be empty even if the bookmark exists.
  ///   Set to an empty string if the bookmark does not exist.
  /// @param settings The settings object to restore the bookmark from.
  /// @param id The ID of the bookmark of interest.
  /// @return True if the bookmark exists.
  bool name(QString &name, QSettings &settings, unsigned id);

  /// Migrate bookmark settings from @c from to @c to.
  /// For import/export.
  void migrate(QSettings &to, QSettings &from, int bookmarkCount);
}

#endif // BOOKMARKS_H_
