//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTFILE_H
#define PLOTFILE_H

#include "ocurvesconfig.h"

#include "stringitems.h"

#include <QTextStream>

#include <vector>

#include <QFile>
#include <QString>
#include <QStringList>

/// @ingroup gen
/// The @c PlotFile supports loading of data series from a CSV style text file.
///
/// While data files are CSV like, they can be whitespace delimited. Any number of
/// whitespace characters may separate data values.
///
/// The supported file format details are:
/// - Each line is delimited into distinct sample quantities, where each quantity evolves
///   overtime with one sample per line.
/// - Each line represents a discrete sample point.
/// - The first line is an optional line of column names or headings.
/// - The file may contain a time stamp or time value quantity.
/// - Each line must contain the same number of items.
///
/// The presence or absence of headings is inferred by checking if the first line
/// can be split and converted into numeric values. The line is deemed to be headings
/// should that fail. If there is no heading line, then heading names are generated
/// as "Column 1", "Column 2", etc.
///
/// Heading parsing and delimiting differs from data line parsing. Heading names may contain
/// spaces, with heading parsing using the following rules.
/// - Try parse headings with the same delimiters as data (whitespace and comma).
/// - Parse the next line (first data line) and validate the number of items on each line.
///   We stop if these two match and generate the headings from the first line as is.
/// - Otherwise:
///   -# Allow tab-delimited, but not space delimited columns.
///   -# Allow general whitespace delimiting, but extend names to include any bracketed
///     text which follows it. E.g., "speed (m/s)" is treated as a single heading name.
///
/// Unicode formats are supported through Qt's QTextStream class.
class PlotFile
{
public:
  /// Enumeration of option flags used to control file loading.
  enum OptionFlag
  {
    /// Use the current @c QLocale for parsing rather than the C locale.
    /// The system locale may be set such that the digit separator is '.' and
    /// the decimal separator is ',' as in much of Europe.
    OptUseLocale  = (1 << 0)
  };

  /// Create a plot file opening @c fileName.
  /// @param fileName The file to open.
  /// @param options Combination of @c OptionFlag values to load with.
  PlotFile(const QString &fileName, unsigned options = 0);

  /// Destructor.
  ~PlotFile();

  /// Returns true if a file is currently open.
  /// @return True if the file is open for reading.
  bool isOpen() const;

  /// Returns the overall file size in bytes.
  /// @return The file size in bytes.
  qint64 fileSize() const;

  /// Returns the current buffered byte position in the file.
  ///
  /// This can be ahead of the stream as the stream is buffered, but is
  /// much faster than @c streamPos(). Do not use the result with as
  /// the buffered position is not the read position.
  ///
  /// @return The file position.
  qint64 filePos() const;

  /// Returns the current byte position in the stream.
  ///
  /// This can be a slow operation as the stream may need to re-buffer to
  /// detemine a valid position. Use @c filePos() in tight loops, but
  /// use @c streamPos() when there is a need to reset to the current
  /// file position with @c streamSeek().
  qint64 streamPos() const;

  /// Sets the stream position to @p pos. Do not use with results from
  /// @c filePos(), preferring @c streamPos().
  /// @param pos The position to seek.
  void streamSeek(qint64 pos);

  /// Reads the next line from the file, caching it.
  /// @return True if another line was successfully read, false at end of
  /// file.
  bool readLine();

  /// Generates headings for the plot file. Headings are read from the
  /// first line if possible.
  ///
  /// Headings are deduces from the first line of the file. Headings may
  /// be delimited by either tabs, commas or spaces. Headings may contain
  /// spaces when:
  /// - Headings are tab or comma delimited
  /// - Headings are of the form "heading (unit)". Heading and unit are
  ///   arbitrary strings, but are intended as they are named.
  ///
  /// If no headings line is present, then this method counts columns in
  /// the first line and generates headings as "Column #" where # is the
  /// one base column index.
  ///
  /// The file position is first reset to the start of the file and
  /// is left at the first data line (possibly the start of the file).
  ///
  /// @param headings Populated with headings
  /// @return False if no data was read from the file.
  bool generateHeadings(QStringList &headings);

  /// Returns the last line read from the file (after @c readLine()).
  /// @return The currently cached line as a @c QString.
  const QString &currentLine() const;

  /// Converts the currently cached line into double precision data values.
  ///
  /// The current line is split by the delimiters and @p data populated with
  /// the data items. The @p data array is resize to the number of data items
  /// in the current line. The number of items should be consistent line to line.
  ///
  /// Uses a std::vector because QVector releases memory on @c clear().
  ///
  /// @param data Populated with the data items of the current line.
  void dataLine(std::vector<double> &data);

  /// A static implementation of @c dataLine() above, supporting generalised parsing
  /// of data text.
  ///
  /// Parsing has been optimised for best data conversion speed.
  ///
  /// Uses a std::vector because QVector releases memory on @c clear().
  ///
  /// @param line The data line to parse. Mutable to maximise parsing performance.
  ///   The content is only temporarily modified and restored before exiting.
  /// @param data Populated with the loaded data. The element count is set to the
  ///   number of items parsed from @p line.
  /// @param useSystemLocale Use the system locale for parsing (true), or the C locale (false).
  static void dataLine(QString &line, std::vector<double> &data, bool useSystemLocale);

protected:
  QFile _file;          ///< The file object.
  QTextStream _stream;  ///< Stream wrapping the file object.
  QString _line;        ///< Last read line (@c readLine()).
  unsigned _options;    ///< Option flags.
};

#endif  // PLOTFILE_H
