//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REALTIMESERIALCONNECTION_H_
#define REALTIMESERIALCONNECTION_H_

#include "ocurvesconfig.h"

#include "realtimeconnection.h"

#include <QHostAddress>
#include <vector>

class QSerialPort;

/// @ingroup realtime
/// A @c RealTimeConnection which reads from a serial port.
class RealTimeSerialConnection : public RealTimeConnection
{
public:
  /// Create an unconnected serial connection.
  RealTimeSerialConnection();

  /// Destructor, ensures @c disconnect() is called.
  ~RealTimeSerialConnection();

  /// The name of the serial port being used.
  /// @return The serial port name.
  virtual QString name() const override;

  /// Open a serial connection, first disconnecting any existing connection.
  /// @param name The name of the serial port to open. For example, "/dev/ttyUSB0" or "COM3".
  /// @param baudRate The baud rate at which to open the port.
  /// @param readOnly True to open the connection is a read only mode.
  /// @return True on success, false on failure.
  bool open(const QString &name, quint32 baudRate, bool readOnly);

  /// Is the port open and connected?
  /// @return True if open.
  bool isConnected() const override;

  /// Disconnect if connected.
  void disconnect() override;

  /// Send the given buffer to the source.
  /// @param buffer The buffer to send.
  int send(const QByteArray &buffer) override;

  /// Read data into the given buffer.
  /// @param buffer The buffer to read into.
  int read(QByteArray &buffer) override;

private:
  QSerialPort *_port;             ///< Port implementation.
};

#endif // REALTIMESERIALCONNECTION_H_
