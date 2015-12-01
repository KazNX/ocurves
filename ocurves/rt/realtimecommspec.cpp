//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "realtimecommspec.h"

#include "realtimeconnection.h"
#include "rtmessage.h"

RealTimeCommSpec::RealTimeCommSpec()
  : _connection(nullptr)
  , _connectMsg(nullptr)
  , _disconnectMsg(nullptr)
  , _incomingMsg(nullptr)
  , _bufferSize(1000000u)
  , _timeColumn(0)
  , _timeScale(1)
{
}


RealTimeCommSpec::~RealTimeCommSpec()
{
  delete _connection;
  delete _connectMsg;
  delete _disconnectMsg;
  delete _incomingMsg;
}


void RealTimeCommSpec::setConnection(RealTimeConnection *connection)
{
  delete _connection;
  _connection = connection;
}

void RealTimeCommSpec::setConnectMessage(RTMessage *msg)
{
  delete _connectMsg;
  _connectMsg = msg;
}

void RealTimeCommSpec::setDisconnectMessage(RTMessage *msg)
{
  delete _disconnectMsg;
  _disconnectMsg = msg;
}

void RealTimeCommSpec::setIncomingMessage(RTMessage *msg)
{
  delete _incomingMsg;
  _incomingMsg = msg;
}


void RealTimeCommSpec::disconnect()
{
  if (_connection && _connection->isConnected())
  {
    if (_disconnectMsg)
    {
      QByteArray buffer;
      _disconnectMsg->setMessage(buffer);
      _connection->send(buffer);
    }

    _connection->disconnect();
  }
}
