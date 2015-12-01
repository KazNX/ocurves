//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTFILELOADER_H_
#define PLOTFILELOADER_H_

#include "ocurvesconfig.h"

#include "plotgenerator.h"

#include "timesampling.h"

#include <QStringList>

/// @ingroup gen
/// A plot generator which loads data from CSV style text files.
///
/// Data loading is supported by the @c PlotFile class. See that class for more details
/// of how data are loaded.
///
/// The file loader may optionally down-sample the data file by skipping data lines.
/// This is controlled by the @c targetSampleCount(). A crude estimate is made of the
/// number of lines in the data file and lines are skipped to attempt to meet the target.
/// The line count estimate is made by determining the length of the first data line and
/// dividing the file by this value. All samples are loaded if the estimate does not
/// exceed the target.
///
/// The first and last data lines are always preserved.
///
/// General usage is to construct the generator with the files to load,
/// the @c start() the thread. Additional files may be queued using @c append(),
/// but a new loader is required if this call fails.
///
/// Each file may optionally be given its own @c TimeSampling to set the time
/// column, time scale and base time.
class PlotFileLoader : public PlotGenerator
{
  Q_OBJECT
public:
  /// Creates a loader for the given file set.
  /// @param curves The @c Curves model to add curves to.
  /// @param plotFiles The list of files to load.
  /// @param timing Optionally specifying timing data for each file.
  ///   Uses as many as it can, attempting to match 1-to-1 with @p plotFiles,
  ///   but supports element count mismatches. Falls back to the generator's
  ///   general time settings.
  PlotFileLoader(Curves *curves, const QStringList &plotFiles, QVector<TimeSampling> *timing = nullptr);

  /// Append a list of files to the currently loading list of files.
  ///
  /// This extends the files to load as if originally given to the constructor.
  /// The call is thread-safe and extends the list before or during loading,
  /// but fails if the loading thread has progressed beyond the load loop.
  ///
  /// @param plotFiles The additional files to load.
  /// @param timing Optional timing data for the plot files. See constructor.
  /// @return True if the file list has been added, false if the thread
  ///   has already completed loading.
  bool append(const QStringList &plotFiles, QVector<TimeSampling> *timing = nullptr);

  /// Set the target maximum number of sample points in a curve. This limits the
  /// number of loaded samples for large files.
  /// @param target The target sample count.
  inline void setTargetSampleCount(uint target) { _targetSampleCount = target; }

  /// Access the target sample count.
  /// @return The target number of samples per @c PlotInstance.
  inline uint targetSampleCount() const { return _targetSampleCount; }

  /// True.
  /// @return true.
  virtual inline bool isFileLoad() const override { return true; }

protected:
  /// File loading implementation.
  void run() override;

private:
  /// Load file data from @p filePath.
  /// @param filePath The path to the file to load, directory and file name.
  /// @param fileName Just the file name part of the file path.
  /// @param fileNumber A progress value, indicating the number of this file
  ///   among all the files to be loaded (for reference). First file is 1, next is 2...
  /// @param totalFileCount Total number of files being processed (for reference).
  /// @return The number of curves added from the given file, or zero on error or
  ///     if loading has been aborted.
  size_t loadFile(const QString &filePath, const QString &fileName, int fileNumber, int totalFileCount, const TimeSampling &timing);

  /// Calculates an estimated sample rate for sub-sampling the file.
  /// Supports @c targetSampleCount().
  ///
  /// The file position is reset to the start after calling because
  /// if seek issues with MSC and text mode files (newline handling
  /// throws the position).
  ///
  /// @return The number of lines per sample to record.
  double calculateSampleRate(PlotFile &file);

  QStringList _plotFiles; ///< List of files to load.
  /// @c TimeSampling details for each plot file. Contains one entry for each item in
  /// @c _plotFiles. Explicitly tracked to support late additions via @c append().
  QVector<TimeSampling> _plotTiming;
  uint _targetSampleCount;  ///< Target samples per @c PlotInstance.
  bool _loadComplete;       ///< True when the loading loop has completed.
};

#endif // PLOTFILELOADER_H_
