//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REALTIMEPLOT_H_
#define REALTIMEPLOT_H_

#include "ocurvesconfig.h"

#include "plotgenerator.h"
#include "plotsource.h"

#include <QMutex>

class Curves;
class QSerialPort;
class QAbstractSocket;
class QMutex;

/*!
@ingroup rt
@page rtxmlformat Real Time Connection File Format

Real time data connections used an XML file to specify the connection type and messaging formats.
These files are loaded by the @c RealTimePlot class. The XML file identifies the following attributes:
- The connection port and communications protocol
- The outgoing and incoming message formats.

The serial example below uses the string data format, while the network section shows the a binary
format example. Either is valid in either block. The \<headings\> element is optional.

@code
<connection>
  <serial port="portname" baud="9600">
    <buffer size="xxx"/>
    <time column="1" scale="1.0"/>
    <comms format="utf-8">
      <onconnect>abcdefg</onconnect>
      <headings>
        <heading name="name1"/>
        <heading name="name2"/>
        <heading name="name3"/>
      </headings>
    </comms>
  </serial>
  <network ip="ipv4" port="1234" protocol="udp|tcp">
    <buffer size="xxx"/>
    <time field="timestamp" scale="1.0e-6"/>
    <comms format="binary">
      <onconnect>
        <!-- Data structure sent when connection is made -->
        <!-- Endian specification is optional, defaulting to the current platform -->
        <struct endian="little|big|network">
          <field type="uint16" name="type">6</field>
          <field type="uint16" name="subtype">1</field>
          <field type="uint16" name="size">16</field>
          <field type="uint16" name="flags">2</field>
          <field type="uint32" name="sequenceNumber">0</field>
        </struct>
      </onconnect>
      <ondisconnect>
        <!-- Data structure sent when disconnecting -->
        <struct endian="little|big|network">
          <field type="uint16" name="type">6</field>
          <field type="uint16" name="subtype">2</field>
          <field type="uint16" name="size">16</field>
          <field type="uint16" name="flags">2</field>
          <field type="uint32" name="sequenceNumber">0</field>
        </struct>
      </ondisconnect>
      <receive>
        <!-- Expected incoming data structure -->
        <!-- Only fields with 'heading'='yes' are displayed. Additionally, the time element above may specify any field to derive a time value from. -->
        <struct endian="little|big|network">
          <field type="uint16" name="type" />
          <field type="uint16" name="subtype" />
          <field type="uint16" name="size" />
          <field type="uint16" name="flags" />
          <field type="uint32" name="sequenceNumber" />
          <field type="uint64" name="timestamp" />
          <field type="double" name="pitch" heading="yes" />
          <field type="double" name="roll" heading="yes" />
          <field type="double" name="yaw" heading="yes" />
          <field type="uint8" name="mode" />
          <!-- Pad out the read to the size field -->
          <padby field="size" />
          <!-- Pad out the read by another 2 bytes -->
          <pad size="2" />
        </struct>
      </receive>
    </comms>
  </network>
</connection>
@endcode
 !*/

class RealTimeCommSpec;

/// @ingroup gen
/// Generates @c PlotInstance objects for plotting over real-time connections.
///
/// This generator supports loading XML files which identify the real-time connections
/// to make. See @ref rtxmlformat.
///
/// This generator is designed to run indefinitely, and will never complete. It
/// can be started immediately with no comm-spec files. It continually checks for
/// new comm-specs provided to @c appendLoad(). Existing real-time sources can
/// be cleared by calling @c stop(), leaving the thread running, ready for more
/// @c appendLoad() calls. Alternatively the entire thread aborted @c abortLoad()
/// or @c quit().
class RealTimePlot : public PlotGenerator
{
  Q_OBJECT
public:
  enum
  {
    DEFAULT_SAMPLE_LIMIT = 1000000, ///< Default sample buffer size limit (element count).
    MAX_READ_BUFFER_SIZE = 4 * 1024 ///< Default read buffer size (bytes).
  };

  /// Data tracked about a real time source.
  struct RealTimePlotInfo
  {
    PlotSource::Ptr source; ///< The source
    RealTimeCommSpec *spec; ///< Communications specification.
    QByteArray readBuffer;  ///< Read buffer.

    /// Constructor
    RealTimePlotInfo();

    /// Deletes the @c spec.
    ~RealTimePlotInfo();
  };

  /// Create a real-time plot generator based on the given connection file set.
  /// @param curves Curves data model.
  /// @param connectionFiles XML files defining the connections.
  RealTimePlot(Curves *curves, const QStringList &connectionFiles);

  /// Destructor, ensuring the thread is stopped and connections are closed.
  ~RealTimePlot();

  /// Adds additional comm-specs to load (xml files).
  /// @param connectionFiles Additional connections to load.
  void appendLoad(const QStringList &connectionFiles);

  /// Retrieve the list of connection files pending processing.
  /// @param pendingFiles Populated with the pending file list.
  /// @return True if there are pending files.
  bool pendingLoad(QStringList &pendingFiles);

  /// Stop current real-time plots without terminating the thread.
  void stop();

protected:
  /// Run loop.
  void run() override;

private:
  /// Load the pending connection files initiating real-time data loading.
  /// @param startTime The current time value (for time-stamping).
  /// @return The number of additional sources loaded.
  unsigned loadSpecs(double startTime);

  /// Load a single real-time XML comm-spec file.
  /// @param filePath The path to the file to load.
  /// @return A valid @c RealTimePlotInfo object on success, null on failure.
  RealTimePlotInfo *loadCommFile(const QString &filePath);

  /// Create the @c PlotInstance objects for @c rtplot.
  ///
  /// Called either immediately if there are headings specified in the incoming @c RTMessage.
  /// Otherwise called after the first data line (to ensure column count is known).
  ///
  /// @param rtplot Real-time info.
  /// @param count Number of columns for the plot.
  /// @param headings Optional column headings. Must be of length @c count.
  ///     Generates headings if omitted.
  void createPlots(RealTimePlotInfo &rtplot, unsigned count, const QStringList *headings = nullptr);

  /// @overload
  void createPlots(RealTimePlotInfo &rtplot, const QStringList &headings);

  QStringList _connectionFiles;         ///< Pending loading list.
  QList<RealTimePlotInfo *> _sources;   ///< Loaded sources.
  bool _stopRequested;                  ///< True if @c stop() has been requested.
};

#endif // REALTIMEPLOT_H_
