//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotfile.h"

#include <QRegExp>

namespace
{
  // Regular expression: float/double.
#define RE_FLOAT "-?(\\d+\\.\\d*)|(\\d*\\.\\d+)"
  // Regular expression: float/double in scientific notation (exponent).
#define RE_EXP "-?(([0-9]+\\.?[0-9]*)|([0-9]*\\.[0-9]+))[eE][-\\+]?[0-9]+"
  // Regular expression: integer
#define RE_INT "-?\\d+"
  bool looksLikeHeadings(const QStringList &headings)
  {
    QRegExp isNumeric("(" RE_FLOAT ")|(" RE_INT ")|(" RE_EXP ")");
    bool allNumeric = true;
    foreach (const QString &str, headings)
    {
      allNumeric = allNumeric && isNumeric.exactMatch(str);
    }

    return !allNumeric;
  }
}


PlotFile::PlotFile(const QString &fileName, unsigned options)
  : _file(fileName)
  , _options(options)
{
  _file.open(QIODevice::ReadOnly | QIODevice::Text);
  _stream.setAutoDetectUnicode(true);
  _stream.setDevice(&_file);
}


PlotFile::~PlotFile()
{
}


bool PlotFile::isOpen() const
{
  return _file.isOpen();
}


qint64 PlotFile::fileSize() const
{
  return _file.size();
}


qint64 PlotFile::filePos() const
{
  return _file.pos();
}


qint64 PlotFile::streamPos() const
{
  return _stream.pos();
}


void PlotFile::streamSeek(qint64 pos)
{
  _stream.seek(pos);
}


bool PlotFile::readLine()
{
  _line = _stream.readLine();
  return _line.size() != 0;
}


bool PlotFile::generateHeadings(QStringList &headings)
{
  // Read first line to ascertain headings.
  streamSeek(0);
  readLine();
  QString headingLine = currentLine();

  if (headingLine.isEmpty())
  {
    return false;
  }

  QRegExp separator("[ \t,\n]+");
  // Try deducing the "columns".
  headings = headingLine.split(separator, QString::SkipEmptyParts);

  int columnCount = headings.count();
  // Check the line to see if it contains numeric values or if it looks line
  // headings.
  if (!looksLikeHeadings(headings))
  {
    // Rewind the file pointer.
    streamSeek(0);
    headings.clear();
    // Create columns.
    for (int i = 0; i < columnCount; ++i)
    {
      headings.push_back(QString("Column %1").arg(i + 1));
    }
    return true;
  }

  // Looks like we have a line of headings. However, the column headings
  // may contain spaces, so lets do something "clever":
  // 1. Get the column count from the second line (first data line).
  // 2. If this doesn't match the first line's "column" count, then try:
  //    a. Allow tab delimited columns, but not space delimited.
  //    b. Allow column names to finish with some bracketed text, assuming a
  //       unit specification - e.g., "vNorth (m/s)"
  readLine();
  QString dataLine = currentLine();
  // Rewind to just beyond the headings
  streamSeek(0);
  readLine();

  int targetColumns = dataLine.split(separator, QString::SkipEmptyParts).count();

  if (targetColumns == headings.count())
  {
    // Column count from first line matches data line. We can quit now.
    return true;
  }

  // Potential discrepancy. Try no space delimiter for column headings.
  QRegExp trialSeparator("[\t,\n]+");
  headings = headingLine.split(trialSeparator, QString::SkipEmptyParts);

  if (targetColumns == headings.count())
  {
    // That seems to have done the trick.
    return true;
  }

  // Final attempt: allow space delimiter, but only if not followed by a bracket.
  trialSeparator = QRegExp("(([\\t,\\n]+)|( +(?=[^\\(])))");
  headings = headingLine.split(trialSeparator, QString::SkipEmptyParts);

  if (targetColumns == headings.count())
  {
    // Finally found it.
    return true;
  }

  // Can't match data line column count to headings line count. Just use the
  // default separator.
  headings = headingLine.split(separator, QString::SkipEmptyParts);

  return true;
}


const QString &PlotFile::currentLine() const
{
  return _line;
}


namespace
{
  void addValue(QLocale &locale, std::vector<double> &data, size_t &index, const QString &dataLine, QChar *start, QChar *curr, QChar *end)
  {
    // Found white space. Convert data.

    // QStringRef performance is pretty good, but on Windows
    // we can do better by casting QChar to wchar_t and using wcstod()
    // because they are both 16-bit unicode. On other platforms we can have
    // different unicode widths and we can't use wcstod().
    // ### Disabled use of wcstod() because it may yield different results esp. wrt/ NaN and INF. ###
//#ifndef WIN32
    QStringRef dataStr(&dataLine, curr - start, end - curr);
    double value = locale.toDouble(dataStr);
//#else  // WIN32
//    // Validate wchar_t and QChar compatibility.
//    static_assert(sizeof(wchar_t) == sizeof(QChar), "Size of QChar and wchar_t must match to use this code.");
//    QChar restore = *end;
//    *end = '\0';
//    double value = wcstod((const wchar_t *)curr, nullptr);
//    *end = restore;
//#endif // WIN32
    if (index < data.size())
    {
      data[index++] = value;
    }
    else
    {
      data.push_back(value);
      ++index;
    }
  }
}


void PlotFile::dataLine(std::vector<double> &data)
{
  return dataLine(_line, data, (_options & OptUseLocale) != 0);
}


void PlotFile::dataLine(QString &line, std::vector<double> &data, bool useSystemLocale)
{
  size_t added = 0;
  QChar *start = line.data();
  QChar *ch = line.data();
  QChar *item = line.data();
  QLocale locale = (useSystemLocale) ? QLocale::system() : QLocale(QLocale::C);

  while (ch->unicode())
  {
    switch (ch->unicode())
    {
    case ' ':
    case '\t':
    case ',':
    case '\n':
    case '\r':
      if (item < ch)
      {
        addValue(locale, data, added, line, start, item, ch);
        item = ch + 1;
      }
      else
      {
        item = ch + 1;
      }
      break;
    default:
      break;
    }
    ++ch;
  }

  if (item && item->unicode() != '\0')
  {
    addValue(locale, data, added, line, start, item, ch);
  }

  if (data.size() > added)
  {
    data.resize(added);
  }
}
