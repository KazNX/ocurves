//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "realtimeplot.h"

#include "plotinstance.h"

#include "model/curves.h"

#include "rt/realtimecommspec.h"
#include "rt/realtimeconnection.h"
#include "rt/realtimesourceloader.h"
#include "rt/rtmessage.h"

#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QRegExp>


RealTimePlot::RealTimePlotInfo::RealTimePlotInfo()
  : spec(nullptr)
{
}


RealTimePlot::RealTimePlotInfo::~RealTimePlotInfo()
{
  delete spec;
}


RealTimePlot::RealTimePlot(Curves *curves, const QStringList &connectionFiles)
  : PlotGenerator(curves)
  , _connectionFiles(connectionFiles)
  , _stopRequested(false)
{
}


RealTimePlot::~RealTimePlot()
{
  quit();
  wait();
}


void RealTimePlot::appendLoad(const QStringList &connectionFiles)
{
  QMutexLocker guard(_dataMutex);
  _connectionFiles.append(connectionFiles);
}


bool RealTimePlot::pendingLoad(QStringList &pending)
{
  QMutexLocker guard(_dataMutex);
  if (!_connectionFiles.isEmpty())
  {
    pending = _connectionFiles;
    return true;
  }
  return false;
}


void RealTimePlot::stop()
{
  QMutexLocker guard(_dataMutex);
  _stopRequested = true;
  do
  {
    guard.unlock();
    sleep(0);
    guard.relock();
  }
  while (!_sources.empty());
}


void RealTimePlot::run()
{
  QMutexLocker guard(_dataMutex);
  guard.unlock();

  qint64 startTime = QDateTime::currentMSecsSinceEpoch();

  loadSpecs(startTime);

  while (!_abortFlag)
  {
    std::vector<double> sampleLine;
    guard.relock();

    if (_stopRequested)
    {
      for (auto iter = _sources.begin(); iter != _sources.end();)
      {
        RealTimePlotInfo *rtplot = *iter;
        rtplot->spec->disconnect();
        for (unsigned i = 0; i < rtplot->source->curveCount(); ++i)
        {
          if (PlotInstance *plot = rtplot->source->curve(i))
          {
            _curves->completeLoading(plot);
          }
        }
        iter = _sources.erase(iter);
        delete rtplot;
      }
      _sources.clear();
      _stopRequested = false;
    }

    for (auto iter = _sources.begin(); iter != _sources.end();)
    {
      RealTimePlotInfo *rtplot = *iter;

      if (rtplot->spec->connection()->isConnected())
      {
        // Try get new samples.
        rtplot->spec->connection()->read(rtplot->readBuffer);
        RTMessage *msg = rtplot->spec->incomingMessage();

        int bytesRead = 0;
        while ((bytesRead = msg->readMessage(rtplot->readBuffer)) > 0)
        {
          // Trim the buffer of the processed data
          rtplot->readBuffer = rtplot->readBuffer.right(rtplot->readBuffer.size() - bytesRead);

          unsigned sampleCount = msg->populateValues(sampleLine);

          // Do we need to create plots based on the first data sample?
          if (rtplot->source->curveCount() == 0)
          {
            // Create plots.
            if (!msg->headings().empty())
            {
              createPlots(*rtplot, msg->headings());
            }
            else
            {
              createPlots(*rtplot, sampleCount);
            }
          }

          double time = 1e-3 * (QDateTime::currentMSecsSinceEpoch() - startTime);
          unsigned limit = std::min<unsigned>(rtplot->source->curveCount(), sampleCount);
          for (unsigned i = 0; i < limit; ++i)
          {
            if (PlotInstance *plot = rtplot->source->curve(i))
            {
              plot->addPoint(QPointF(time, sampleLine[i]));
            }
          }
        }

        // Clear the buffer on error, or if too large without reading any data.
        if (bytesRead < 0 || rtplot->readBuffer.size() >= MAX_READ_BUFFER_SIZE)
        {
          rtplot->readBuffer.clear();
        }

        ++iter;
      }
      else
      {
        // Disconnected. Remove.
        rtplot->spec->disconnect();
        for (unsigned i = 0; i < rtplot->source->curveCount(); ++i)
        {
          if (PlotInstance *plot = rtplot->source->curve(i))
          {
            _curves->completeLoading(plot);
          }
        }
        iter = _sources.erase(iter);
        delete rtplot;
      }
    }
    guard.unlock();
  }
}


unsigned RealTimePlot::loadSpecs(double startTime)
{
  QMutexLocker guard(_dataMutex);

  if (_connectionFiles.isEmpty())
  {
    return 0;
  }

  QStringList connectionFiles = _connectionFiles;
  _connectionFiles.clear();

  guard.unlock();

  unsigned loaded = 0;
  if (!connectionFiles.empty())
  {
    for (const QString &file : connectionFiles)
    {
      if (RealTimePlotInfo *rtplot = loadCommFile(file))
      {
        rtplot->source->setTimeBase(startTime);
        ++loaded;
      }
    }
  }

  return loaded;
}


RealTimePlot::RealTimePlotInfo *RealTimePlot::loadCommFile(const QString &filePath)
{
  QFile file(filePath);
  file.open(QFile::ReadOnly);

  if (!file.isOpen())
  {
    // TODO: log error
    return nullptr;
  }

  RealTimeSourceLoader loader;
  RealTimeCommSpec *spec = loader.load(filePath);
  if (!spec)
  {
    // TODO: log error
    return nullptr;
  }

  RealTimePlotInfo *rtplot = new RealTimePlotInfo;
  rtplot->spec = spec;
  rtplot->source = new PlotSource(PlotSource::RealTime, spec->connection()->name());

  if (!spec->incomingMessage()->headings().empty())
  {
    // Have headings. Create plots.
    createPlots(*rtplot, spec->incomingMessage()->headings());
  }
  // else no headings. Create plots by first sample line.

  rtplot->source->setTimeColumn(rtplot->spec->timeColumn());
  rtplot->source->setTimeScale(rtplot->spec->timeScale());
  _sources.push_back(rtplot);

  return rtplot;
}


void RealTimePlot::createPlots(RealTimePlotInfo &rtplot, unsigned count, const QStringList *headings)
{
  //rtplot.plots.resize(count);
  emit beginNewCurves();
  for (unsigned i = 0; i < count; ++i)
  {
    PlotInstance *plot = new PlotInstance(rtplot.source);
    plot->setExplicitTime(true);
    rtplot.source->addCurve(plot);
    plot->setName((headings) ? (*headings)[i] : QString("Column %1").arg(i));
    plot->makeRingBuffer(rtplot.spec->bufferSize() ? rtplot.spec->bufferSize() : 2000000);

    _curves->newCurve(plot);
  }
  emit endNewCurves();
}


void RealTimePlot::createPlots(RealTimePlotInfo &rtplot, const QStringList &headings)
{
  createPlots(rtplot, headings.count(), &headings);
}
