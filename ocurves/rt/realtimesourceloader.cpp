//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "realtimesourceloader.h"

#include "plotfile.h"
#include "realtimecommspec.h"
#include "realtimeserialconnection.h"
#include "realtimetcpconnection.h"
#include "realtimeudpconnection.h"
#include "rtbinarymessage.h"
#include "rtstringmessage.h"

#include <QDataStream>
#include <QFile>
#include <QDomDocument>
#include <QVariant>

#include <vector>

RealTimeCommSpec *RealTimeSourceLoader::load(const QString &fileName, QString *error, int *errorLine, int *errorColumn)
{
  QFile file(fileName);
  QDomDocument doc;

  if (!doc.setContent(&file, error, errorLine, errorColumn))
  {
    return nullptr;
  }

  return parse(doc);
}


RealTimeCommSpec *RealTimeSourceLoader::parse(const QString &xmlString, QString *error, int *errorLine, int *errorColumn)
{
  QDomDocument doc;
  if (!doc.setContent(xmlString, error, errorLine, errorColumn))
  {
    return nullptr;
  }

  return parse(doc);
}

RealTimeCommSpec *RealTimeSourceLoader::parse(QDomDocument &doc)
{
  QDomElement root = doc.firstChildElement();
  if (root.tagName().compare("connection") != 0)
  {
    return nullptr;
  }

  QDomElement con;

  if (!(con = root.firstChildElement("serial")).isNull())
  {
    return parseSerial(con);
  }
  else if (!(con = root.firstChildElement("network")).isNull())
  {
    return parseNetwork(con);
  }

  // Unknown or unspecified connection type.
  return nullptr;
}


RealTimeCommSpec *RealTimeSourceLoader::parseSerial(QDomElement &element)
{
  QString port;
  unsigned baud = 9600;

  RealTimeCommSpec *spec = new RealTimeCommSpec();
  QString timeFieldName;
  if (!parseCommon(element, *spec, timeFieldName))
  {
    delete spec;
    return nullptr;
  }

  QDomElement comms = element.firstChildElement("comms");
  if (!parseComms(comms, *spec, timeFieldName))
  {
    delete spec;
    return nullptr;
  }

  port = element.attribute("port");
  QString s = element.attribute("baud");
  if (!s.isEmpty())
  {
    baud = s.toUInt();
  }

  RealTimeSerialConnection *source = new RealTimeSerialConnection();
  bool readOnly = spec->connectMessage() == nullptr && spec->disconnectMessage() == nullptr;
  if (!source->open(port, baud, readOnly))
  {
    delete spec;
    delete source;
    return nullptr;
  }

  spec->setConnection(source);
  return spec;
}


RealTimeCommSpec *RealTimeSourceLoader::parseNetwork(QDomElement &element)
{
  QString address = element.attribute("address");
  ushort port = ushort(element.attribute("port").toUInt());
  QString protocol = element.attribute("protocol");
  QString timeFieldName;

  RealTimeCommSpec *spec = new RealTimeCommSpec();
  if (!parseCommon(element, *spec, timeFieldName))
  {
    delete spec;
    return nullptr;
  }

  QDomElement comms = element.firstChildElement("comms");
  if (!parseComms(comms, *spec, timeFieldName))
  {
    delete spec;
    return nullptr;
  }

  if (protocol.compare("tcp") == 0)
  {
    RealTimeTcpConnection *source = new RealTimeTcpConnection();
    if (!source->open(QHostAddress(address), port))
    {
      delete source;
      delete spec;
      return nullptr;
    }
    spec->setConnection(source);
  }
  else if (protocol.compare("udp") == 0)
  {
    RealTimeUdpConnection *source = new RealTimeUdpConnection();
    if (!source->open(QHostAddress(address), port))
    {
      delete source;
      delete spec;
      return nullptr;
    }
    spec->setConnection(source);
  }

  if (!spec->connection())
  {
    delete spec;
    return nullptr;
  }

  return spec;
}


bool RealTimeSourceLoader::parseCommon(QDomElement &element, RealTimeCommSpec &spec, QString &timeFieldName)
{
  QDomElement bufferElem = element.firstChildElement("buffer");
  QDomElement timeElem = element.firstChildElement("time");

  bool ok = true;
  spec.setBufferSize(bufferElem.attribute("size").toUInt(&ok));
  if (!ok || spec.bufferSize() == 0)
  {
    spec.setBufferSize(64 * 1024);
  }
  spec.setTimeColumn(timeElem.attribute("column").toUInt());
  timeFieldName = timeElem.attribute("field");
  spec.setTimeScale(timeElem.attribute("scale").toDouble(&ok));
  if (!ok || spec.timeScale() == 0)
  {
    spec.setTimeScale(1);
  }

  return true;
}


