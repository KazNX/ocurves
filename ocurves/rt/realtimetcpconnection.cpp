//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "realtimetcpconnection.h"

#include "plotfile.h"

#include <QTcpSocket>

RealTimeTcpConnection::RealTimeTcpConnection()
  : _socket(nullptr)
{
}


RealTimeTcpConnection::~RealTimeTcpConnection()
{
  disconnect();
}


QString RealTimeTcpConnection::name() const
{
  return _socket ? _socket->peerAddress().toString() : "???";
}


bool RealTimeTcpConnection::open(const QHostAddress &address, quint16 port, uint timeout)
{
  disconnect();
  _socket = new QTcpSocket;
  _socket->connectToHost(address, port);
  return _socket->waitForConnected(timeout);
}


bool RealTimeTcpConnection::isConnected() const
{
  QAbstractSocket::SocketState state = _socket->state();
  return _socket && _socket->isOpen();
}


void RealTimeTcpConnection::disconnect()
{
  if (_socket)
  {
    _socket->close();
    delete _socket;
    _socket = nullptr;
  }
}


int RealTimeTcpConnection::send(const QByteArray &buffer)
{
  if (!_socket || _socket->state() != QAbstractSocket::ConnectedState)
  {
    return -1;
  }

  return int(_socket->write(buffer));
}


int RealTimeTcpConnection::read(QByteArray &buffer)
{
  if (!_socket || _socket->state() != QAbstractSocket::ConnectedState)
  {
    return -1;
  }

  _socket->waitForReadyRead(0);
  QByteArray newBytes = _socket->readAll();
  if (newBytes.size())
  {
    buffer.append(newBytes);
    return newBytes.size();
  }

  return 0;
}
