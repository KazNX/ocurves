//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "bookmarks.h"

#include "model/curves.h"
#include "model/expressions.h"

#include "ui/expressionsview.h"
#include "ui/ocurvesui.h"
#include "ui/splitplotview.h"

#include "plotinstance.h"
#include "plotsource.h"
#include "timesampling.h"

#include <QHash>
#include <QSettings>

namespace bookmarks
{
  bool requiresStorage(const PlotInstance &curve)
  {
    if (curve.flags() & (PlotInstance::ExplicitColour | PlotInstance::FilterNaN | PlotInstance::FilterInf))
    {
      return true;
    }

    if (curve.width() > 0 || curve.style() > 0 || curve.symbol() >= 0 || curve.symbolSize() != PlotInstance::DefaultSymbolSize)
    {
      return true;
    }

    return false;
  }


  /// Save @c PlotInstance settings to @c QSettings.
  void saveCurveSettings(QSettings &settings, const PlotInstance &curve)
  {
    settings.setValue("name", curve.name());
    if (curve.explicitColour())
    {
      settings.setValue("colour", curve.colour());
    }
    else
    {
      settings.remove("colour");
    }

    settings.setValue("style", curve.style());
    settings.setValue("width", curve.width());
    settings.setValue("symbol", curve.symbol());
    settings.setValue("symbol-size", curve.symbolSize());

    settings.setValue("filter-inf", curve.filterInf());
    settings.setValue("filter-nan", curve.filterNaN());
  }


  /// Load curve settings to a data map. Can't load to a @c PlotInstance as it may not yet be loaded.
  void loadCurveSettings(QSettings &settings, const QString &sourceName, VariantMap &map)
  {
    QString key("%1|%2|%3");
    bool ok = false;
    QRgb colour;

    QString name = settings.value("name").toString();
    key = key.arg(sourceName).arg(name);
    colour = settings.value("colour").toUInt(&ok);
    if (ok)
    {
      map.insert(key.arg("colour"), colour);
    }

    map.insert(key.arg("style"), settings.value("style"));
    map.insert(key.arg("width"), settings.value("width"));
    map.insert(key.arg("symbol"), settings.value("symbol"));
    map.insert(key.arg("symbol-size"), settings.value("symbol-size"));
    map.insert(key.arg("filter-inf"), settings.value("filter-inf"));
    map.insert(key.arg("filter-nan"), settings.value("filter-nan"));
  }


  void setBookmark(QSettings &settings, unsigned id, const QString &name, OCurvesUI *ui, bool includeInactiveSources)
  {
    QStringList activeSources, activeCurves;

    settings.beginGroup(QString("bookmark%1").arg(id));
    settings.setValue("set", true);
    settings.setValue("name", name);

    if (!includeInactiveSources)
    {
      ui->splitPlotView()->collateActive(activeSources, activeCurves);
    }

    // Save active sources and their settings.
    QList<PlotSource *> sources;
    ui->curves()->enumerateSources(sources, PlotSource::File);
    int sourceCount = (includeInactiveSources) ? sources.count() : activeSources.count();
    settings.beginWriteArray("file-sources", sourceCount);
    int i = 0;
    for (PlotSource *source : sources)
    {
      if (includeInactiveSources || activeSources.contains(source->name()))
      {
        settings.setArrayIndex(i++);
        settings.setValue("name", source->fullName());
        settings.setValue("short-name", source->name());
        settings.setValue("column", source->timeColumn());
        settings.setValue("base", source->timeBase());
        settings.setValue("scale", source->timeScale());

        // Write curve properties for curves with modified properties.
        settings.beginWriteArray("curves");
        int ci = 0;
        for (unsigned c = 0; c < source->curveCount(); ++c)
        {
          const PlotInstance *curve = source->curve(c);
          if (requiresStorage(*curve))
          {
            settings.setArrayIndex(ci++);
            saveCurveSettings(settings, *curve);
          }
        }
        settings.endArray();
      }
    }
    settings.endArray();

    // Save UI settings.
    ui->saveSettings(settings);
    ui->splitPlotView()->saveSettings(settings);
    settings.endGroup();
  }


