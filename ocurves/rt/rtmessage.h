//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef RTMESSAGE_H_
#define RTMESSAGE_H_

#include "ocurvesconfig.h"

#include <QStringList>

#include <vector>

/// @ingroup realtime
/// Base message handling for real time data plot communications.
///
/// Implementations support reading and writing messages for real-time plot communications.
/// There are two key implementations; RTStringMessage and RTBinaryMessage.
///
/// For sending, only @p setMessage() is called. For reading, the sequence is:
/// - @c readMessage() to populate this instance with data from the message buffer.
/// - @c populateValues() to get the latest values.
/// - @c headings() to determine the headings
///
/// The @c headings() stops once a valid set of headings is returned. This is to support
/// string based messaging where the number of headings is not given in advance, but
/// stops requesting headings once known.
class RTMessage
{
public:
  /// Virtual destructor.
  virtual ~RTMessage();

  /// Called to fill out the send buffer with the contained message.
  /// @param buffer The buffer to write to.
  virtual void setMessage(QByteArray &buffer) = 0;

  /// Called to read incoming data from the given buffer.
  /// @param buffer The buffer to read from.
  /// @return The number of bytes read on success, negative on error. Zero if nothing to read.
  virtual int readMessage(const QByteArray &buffer) = 0;

  /// Request the list of headings for the plot.
  /// @return The headings or an empty list if not yet known.
  virtual QStringList headings() const = 0;

  /// Request the latest values.
  ///
  /// Uses a @c std::vector, not @c QVector because the former can be
  /// cleared without reallocation (at the time of writing).
  ///
  /// @param values Resized and populated to the latest values set.
  /// @return The number of values available.
  virtual unsigned populateValues(std::vector<double> &values) const = 0;
};


#endif // RTMESSAGE_H_
