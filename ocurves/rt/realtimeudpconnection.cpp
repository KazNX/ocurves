//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "realtimeudpconnection.h"

#include "plotfile.h"

#include <QUdpSocket>

RealTimeUdpConnection::RealTimeUdpConnection()
  : _socket(nullptr)
  , _port(0)
{
}


RealTimeUdpConnection::~RealTimeUdpConnection()
{
  disconnect();
}


QString RealTimeUdpConnection::name() const
{
  return _address.toString();
}


bool RealTimeUdpConnection::open(const QHostAddress &address, quint16 port)
{
  disconnect();
  _socket = new QUdpSocket;
  _address = address;
  _port = port;
  return _socket->bind(port);
}


bool RealTimeUdpConnection::isConnected() const
{
  return _socket;
}


void RealTimeUdpConnection::disconnect()
{
  if (_socket)
  {
    _socket->close();
    delete _socket;
    _socket = nullptr;
  }
}


int RealTimeUdpConnection::send(const QByteArray &buffer)
{
  if (!_socket)
  {
    return -1;
  }

  return _socket->writeDatagram(buffer, _address, _port);
}


int RealTimeUdpConnection::read(QByteArray &buffer)
{
  if (!_socket)
  {
    return -1;
  }

  const unsigned BufferSize = 1024;
  char bytes[BufferSize];
  int read = int(_socket->readDatagram(bytes, BufferSize));
  if (read > 0)
  {
    buffer.append(bytes, read);
  }
  return read;
}
