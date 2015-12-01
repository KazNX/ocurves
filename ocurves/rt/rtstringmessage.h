//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef RTSTRINGMESSAGE_H_
#define RTSTRINGMESSAGE_H_

#include "ocurvesconfig.h"

#include "rtmessage.h"

#include <QString>

/// @ingroup realtime
/// Represents a string based message for real-time data plots.
///
/// A string message is simply parsed as a delimited value string. Parsing
/// matches that used for file loading; @c PlotFile::dataLine().
///
/// A real time source does not expect heading names, and by default simply
/// labels values "Column 1", "Column 2", etc. This can be overridden by
/// calling @c setHeadings(), but the number of items must match the incoming
/// data.
///
/// As with a data file, each line is expected to have the same number of items.
class RTStringMessage : public RTMessage
{
public:
  /// Constructor, sending @p message() when @c setMesssage() is called.
  RTStringMessage(const QString &message = QString());

  /// Set the message to send in @p buffer.
  ///
  /// Write the message given on construction.
  /// @param buffer to populate.
  void setMessage(QByteArray &buffer) override;

  /// Read incoming message data. Simply converted to string.
  /// @param buffer The buffer to convert.
  /// @return The number of bytes read on success, negative on error. Zero if nothing to read.
  int readMessage(const QByteArray &buffer) override;

  /// Set the headings.
  /// @param headings The new headings.
  inline void setHeadings(const QStringList &headings) { _headings = headings; }

  /// Returns the cache headings.
  ///
  /// Headings are either set explicitly or determined by the first value @p populateValues() call.
  ///
  /// @return The headings if available. Empty otherwise.
  QStringList headings() const override;

  /// Data access request for the latest data.
  ///
  /// This parses the current message, as read by the last @c readMessage()
  /// call. The headings are populate, if not set, to a numerical sequence
  /// 'Column #' to match the number of incoming values.
  ///
  /// @param values Populated with the latest data set.
  /// @return The number of items in @p values.
  unsigned populateValues(std::vector<double> &values) const override;

protected:
  QString _message; ///< Message to send, or last read.
  mutable QStringList _headings;  ///< Cached headings.
};

#endif // RTSTRINGMESSAGE_H_
