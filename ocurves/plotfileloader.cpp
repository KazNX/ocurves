//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotfileloader.h"

#include "plotfile.h"
#include "plotinstance.h"

#include "model/curves.h"

#define TARGET_SAMPLES 20000

#define ITEM_PROGESS_TICKS 1000
#define OVERALL_PROGRESS_FILE_TICKS 100

namespace
{
  /// Returns true if the value can be displayed.
  /// Can display if not NaN and not infinite.
  template <class T>
  bool canDisplay(T value)
  {
    const T maxValue = std::numeric_limits<T>::max();
    // NaN check: value == value
    // Infinite check : value <= max && -value <= max
    return value == value && (value <= maxValue && -value <= maxValue);
  }
}


PlotFileLoader::PlotFileLoader(Curves *curves, const QStringList &plotFiles, QVector<TimeSampling> *timing)
  : PlotGenerator(curves)
  , _plotFiles(plotFiles)
  , _targetSampleCount(TARGET_SAMPLES)
  , _loadComplete(false)
{
  if (timing)
  {
    _plotTiming = *timing;
  }

  int initialTimingSize = _plotTiming.size();
  _plotTiming.resize(_plotFiles.size());

  if (initialTimingSize < _plotTiming.size())
  {
    TimeSampling dummy = { 0, 0, 0, 0 };
    for (int i = initialTimingSize; i < _plotTiming.size(); ++i)
    {
      _plotTiming[i] = dummy;
    }
  }
}


bool PlotFileLoader::append(const QStringList &plotFiles, QVector<TimeSampling> *timing)
{
  QMutexLocker locker(_dataMutex);
  if (!_loadComplete)
  {
    // Loading already done. Appending will do no good.
    _plotFiles.append(plotFiles);

    int initialTimingSize = _plotTiming.size();
    _plotTiming.resize(_plotFiles.size());

    if (initialTimingSize < _plotTiming.size())
    {
      if (timing)
      {
        int i;
        for (i = 0; i < timing->size() && i + initialTimingSize < _plotTiming.size(); ++i)
        {
          _plotTiming[i + initialTimingSize] = (*timing)[i];
        }
        initialTimingSize -= i;
      }

      TimeSampling dummy = { 0, 0, 0, 0 };
      for (int i = initialTimingSize; i < _plotTiming.size(); ++i)
      {
        _plotTiming[i] = dummy;
      }
    }
    return true;
  }

  return false;
}


void PlotFileLoader::run()
{
  QMutexLocker locker(_dataMutex);
  size_t loadCount = 0;
  _loadComplete = false;
  if (!_plotFiles.empty())
  {
    // Populate timing details.
    int plotCount = _plotFiles.size();
    TimeSampling defaultSampling = { _timeColumn, 0, _timeScale, _controlFlags };
    for (int i = 0; i < plotCount; ++i)
    {
      if (_plotTiming[i].scale == 0)
      {
        // Not set. Use default.
        _plotTiming[i] = defaultSampling;
      }
    }

    emit overallProgress(0, _plotFiles.count() * OVERALL_PROGRESS_FILE_TICKS);
    int processedCount = 0;

    // Unwind the plot file load loop to support expanding the list of files.
    auto fileIter = _plotFiles.constBegin();
    auto timeIter = _plotTiming.constBegin();
    while (!_abortFlag && fileIter != _plotFiles.constEnd())
    {
      const QString file = *fileIter;
      const TimeSampling timing = *timeIter;
      const int fileCount = _plotFiles.count();
      // Allow appending new files.
      locker.unlock();

      loadCount += loadFile(file, QFileInfo(file).baseName(), processedCount + 1, fileCount, timing);
      emit overallProgress(++processedCount * OVERALL_PROGRESS_FILE_TICKS, fileCount * OVERALL_PROGRESS_FILE_TICKS);
      ++loadCount;

      // Disallow appending new files as we loop, or prepare to wrap up.
      locker.relock();

      // Increment the iterators. We have to handle the case where the lists have
      // been changed via request to load more files.
      if (fileCount == _plotFiles.count())
      {
        // No change in file list. Increment iterator normally.
        ++fileIter;
        ++timeIter;
      }
      else
      {
        // File list changed. Re-initialise the iterator.
        fileIter = _plotFiles.constBegin() + processedCount;
        timeIter = _plotTiming.constBegin() + processedCount;
      }
    }
  }

  _loadComplete = true;

  // No more data protection required while we emit the completion signal.
  locker.unlock();

  emit loadComplete(int(loadCount));
}


