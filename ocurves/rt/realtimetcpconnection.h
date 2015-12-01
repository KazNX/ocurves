//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REALTIMETCPCONNECTION_H_
#define REALTIMETCPCONNECTION_H_

#include "ocurvesconfig.h"

#include "realtimeconnection.h"

#include <QHostAddress>
#include <vector>

class QTcpSocket;

/// @ingroup realtime
/// A @c RealTimeSource which reads from a TCP socket.
class RealTimeTcpConnection : public RealTimeConnection
{
public:
  /// Create an unconnected TCP connection.
  RealTimeTcpConnection();

  /// Destructor, ensures @c disconnect() is called.
  ~RealTimeTcpConnection();

  /// The TCP address being used.
  /// @return The TCP connection name.
  virtual QString name() const override;

  /// Open a TCP connection to the indicated address and port.
  /// Closes the existing connection as required.
  ///
  /// This function may block for up to the specified @p timeout,
  /// attempting to establish a connection. After this time has elapsed, the
  /// connection is deemed to have failed.
  ///
  /// @param address The target address.
  /// @param port The connection port.
  /// @param timeout The connection wait timeout (millisecond).
  /// @return True if the connection is successfully established.
  bool open(const QHostAddress &address, quint16 port, uint timeout = 5000);

  /// Is the port open and connected?
  /// @return True if open.
  virtual bool isConnected() const override;

  /// Disconnect if connected.
  virtual void disconnect() override;

  /// Send the given buffer to the source.
  /// @param buffer The buffer to send.
  int send(const QByteArray &buffer) override;

  /// Read data into the given buffer.
  /// @param buffer The buffer to read into.
  int read(QByteArray &buffer) override;

private:
  QTcpSocket *_socket;            ///< TCP connection.
};

#endif // REALTIMETCPCONNECTION_H_
