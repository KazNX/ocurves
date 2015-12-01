//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REALTIMESOURCELOADER_H_
#define REALTIMESOURCELOADER_H_

#include "ocurvesconfig.h"

#include <QString>

class QDomDocument;
class QDomElement;
class RealTimeConnection;
class RealTimeCommSpec;

class RTMessage;
class RTBinaryMessage;

/// @ingroup realtime
/// Loads the XML specification files for @c RealTimeSource connections.
/// A description of the supported format can be found @ref rtxmlformat "here".
class RealTimeSourceLoader
{
public:
  /// Attempt to load a real time source file.
  /// @param fileName The name of the file to load.
  /// @param error Used to return an error message (if provided).
  /// @param errorLine Used to indicate the line number for the error (if provided).
  /// @param errorColumn Used to indicate the column number for the error (if provided).
  /// @return The @c RealTimeCommSpec loaded from the file on success, null on failure.
  RealTimeCommSpec *load(const QString &fileName, QString *error = nullptr, int *errorLine = nullptr, int *errorColumn = nullptr);

  /// Attempt to parse an XML string for a real time source specification.
  /// @param xmlString XML formated text to parse.
  /// @param error Used to return an error message (if provided).
  /// @param errorLine Used to indicate the line number for the error (if provided).
  /// @param errorColumn Used to indicate the column number for the error (if provided).
  /// @return The parsed @c RealTimeCommSpec on success, null on failure.
  RealTimeCommSpec *parse(const QString &xmlString, QString *error = nullptr, int *errorLine = nullptr, int *errorColumn = nullptr);

private:

  /// Parser implementation.
  /// @param doc XML document to parse.
  /// @return The parsed @c RealTimeCommSpec on success, null on failure.
  RealTimeCommSpec *parse(QDomDocument &doc);

  /// Parse the XML elements for a @c RealTimeSerialConnection.
  /// @param element The "serial" XML element.
  /// @return The parsed @c RealTimeCommSpec on success, null on failure.
  RealTimeCommSpec *parseSerial(QDomElement &element);

  /// Parse the XML elements for a @c RealTimeTcpConnection or @c RealTimeUdpConnection.
  /// @param element The "network" XML element.
  /// @return The parsed @c RealTimeCommSpec on success, null on failure.
  RealTimeCommSpec *parseNetwork(QDomElement &element);

  /// Parse elements common to all connections.
  /// @param element The "serial" or "network" element.
  /// @param spec The spec to parse data into.
  /// @param timeFieldName Set to the name of the time column field. Empty otherwise.
  /// @return True on success.
  bool parseCommon(QDomElement &element, RealTimeCommSpec &spec, QString &timeFieldName);

  /// Parse the "comms" element.
  /// @param element The "comms" element.
  /// @param spec The spec to parse data into.
  /// @param timeFieldName The name of the time column field as read in @c parseCommon().
  /// @return True on success.
  bool parseComms(QDomElement &commsElement, RealTimeCommSpec &spec, const QString &timeFieldName);

  /// Parse binary structure details for a binary message.
  /// @param element The structure element.
  /// @return The message representation on success, null on failure.
  RTBinaryMessage *parseBinaryStruct(QDomElement &element);
};

#endif // REALTIMESOURCELOADER_H_