size_t PlotFileLoader::loadFile(const QString &filePath, const QString &fileName, int fileNumber, int totalFileCount, const TimeSampling &timing)
{
  // Use auto_ptr to ensure delete. TODO: change to unique_ptr (c++11) as auto_ptr is deprecated.
  PlotFile file(filePath);

  if (!file.isOpen())
  {
    return 0;
  }

  emit itemName(fileName);

  // Remember, calculateSampleRate(), fileSize() and generateHeadings()
  // all reset the file position to the start. This is because
  // seeking under MSC doesn't work in text mode (new lines are counted
  // incorrectly due to /n vs. \r\n differences).
  int sampleRate = calculateSampleRate(file);
  qint64 fileSize = file.fileSize();
  QStringList headings;
  if (!file.generateHeadings(headings))
  {
    return 0;
  }

  size_t columnCount = headings.count();

  // Create and add new plots for the curves we are loading.
  PlotSource::Ptr source(new PlotSource(PlotSource::File, filePath, unsigned(columnCount)));

  source->deriveName();
  source->setTimeScale(timing.scale);
  // Remember: 1 based index for time column.
  source->setTimeColumn((timing.column <= columnCount) ? timing.column : 0);

  emit beginNewCurves();
  for (unsigned i = 0; !_abortFlag && i < columnCount; ++i)
  {
    PlotInstance *c = new PlotInstance(source);
    c->setName(headings[i].trimmed());  // Ensure new lines are also removed.
    source->addCurve(c);
    _curves->newCurve(c);
  }
  emit endNewCurves();

  int pointIndex = 0;
  qint64 progressIncrement = fileSize;
  progressIncrement = progressIncrement / ITEM_PROGESS_TICKS + !!(progressIncrement % ITEM_PROGESS_TICKS);
  unsigned sample = 0;
  double nextSample = 0;
  double time = 0;
  bool first = true;
  std::vector<double> dataLine;
  qint64 pos = file.filePos();
  while (file.readLine() && !_abortFlag)
  {
    // May not sample every line, but make sure the first and last lines are sampled.
    // The position vs. size test is attempting to estimate the end of file
    // allowing for a trailing newline (or two).
    pos = file.filePos();
    if (++sample >= nextSample || pos >= fileSize - 4)
    {
      nextSample += sampleRate;
      file.dataLine(dataLine);

      size_t indexLimit = qMin(dataLine.size(), columnCount);
      if (timing.column > 0 && timing.column <= dataLine.size())
      {
        time = dataLine[timing.column - 1];
      }
      else
      {
        ++time;
      }

      if (first)
      {
        if (timing.column && (timing.flags & RelativeTime))
        {
          source->setTimeBase(time);
        }
        else
        {
          source->setTimeBase(timing.base);
        }
        first = false;
      }

      for (unsigned i = 0; i < indexLimit; ++i)
      {
        PlotInstance &c = *source->curve(i);
        double value = dataLine[i];
        // Unsuccessful conversion, NaN or infinite results in a zero value for better plotting
        // and range handling.
        value = (canDisplay(value)) ? value : 0.0;
        c.addPoint(QPointF(time, value));
      }

      first = false;

      ++pointIndex;
      if (pointIndex % ITEM_PROGESS_TICKS)
      {
        emit itemProgress(int(pos / progressIncrement));
        const int overallCurrent = (fileNumber - 1) * OVERALL_PROGRESS_FILE_TICKS + (pos / progressIncrement) / (ITEM_PROGESS_TICKS / OVERALL_PROGRESS_FILE_TICKS);
        const int overallTotal = totalFileCount * OVERALL_PROGRESS_FILE_TICKS;
        emit overallProgress(overallCurrent, overallTotal);
      }
    }
  }

  emit itemProgress(int(pos / progressIncrement));
  emit overallProgress(fileNumber * OVERALL_PROGRESS_FILE_TICKS, totalFileCount * OVERALL_PROGRESS_FILE_TICKS);

  // Done reading.
  if (!_abortFlag)
  {
    for (unsigned i = 0; i < columnCount; ++i)
    {
      PlotInstance *c = source->curve(i);
      _curves->completeLoading(c);
    }

    return columnCount;
  }

  for (unsigned i = 0; i < columnCount; ++i)
  {
    PlotInstance *c = source->curve(i);
    _curves->removeCurve(c);
  }
  return 0;
}


double PlotFileLoader::calculateSampleRate(PlotFile &file)
{
  double rate = 1; // Every sample.

  file.streamSeek(0);

  // Read two lines in case the first is headings.
  // Use the line size to estimate the sample count.
  file.readLine();
  file.readLine();
  QString line = file.currentLine();
  qint64 fileSize = file.fileSize();

  int lineLength = line.length();
  if (lineLength && fileSize)
  {
    // Estimate file size.
    qint64 estimatedElementCount = fileSize / lineLength;

    if (_targetSampleCount > 0 && estimatedElementCount > _targetSampleCount)
    {
      rate = double(estimatedElementCount) / _targetSampleCount;
    }
  }

  return rate;
}
