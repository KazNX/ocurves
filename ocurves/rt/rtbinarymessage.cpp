//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "rtbinarymessage.h"

#include <QDataStream>

const unsigned RTBinaryMessage::TypeSizes[] =
{
  0,
  0,
  0,
  0,
  1,
  2,
  4,
  8,
  1,
  2,
  4,
  8,
  4,
  8
};

RTBinaryMessage::RTBinaryMessage(bool littleEndian)
  : _littleEndian(littleEndian)
  , _messageMinSize(0)
{
}


void RTBinaryMessage::setMessage(QByteArray &buffer)
{
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream.setByteOrder((_littleEndian) ? QDataStream::LittleEndian : QDataStream::BigEndian);

  uint written = 0;
  for (const Field &field : _fields)
  {
    written += writeField(stream, field, written);
  }
}


int RTBinaryMessage::readMessage(const QByteArray &source)
{
  if (unsigned(source.size()) < _messageMinSize)
  {
    return 0;
  }

  QDataStream stream(source);
  stream.setByteOrder((_littleEndian) ? QDataStream::LittleEndian : QDataStream::BigEndian);

  int read = 0;
  for (Field &field : _fields)
  {
    read += readField(stream, field, read);
  }

  return stream.status() == QDataStream::Ok ? read : -1;
}


QStringList RTBinaryMessage::headings() const
{
  return _headings;
}


unsigned RTBinaryMessage::populateValues(std::vector<double> &values) const
{
  values.resize(_headings.size());
  unsigned index = 0;
  for (const Field &field : _fields)
  {
    if (field.heading)
    {
      values[index++] = field.value.toDouble();
    }
  }

  return unsigned(values.size());
}


void RTBinaryMessage::addField(const QString &name, FieldType type, bool heading, const QVariant &value)
{
  Field field;
  field.name = name;
  field.type = type;
  field.value = value;
  field.heading = heading;
  _fields << field;
  _messageMinSize += TypeSizes[field.type];
  if (heading)
  {
    _headings << name;
  }
}


int RTBinaryMessage::fieldIndex(const QString &key) const
{
  int index = 0;
  for (const Field &field : _fields)
  {
    if (field.name.compare(key) == 0)
    {
      return index;
    }
    ++index;
  }

  return -1;
}


uint RTBinaryMessage::writeField(QDataStream &stream, const Field &field, uint pos)
{
  switch (field.type)
  {
  case Padding:
    {
      uint paddingSize = field.value.toUInt();
      quint8 pad = 0;
      for (uint i = 0; i < paddingSize; ++i)
      {
        stream << pad;
      }

      return paddingSize;
    }
    break;
  case PadTo:
    {
      uint paddingSize = field.value.toUInt();
      quint8 pad = 0;
      for (uint i = pos; i < paddingSize; ++i)
      {
        stream << pad;
      }

      return paddingSize;
    }
    break;
  case PadByField:
    {
      QString fieldLookup = field.value.toString();
      uint paddingSize = fieldValueV(fieldLookup).toUInt();
      quint8 pad = 0;
      for (uint i = 0; i < paddingSize; ++i)
      {
        stream << pad;
      }

      return paddingSize;
    }
    break;
  case PadToField:
    {
      QString fieldLookup = field.value.toString();
      uint paddingSize = fieldValueV(fieldLookup).toUInt();
      quint8 pad = 0;
      for (uint i = pos; i < paddingSize; ++i)
      {
        stream << pad;
      }

      return paddingSize;
    }
    break;
  case Int8:
    {
      qint8 val = (qint8)field.value.toInt();
      stream << val;
      return 1;
    }
    break;
  case Uint8:
    {
      quint8 val = (quint8)field.value.toInt();
      stream << val;
      return 1;
    }
    break;
  case Int16:
    {
      qint16 val = (qint16)field.value.toInt();
      stream << val;
      return 2;
    }
    break;
  case Uint16:
    {
      quint16 val = (quint16)field.value.toInt();
      stream << val;
      return 2;
    }
    break;
  case Int32:
    {
      qint32 val = (qint32)field.value.toInt();
      stream << val;
      return 4;
    }
    break;
  case Uint32:
    {
      quint32 val = (quint32)field.value.toUInt();
      stream << val;
      return 4;
    }
    break;
  case Int64:
    {
      qint64 val = (qint64)field.value.toLongLong();
      stream << val;
      return 8;
    }
    break;
  case Uint64:
    {
      quint64 val = (quint64)field.value.toULongLong();
      stream << val;
      return 8;
    }
    break;
  case Float32:
    {
      float val = field.value.toFloat();
      stream << val;
      return 4;
    }
    break;
  case Float64:
    {
      double val = field.value.toDouble();
      stream << val;
      return 8;
    }
    break;
  default:
    break;
  }

  return 0;
}


int RTBinaryMessage::readField(QDataStream &stream, Field &field, uint pos)
{
  switch (field.type)
  {
  case Padding:
    {
      uint paddingSize = field.value.toUInt();
      quint8 pad = 0;
      uint read = 0;
      while (read < paddingSize && stream.status() == QDataStream::Ok)
      {
        stream >> pad;
        ++read;
      }

      return read;
    }
    break;
  case PadTo:
    {
      uint paddingSize = field.value.toUInt();
      quint8 pad = 0;
      uint read = pos;
      while (read < paddingSize && stream.status() == QDataStream::Ok)
      {
        stream >> pad;
        ++read;
      }

      return read;
    }
    break;
  case PadByField:
    {
      QString fieldLookup = field.value.toString();
      uint paddingSize = fieldValueV(fieldLookup).toUInt();
      quint8 pad = 0;
      uint read = 0;
      while (read < paddingSize && stream.status() == QDataStream::Ok)
      {
        stream >> pad;
        ++read;
      }

      return read;
    }
    break;
  case PadToField:
    {
      QString fieldLookup = field.value.toString();
      uint paddingSize = fieldValueV(fieldLookup).toUInt();
      quint8 pad = 0;
      uint read = pos;
      while (read < paddingSize && stream.status() == QDataStream::Ok)
      {
        stream >> pad;
        ++read;
      }

      return read;
    }
    break;
  case Int8:
    {
      qint8 val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 1 : 0;
    }
    break;
  case Uint8:
    {
      quint8 val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 1 : 0;
    }
    break;
  case Int16:
    {
      qint16 val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 2 : 0;
    }
    break;
  case Uint16:
    {
      quint16 val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 2 : 0;
    }
    break;
  case Int32:
    {
      qint32 val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 4 : 0;
    }
    break;
  case Uint32:
    {
      quint32 val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 4 : 0;
    }
    break;
  case Int64:
    {
      qint64 val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 8 : 0;
    }
    break;
  case Uint64:
    {
      quint64 val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 8 : 0;
    }
    break;
  case Float32:
    {
      float val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 4 : 0;
    }
    break;
  case Float64:
    {
      double val = 0;
      stream >> val;
      field.value = val;
      return stream.status() == QDataStream::Ok ? 8 : 0;
    }
    break;
  default:
    break;
  }

  return 0;
}


double RTBinaryMessage::fieldValueD(const QString &key) const
{
  for (const Field &field : _fields)
  {
    if (field.name.compare(key) == 0)
    {
      return field.value.toDouble();
    }
  }
  return 0;
}


QVariant RTBinaryMessage::fieldValueV(const QString &key) const
{
  for (const Field &field : _fields)
  {
    if (field.name.compare(key) == 0)
    {
      return field.value;
    }
  }
  return 0;
}

