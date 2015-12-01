//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REALTIMEUDPCONNECTION_H_
#define REALTIMEUDPCONNECTION_H_

#include "ocurvesconfig.h"

#include "realtimeconnection.h"

#include <QHostAddress>
#include <vector>

class QUdpSocket;

/// @ingroup realtime
/// A @c RealTimeSource which reads from a UDP socket.
class RealTimeUdpConnection : public RealTimeConnection
{
public:
  /// Create an unconnected UDP connection.
  RealTimeUdpConnection();

  /// Destructor, ensures @c disconnect() is called.
  ~RealTimeUdpConnection();

  /// The UDP address being used.
  /// @return The UDP connection name.
  virtual QString name() const override;

  /// Open a UDP connection to the indicated address and port.
  /// Closes the existing connection as required.
  ///
  /// This function may block for up to the specified @p timeout,
  /// attempting to establish a connection. After this time has elapsed, the
  /// connection is deemed to have failed.
  ///
  /// @param address The target address.
  /// @param port The connection port.
  /// @return True if the connection is successfully established.
  bool open(const QHostAddress &address, quint16 port);

  /// Is the port open for communication?
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
  QUdpSocket *_socket;
  QHostAddress _address;
  quint16 _port;
};

#endif // REALTIMEUDPCONNECTION_H_
