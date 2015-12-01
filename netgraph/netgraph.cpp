//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include <QByteArray>
#include <QDataStream>
#include <QElapsedTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#include <chrono>
#define _USE_MATH_DEFINES
#include <cmath>
#include <csignal>
#include <sstream>
#include <thread>

#ifndef __CDECL
#ifdef WIN32
#define __CDECL __cdecl
#else  // WIN32
#define __CDECL
#endif // WIN32
#endif // __CDECL

namespace
{
  bool s_quit = false;

  void __CDECL onSignal(int arg)
  {
    if (arg == SIGINT || arg == SIGTERM)
    {
      s_quit = true;
    }
  }
}

double plota(double time)
{
  return sin(time);
}

double plotb(double time)
{
  return cos(time);
}

double plotc(double time)
{
  return 2 * sin(2 * time);
}

double plotd(double time)
{
  return 2 * cos(2 * time);
}

int argIndex(const char *arg, int argc, char *argv[])
{
  for (int i = 0; i < argc; ++i)
  {
    if (strcmp(arg, argv[i]) == 0)
    {
      return i;
    }
  }

  return -1;
}


void usage()
{
  printf("Usage:\n");
  printf("  netgraph [port <port>] [tcp] [binary]\n");
}


int main(int argc, char *argv[])
{
  QElapsedTimer timer;
  quint16 port = 14444u;
  bool udp = true;
  bool binary = false;

  if (argIndex("-?", argc, argv) != -1 || argIndex("/?", argc, argv) != -1)
  {
    usage();
    return 0;
  }

  if (argIndex("tcp", argc, argv) != -1)
  {
    udp = false;
  }

  if (argIndex("binary", argc, argv) != -1)
  {
    binary = true;
  }

  if (argIndex("port", argc, argv) != -1)
  {
    int portIndex = argIndex("port", argc, argv);
    if (portIndex + 1 < argc)
    {
      std::istringstream in(argv[portIndex + 1]);
      in >> port;
    }
  }

  signal(SIGINT, &onSignal);
  signal(SIGTERM, &onSignal);

  QUdpSocket *udpSocket = nullptr;
  QTcpServer *tcpServer = nullptr;
  QTcpSocket *tcpSocket = nullptr;

  if (udp)
  {
    udpSocket = new QUdpSocket;
    udpSocket->bind();
  }
  else
  {
    tcpServer = new QTcpServer;
    tcpServer->listen(QHostAddress::Any, port);
  }

  printf("%s socket on port %u %s mode\n", (udp) ? "UDP" : "TCP", port, (!binary) ? "text" : "binary");

  timer.start();

  const unsigned BufferSize = 1024;
  char buffer[BufferSize];
  while (!s_quit)
  {
    quint64 timeMs = timer.elapsed();
    double time = timer.elapsed() * 1e-3;
    int len = 0;
    if (!binary)
    {
#ifdef _MSC_VER
      len = sprintf_s(buffer, BufferSize,
                      "%llu %g %g %g %g\n",
                      timeMs, plota(time), plotb(time), plotc(time), plotd(time));
#else  // _MSC_VER
      len = snprintf(buffer, BufferSize,
                     "%llu %g %g %g %g\n",
                     timeMs, plota(time), plotb(time), plotc(time), plotd(time));
#endif // _MSC_VER
      len = std::min(len, int(BufferSize));
    }
    else
    {
      QByteArray bytes;
      QDataStream stream(&bytes, QIODevice::WriteOnly);
      stream.setByteOrder(QDataStream::BigEndian);
      stream << timeMs;
      stream << plota(time);
      stream << plotb(time);
      stream << plotc(time);
      stream << plotd(time);
      len = std::min<int>(bytes.size(), BufferSize);
      memcpy(buffer, bytes.constData(), len);
    }

    if (udpSocket)
    {
      /*qint64 written = */udpSocket->writeDatagram(buffer, len, QHostAddress::LocalHost, port);
    }
    else if (tcpSocket)
    {
      QAbstractSocket::SocketState state = tcpSocket->state();
      switch (state)
      {
      case QAbstractSocket::ConnectedState:
        tcpSocket->write(buffer, len);
        tcpSocket->waitForBytesWritten();
        break;
      case QAbstractSocket::ConnectingState:
      case QAbstractSocket::HostLookupState:
        break;
      default:
        printf("Connection closed\n");
        tcpSocket->close();
        delete tcpSocket;
        tcpSocket = nullptr;
        break;
      }
    }
    else if (tcpServer)
    {
      tcpServer->waitForNewConnection(1000 / 10);
      tcpSocket = tcpServer->nextPendingConnection();
      if (tcpSocket)
      {
        printf("New connection\n");
      }
      continue; // Already waited in waitForNewConnection()
    }
    else
    {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 10));
  }

  delete udpSocket;
  delete tcpSocket;
  delete tcpServer;

  return 0;
}
