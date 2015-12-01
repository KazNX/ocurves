//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REALTIMESOURCE_H_
#define REALTIMESOURCE_H_

#include "ocurvesconfig.h"

#include <QString>
#include <QVector>

/// @ingroup realtime
/// This is the base class for data source which provides data in real time.
class RealTimeConnection
{
public:
  /// Virtual destructor.
  virtual inline ~RealTimeConnection() {}

  /// The source/connection name.
  /// @return A name for the source/connection.
  virtual QString name() const = 0;

  /// Returns true if the source is connected.
  /// @return True if connected.
  virtual bool isConnected() const = 0;

  /// Request to disconnect the source.
  virtual void disconnect() = 0;

  /// Send the given buffer to the source.
  /// @param buffer The buffer to send.
  virtual int send(const QByteArray &buffer) = 0;

  /// Read data into the given buffer.
  /// @param buffer The buffer to read into.
  virtual int read(QByteArray &buffer) = 0;
};


#endif // REALTIMESOURCE_H_