  void setBookmark(QSettings &settings, unsigned id, OCurvesUI *ui, bool includeInactiveSources)
  {
    setBookmark(settings, id, QString(), ui, includeInactiveSources);
  }


  bool restoreBookmak(QSettings &settings, unsigned id, OCurvesUI *ui, VariantMap *curveDataMap)
  {
    QString name = QString("bookmark%1").arg(id);
    settings.beginGroup(name);
    bool set = settings.value("set", false).toBool();
    if (set)
    {
      QStringList restoreFiles;
      QVector<TimeSampling> timeSamplingArray;

      // Load and restore file sources.
      int fileCount = settings.beginReadArray("file-sources");
      timeSamplingArray.resize(fileCount);
      for (int i = 0; i < fileCount; ++i)
      {
        settings.setArrayIndex(i);
        QString sourceName = settings.value("name").toString();
        restoreFiles.append(sourceName);
        QString shortName = settings.value("short-name").toString();
        TimeSampling &timing = timeSamplingArray[i];
        timing.column = settings.value("column", 0).toUInt();
        timing.base = settings.value("base", 0.0).toDouble();
        timing.scale = settings.value("scale", 0.0).toDouble();
        timing.flags = 0;

        // Load instance settings into the data map.
        if (curveDataMap)
        {
          int plotCount = settings.beginReadArray("curves");
          for (int i = 0; i < plotCount; ++i)
          {
            settings.setArrayIndex(i);
            loadCurveSettings(settings, shortName, *curveDataMap);
          }
          settings.endArray();
        }
      }
      settings.endArray();

      // Update existing and collate not yet loaded.
      QList<PlotSource *> existingSources;
      ui->curves()->enumerateSources(existingSources, PlotSource::File);

      for (PlotSource *source : existingSources)
      {
        int index = restoreFiles.indexOf(source->fullName());
        if (index != -1)
        {
          TimeSampling timing = timeSamplingArray[index];
          restoreFiles.removeAt(index);
          timeSamplingArray.removeAt(index);

          source->setTimeColumn(timing.column);
          source->setTimeBase(timing.base);
          source->setTimeScale(timing.scale);

          ui->curves()->invalidate(source);
        }
      }

      ui->loadSettings(settings);
      ui->splitPlotView()->loadSettings(settings);

      if (!restoreFiles.isEmpty())
      {
        ui->load(restoreFiles, true, &timeSamplingArray);
      }
    }
    settings.endGroup();

    return set;
  }


  bool clearBookmark(QSettings &settings, unsigned id)
  {
    bool wasSet = false;
    settings.beginGroup(QString("bookmark%1").arg(id));
    if (!settings.value("set").isNull())
    {
      wasSet = settings.value("set", false).toBool();
      settings.setValue("set", false);
    }
    settings.endGroup();
    return wasSet;
  }


  bool exists(QSettings &settings, unsigned id)
  {
    bool available = false;
    settings.beginGroup(QString("bookmark%1").arg(id));
    if (!settings.value("set").isNull())
    {
      available = settings.value("set").toBool();
    }
    settings.endGroup();

    return available;
  }


  bool name(QString &name, QSettings &settings, unsigned id)
  {
    bool available = false;
    name = QString();
    settings.beginGroup(QString("bookmark%1").arg(id));
    if (!settings.value("set").isNull())
    {
      available = settings.value("set").toBool();
      if (available)
      {
        name = settings.value("name").toString();
      }
    }
    settings.endGroup();

    return available;
  }


  void migrate(QSettings &to, QSettings &from, int bookmarkCount)
  {
    for (int i = 0; i < bookmarkCount; ++i)
    {
      QString group = QString("bookmark%1").arg(i);
      from.beginGroup(group);
      to.beginGroup(group);

      bool fromSet = from.value("set", false).toBool();

      // Ensure 'set' is false if present in to, regardless of presence in from.
      if (!to.value("set").isNull() && !fromSet)
      {
        to.setValue("set", false);
      }

      for (const QString &key : from.allKeys())
      {
        to.setValue(key, from.value(key));
      }

      from.endGroup();
      to.endGroup();
    }
  }
}
