//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REALTIMECOMMSPEC_H_
#define REALTIMECOMMSPEC_H_

#include "ocurvesconfig.h"

class RealTimeConnection;
class RTMessage;

/// @ingroup realtime
/// Specifies a real time communication specification, messaging and buffer setup.
///
/// The specification collates the @c RealTimeConnection and the @c RTMessage
/// definitions to use with the connection. It also includes expected timing details for
/// the incoming data series.
///
/// The messaging may include three messages:
/// - A specification for the incoming messages (required).
/// - A message to send on establishing a connection (optional).
/// - A message to send just prior to disconnecting (optional).
///
/// The comm-spec may represent either a binary or text-based connection, but this
/// depends entirely on the @c RTMessage objects belonging to the comm-spec.
/// The two primary implementations are @c RTBinaryMessage and @c RTStringMessage.
///
/// This object only serves to collate these messages and the connection, but does not
/// manage the messaging, except that as a convenience, the @c disconnect() function
/// sends the @c disconnectMessage() and calls
/// @ref RealTimeConnection::disconnect() "disconnect" on the connection.
///
/// The code managing the connection communications is responsible for sending the
/// @c connectMessage() message and processing incoming data using @c incommingMessage().
class RealTimeCommSpec
{
public:
  /// Create an empty comm-spec.
  RealTimeCommSpec();

  /// Destroy the comm-spec. Only deletes the wrapped classes (no disconnect).
  ~RealTimeCommSpec();

  /// Set the connection details, deleting and replacing any existing connection.
  /// @param connection The new connection
  void setConnection(RealTimeConnection *connection);

  /// Access the connection. Must not be null for a valid and complete comm-spec.
  /// @return The connection implementation.
  RealTimeConnection *connection() const;

  /// Set the connection message, deleting and replacing any existing connection message.
  ///
  /// The connection message must be sent on establishing a connection. For example, this
  /// may be used to request data streaming begin over the connection.
  /// @param msg The new connection message.
  void setConnectMessage(RTMessage *msg);

  /// Access the current connection message. May be null.
  /// @return The connection message, or null when no such message is required.
  const RTMessage *connectMessage() const;

  /// Set the disconnect message, deleting and replacing any existing disconnect message.
  ///
  /// The connection message must be sent priori to disconnecting. For example, this
  /// may be used to request data streaming begin over the connection.
  /// The @c disconnect() function may be used as a convenience.
  ///
  /// @param msg The new disconnect message.
  void setDisconnectMessage(RTMessage *msg);

  /// Access the current disconnect message. May be null.
  /// @return The disconnect message, or null when no such message is required.
  const RTMessage *disconnectMessage() const;

  /// Set details of the expected incoming messages deleting and replacing any such existing specification.
  ///
  /// The incoming message handles incoming data, converting incoming data to time series values.
  /// See implementations of @c RTMessage for details.
  ///
  /// @param msg The new incoming data message.
  void setIncomingMessage(RTMessage *msg);

  /// Access the current disconnect message. Must not be null for a valid and complete comm-spec.
  /// @return The incoming data message, or null if not set yet.
  RTMessage *incomingMessage() const;

  /// Set the requested plot buffer size for incoming data. This controls the number of
  /// samples to store before discarding the oldest sample.
  ///
  /// The comm-spec only holds this information and does not apply it. It is used by
  /// @ PlotInstance objects to manage a ring buffer of this size.
  ///
  /// @param size The requested buffer size in data samples.
  void setBufferSize(unsigned size);

  /// Access the requested buffer size for incoming sample data buffering.
  /// @return The requested buffer size in data sample elements.
  unsigned bufferSize() const;

  /// Set the expected time column number.
  ///
  /// This identifies the time column for incoming data. It is a one-based index with
  /// a zero value indicating there is no time column.
  ///
  /// @param columnNumber The one-based index for the time column.
  void setTimeColumn(unsigned columnNumber);

  /// Access the time column.
  /// @return The one-based index of the time column, or zero for no time column.
  unsigned timeColumn() const;

  /// Sets the time scaling value.
  ///
  /// All values in the time column are multiplied by this scale before publishing the
  /// actual time value. For example, this may be used to convert from a microsecond
  /// timestamp to seconds by setting the value to 1e-6.
  ///
  /// @param scale The time scaling multiplier. Must not be zero.
  void setTimeScale(double scale);

  /// Access the time scale value.
  /// @see setTimeScale()
  /// @return The time scaling multiplier.
  double timeScale() const;

  /// Send disconnect message and disconnect the connection.
  ///
  /// This is a convenience function for managing disconnection.
  /// Before disconnecting, this function sends the @c disconnectMessage()
  /// if a valid message has been associated with the comm-spec.
  /// The @c RealTimeConnection::disconnect() message is then called.
  ///
  /// The function does nothing if there is no connection or if
  /// @c RealTimeConnection::isConnected() returns false.
  void disconnect();

private:
  RealTimeConnection *_connection;  ///< The connection object. Must not be null.
  RTMessage *_connectMsg;           ///< Message to send on connect. May be null.
  RTMessage *_disconnectMsg;        ///< Message to send on disconnect. May be null.
  RTMessage *_incomingMsg;          ///< Processes incoming data. Must not be null.
  unsigned _bufferSize;             ///< Expected data samples buffer size for associated @c PlotInstance objects.
  unsigned _timeColumn;             ///< 1-base index into the time column. Zero for none.
  double _timeScale;                ///< Scaling value applied to the time column.
};

inline RealTimeConnection *RealTimeCommSpec::connection() const
{
  return _connection;
}


inline const RTMessage *RealTimeCommSpec::connectMessage() const
{
  return _connectMsg;
}


inline const RTMessage *RealTimeCommSpec::disconnectMessage() const
{
  return _disconnectMsg;
}


inline RTMessage *RealTimeCommSpec::incomingMessage() const
{
  return _incomingMsg;
}


inline void RealTimeCommSpec::setBufferSize(unsigned size)
{
  _bufferSize = size;
}


inline unsigned RealTimeCommSpec::bufferSize() const
{
  return _bufferSize;
}


inline void RealTimeCommSpec::setTimeColumn(unsigned columnNumber)
{
  _timeColumn = columnNumber;
}


inline unsigned RealTimeCommSpec::timeColumn() const
{
  return _timeColumn;
}


inline void RealTimeCommSpec::setTimeScale(double scale)
{
  _timeScale = scale;
}


inline double RealTimeCommSpec::timeScale() const
{
  return _timeScale;
}

#endif // REALTIMECOMMSPEC_H_
