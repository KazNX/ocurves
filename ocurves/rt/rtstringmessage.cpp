//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "rtstringmessage.h"

#include "plotfile.h"


RTStringMessage::RTStringMessage(const QString &message)
  : _message(message)
{
}


void RTStringMessage::setMessage(QByteArray &buffer)
{
  buffer = _message.toLocal8Bit();
}


int RTStringMessage::readMessage(const QByteArray &buffer)
{
  // Read up to a newline.
  for (int i = 0; i < buffer.size(); ++i)
  {
    if (buffer.at(i) == '\n')
    {
      _message = QString(buffer.left(i + 1));
      return i + 1;
    }
  }

  // No newline. Nothing read.
  return 0;
}



QStringList RTStringMessage::headings() const
{
  return _headings;
}


unsigned RTStringMessage::populateValues(std::vector<double> &values) const
{
  // Parse the current message.
  QString line = _message;
  PlotFile::dataLine(line, values, false);
  if (_headings.empty())
  {
    for (size_t i = 0; i < values.size(); ++i)
    {
      _headings << QString("Column %1").arg(i);
    }
  }
  return unsigned(values.size());
}