bool RealTimeSourceLoader::parseComms(QDomElement &commElement, RealTimeCommSpec &spec, const QString &timeFieldName)
{
  if (commElement.isNull())
  {
    return false;
  }

  QString binaryAttribute = commElement.attribute("binary");
  if (binaryAttribute.compare("true") == 0)
  {
    QDomElement elem = commElement.firstChildElement("onconnect");
    if (!elem.isNull())
    {
      QDomElement child = elem.firstChildElement("struct");
      RTMessage *msg = parseBinaryStruct(child);
      if (!msg)
      {
        return false;
      }
      spec.setConnectMessage(msg);
    }

    elem = commElement.firstChildElement("receive");
    if (!elem.isNull())
    {
      QDomElement child = elem.firstChildElement("struct");
      RTBinaryMessage *msg = parseBinaryStruct(child);
      if (!msg)
      {
        return false;
      }
      spec.setIncomingMessage(msg);
      if (!timeFieldName.isEmpty())
      {
        spec.setTimeColumn(msg->fieldIndex(timeFieldName) + 1);
      }
    }

    elem = commElement.firstChildElement("ondisconnect");
    if (!elem.isNull())
    {
      QDomElement child = elem.firstChildElement("struct");
      RTMessage *msg = parseBinaryStruct(child);
      if (!msg)
      {
        return false;
      }
      spec.setDisconnectMessage(msg);
    }

    return spec.incomingMessage() != nullptr;
  }
  else
  {
    QDomElement elem = commElement.firstChildElement("onconnect");
    if (!elem.isNull())
    {
      spec.setConnectMessage(new RTStringMessage(elem.text()));
    }

    RTStringMessage *incoming = new RTStringMessage();
    spec.setIncomingMessage(incoming);
    elem = commElement.firstChildElement("headings");
    if (!elem.isNull())
    {
      QStringList headings;
      QDomElement headingElem = elem.firstChildElement("heading");
      while (!headingElem.isNull())
      {
        headings << headingElem.attribute("name");
        headingElem = headingElem.nextSiblingElement("heading");
      }

      incoming->setHeadings(headings);
    }

    elem = commElement.firstChildElement("ondisconnect");
    if (!elem.isNull())
    {
      spec.setDisconnectMessage(new RTStringMessage(elem.text()));
    }

    return true;
  }

  return false;
}


RTBinaryMessage *RealTimeSourceLoader::parseBinaryStruct(QDomElement &element)
{
  if (element.isNull())
  {
    return nullptr;
  }

  QString endianAttr = element.attribute("endian");
  bool littleEndian = !endianAttr.isEmpty() && endianAttr.compare("little") == 0;

  RTBinaryMessage *msg = new RTBinaryMessage(littleEndian);

  QDomElement field = element.firstChildElement("field");
  while (!field.isNull())
  {
    QString typeStr = field.attribute("type");
    QString name = field.attribute("name");
    QString valueStr = field.text();
    bool heading = field.attribute("heading").compare("true") == 0;
    RTBinaryMessage::FieldType type = RTBinaryMessage::Padding;
    QVariant val;

    bool ok = true;
    if (typeStr.compare("pad") == 0)
    {
      // Pad by an offset.
      type = RTBinaryMessage::Padding;
      val = valueStr.toUInt();
    }
    else if (typeStr.compare("padto") == 0)
    {
      // Pad to a fixed size.
      type = RTBinaryMessage::PadTo;
      val = valueStr.toUInt();
    }
    else if (typeStr.compare("padbyfield") == 0)
    {
      // Pad by a field name.
      type = RTBinaryMessage::PadByField;
      val = valueStr;
    }
    else if (typeStr.compare("padtofield") == 0)
    {
      // Pad to a field name.
      type = RTBinaryMessage::PadToField;
      val = valueStr;
    }
    else if (typeStr.compare("int8") == 0)
    {
      type = RTBinaryMessage::Int8;
      val = valueStr.toInt();
    }
    else if (typeStr.compare("int16") == 0)
    {
      type = RTBinaryMessage::Int16;
      val = valueStr.toInt();
    }
    else if (typeStr.compare("int32") == 0)
    {
      type = RTBinaryMessage::Int32;
      val = valueStr.toInt();
    }
    else if (typeStr.compare("int64") == 0)
    {
      type = RTBinaryMessage::Int64;
      val = valueStr.toLongLong();
    }
    else if (typeStr.compare("uint8") == 0)
    {
      type = RTBinaryMessage::Uint8;
      val = valueStr.toUInt();
    }
    else if (typeStr.compare("uint16") == 0)
    {
      type = RTBinaryMessage::Uint16;
      val = valueStr.toUInt();
    }
    else if (typeStr.compare("uint32") == 0)
    {
      type = RTBinaryMessage::Uint32;
      val = valueStr.toUInt();
    }
    else if (typeStr.compare("uint64") == 0)
    {
      type = RTBinaryMessage::Uint64;
      val = valueStr.toULongLong();
    }
    else if (typeStr.compare("float") == 0)
    {
      type = RTBinaryMessage::Float32;
      val = valueStr.toFloat();
    }
    else if (typeStr.compare("double") == 0)
    {
      type = RTBinaryMessage::Float64;
      val = valueStr.toDouble();
    }
    else
    {
      ok = false;
    }

    if (ok)
    {
      msg->addField(name, type, heading, val);
    }

    field = field.nextSiblingElement("field");
  }

  return msg;
}
