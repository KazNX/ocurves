//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "realtimeserialconnection.h"

#include "plotfile.h"

#include <QSerialPort>

RealTimeSerialConnection::RealTimeSerialConnection()
  : _port(nullptr)
{
}


RealTimeSerialConnection::~RealTimeSerialConnection()
{
  disconnect();
}


QString RealTimeSerialConnection::name() const
{
  return _port ? _port->portName() : "???";
}


bool RealTimeSerialConnection::open(const QString &name, quint32 baudRate, bool readOnly)
{
  disconnect();
  _port = new QSerialPort;
  _port->setPortName(name);
  _port->setBaudRate(baudRate);
  QSerialPort::OpenMode mode = (readOnly) ? QSerialPort::ReadOnly : QSerialPort::ReadWrite;
  return _port->open(mode);
}


bool RealTimeSerialConnection::isConnected() const
{
  return _port && _port->isOpen();
}


void RealTimeSerialConnection::disconnect()
{
  if (_port)
  {
    _port->close();
    delete _port;
    _port = nullptr;
  }
}


int RealTimeSerialConnection::send(const QByteArray &buffer)
{
  if (!_port || !_port->isOpen())
  {
    return -1;
  }

  return _port->write(buffer);
}


int RealTimeSerialConnection::read(QByteArray &buffer)
{
  if (!_port || !_port->isOpen())
  {
    return 0;
  }

  QByteArray newBytes = _port->readAll();
  if (newBytes.size())
  {
    buffer.append(newBytes);
    return newBytes.size();
  }

  return 0;
}
