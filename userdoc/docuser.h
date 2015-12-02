//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef DOCMAIN_H_
#define DOCMAIN_H_

// Documentation style notes:
// - User documentation page labels are prefixed with a 'u' to help distinguish from API and technical reference documentation.

/*!
@cond USERDOC
@mainpage User Documentation
Open Curves is a time series data plotting application designed to quickly open and view CSV style data
files containing numeric, time series data. Open Curves aims to make plotting such data quick and easy
without providing data manipulation present in packages such as MATLAB.

- @subpage uistart
- @subpage uui
- @subpage uformats
- @subpage urtsources
- @subpage uexp
- @subpage ubookmarks
@endcond
*/

/*!
@cond !USERDOC
@page udoc User Documentation
This section contains the user documentation for Open Curves. This documentation is also available though
the "help" menu option in the Open Curves application.

- @subpage uistart
- @subpage uui
- @subpage uformats
- @subpage urtsources
- @subpage uexp
- @subpage ubookmarks
@endcond
*/

/*!
@page uistart Getting Started
To plot a time series data file, open Open Curves and simply drag-and-drop the file from a file
explorer program into the Open Curves window. Alternatively, select "File->Open" from the Open Curves menu
bar and browse to the data file of interest.

For more information on supported data file formats, see @ref uformats. For information about using
the Open Curves UI, see @ref uui.
*/

/*!
@page uui User Interface
The main user interface consists of several components, centred around the plot view. The main UI
is shown in the image below.

<div style="text-align:center">@image html ui-labelled.png ""</div>

-# [Plot View] (#uplotview)
-# [Toolbar] (#utoolbar)
-# [Source and plots list] (#usourceplotslist)
-# [Legend] (#ulegend)
-# [Curve properties] (#ucurveproperties)
-# [Expressions] (#uexpressions)

# Plot View # {#uplotview}
The plot view plots currently selected data plots. Each curve is plotted in a continuous fashion,
interpolating values in between data points. Curves are individually coloured by cycling through
the available colour palette.

The plot view supports a right click context menu which can be used to change interactions with
the plot view. Note that the context menu is accessed via a right click and hold, rather than just
a quick right click. This is used to distinguish between context menu access and
[pan/zoom interaction](#uzoompan) with the plot view.

## Time and Scaling ##  {#utimescale}
Open Curves is primarily intended for plotting time series data. As such, the X axis is considered a
monotonic time value, while the Y value is the sample at that time. The time value associated with
each line of a file source can be derived in one of several ways.

-# Each data line is considered a new time value. The first line is time zero, the next time 1, etc.
-# From a time data column in the file.

With a time column specified, the X value for each entry is derived by referencing the time column.
The time column dictates the X value for each data sample on the corresponding line. The time
column can be specified on the toolbar and affects any file loaded thereafter.

The time value may be modified by a time offset and by a time scaling multiplier. The offset is
subtracted from each time value before calculating an X value, while the multiplier scales the
result. Note that the offset is applied before scaling. As with the time column, the scale value
can be set for in the toolbar. The toolbar does not support directly specifying a time offset, but
does support "relative time", in which case the offset is derived from the first time value in the
file. Both time offset and time scale can be modified in the curve properties.

## Zooming and Panning ##  {#uzoompan}
The plot view supports zooming and panning. Curves are initially loaded with auto-scaling enabled,
and all selected curves are fitted to the view. The view can then be zoomed by dragging out zoom
box to define the zoomed area. Zooming can be enabled for both X and Y axes, or only X or Y,
leaving the extents of the other axis unchanged.

There are three modes of operation for zooming and panning:
-# [Zoom and Pan](#umultitool)
-# [Zoom only](#uzoomtool)
-# [Pan only](#upantool)

Zoom and Pan multi-tool is designed for use with a three button mouse. The other modes are intended
for use where a middle mouse click cannot easily be generated or distinguished, such as when using
a touch pad.

Note that right click and hold is used to access the [plot view](#uplotview) context menu, while a
quick right click is used for zoom interaction.

### Zoom and Pan ###  {#umultitool}
The zoom and pan mode combines zooming and panning in one mode, but requires a three button mouse.
The buttons are mapped as follows:
- <b>Left:</b> Click and drag to define the zoom box.
- <b>Middle:</b> Click and drag to pan.
- <b>Right:</b> Reset to zoom to auto scale.
- <b>Mouse Wheel:</b> Cycle through previous zoom levels.

### Zoom Only ### {#uzoomtool}
Zoom only mode acts like the Zoom and Pan mode, but disables middle mouse button panning.

### Pan Only ###  {#upantool}
In pan only mode, the left mouse click and drag results in panning only and zooming is disabled.

## Splitting ## {#usplitting}
The plot view can be split to show different views into the same data set. Using the "View->Split"
menu item, the active view can be split either horizontally or vertically. When split, a new view
is generated as a copy of the active view, showing the same data.

The active view is changed by clicking one the view to activate. The source and plot lists are
updated to reflect the active view. Curves are coloured based on the active plots across all views.

## View Synchronisation ##
@parblock
Once split, views can be synchronised in order to maintain the same zoom level and pan position
across different views. A view is synchronised by clicking the view synchronisation button
@image html unlock.png "", which changes the icon to the synchronised icon:
@image html lock.png "". Synchronised views respond to zoom and pan changes in other synchronised
view. That is, zooming or panning one synchronised view affects all synchronised views. Note that
this respects the zoom axes, thus zooming in X only will change only the X range for all
synchronised views, leaving the Y range unchanged in each view.
@endparblock

Panning a synchronised view pans other synchronised views by the same delta.

## View Legend ## {#uviewlegend}
Each view supports its own legend. The legend position is controlled by the legend drop down menu
at the bottom of a view. There are four positions locating the legend around the outside of the
view, while the "shared" legend option indicates the view should only use the
[shared legend](#ulegend) control. The view positions are:
- Left
- Right
- Bottom
- Top
- Shared (default)

The per-view legend is useful when
[copying the view content to the system clipboard] (#uviewclipboard).

## Plot Views and the Clipboard ## {#uviewclipboard}
The contents of a plot view may be copied to the clipboard on platforms where a system clipboard
exists. The view is copied via the menu item "Edit->Copy Active View", or using it's assigned hot
key (e.g., Control-C). This copies the view content at its current size as a bitmap image. The
image can then be paste into another program, such as an image editing program, and saved to disk.

A legend is present in the clipboard image only when the plot view legend is position is other than
"shared".

# Toolbar # {#utoolbar}
The toolbar contains tools and controls affecting how data files are loaded and tools which control
interaction with the plot view. This includes [time scaling] (#utimescale) and
[pan/zoom] (#uzoompan) tools.

# Source and Plots List # {#usourceplotslist}
The source view displays a list of open sources and available plots from the selected sources. A
@em source is generally a single file, but can refer to other data sources such as a
[real time sources](@ref urtsources). Selecting a source displays the available plots from that
source in the plots list. Selected plots are displayed in the plot view.

Both source and plots lists support a context menu for modifying the selection. The source list
context menu also supports removing a source, thereby removing all plots from that source. Double
clicking an item in either list selects only that item.

# Legend #  {#ulegend}
@parblock
The legend control identifies curves by colour and name. All visible plots are represented in the
legend by its colour and name: @image html ui-legend-item.png "". The name is derived by combining
the plot name and its source name (see @ref uformats).

Items in the legend can be clicked. Clicking a legend displays the
[curve properties] (#ucurveproperties) for that item.
@endparblock

# Curve Properties #  {#ucurveproperties}
The curve properties control shows editable properties relating to the selected curve. There are
two sections to the properties; source and curve. The source properties relate to all curves loaded
from the same source and include the time column, time offset and time scale. Changing one of these
values affects all curves loaded from the same source.

The curve properties relate only to the selected curve and affect how that curve is drawn. The
editable properties are:
- <b>Colour -</b> Controls the colour uses to display this curve.
- <b>Style -</b> Controls how sample points in the curve are displayed.
- <b>Symbol -</b> Allows a symbol to be drawn at each sample point.
- <b>Filter Infinite -</b> Supports filtering of infinite values.
- <b>Filter NaN -</b> Supports filtering of Not-A-Number values.

## Colour ##
@parblock
Click the colour to select the colour used to display the curve. The adjacent lock icon changed to
@image html lock.png "", indicating that this curve's colour is locked. The curve is no longer
automatically assigned a colour and is always displayed using the selected colour. Click the lock
again to restore automatic colour assignment for this curve.

By default curves are assigned colours from the colour palette.
@endparblock

## Style ##
Curves can be drawn using one of the following styles:
- <b>Line -</b> Sample points are connected by straight lines.
- <b>Sticks -</b> Sample points are represented as sticks or bars extending from zero to the sample
value. Samples are not connected.
- <b>Steps -</b> Samples are connected by discrete steps. The curve maintains the last sample value
until a new value is available.
- <b>Dots -</b> Samples are drawn as discrete dots.

The line or dot thickness can also be modified.

## Symbol ##
Sample points may be represented by a symbol, such as a square or triangle. This can be used in
conjunction with the curve style.

## Filtering ##
Infinite and not-a-number (NaN) values can be filtered, replacing such values with a zero value.
The text format of infinite and NaN values is described in [Supported Formats] (#uformats).

# Colours # {#ucolours}
Open Curves uses a set colour palette to distinguish between curves. Colours are selected whenever the
set of selected curves changes. The first curve is assigned the first colour, the second curve the
second colour and so on. The colour palette can be changed and is edited by selecting
"Edit->Colours" in the menus.

Colours can be added, removed and editing (double-click). The colour palette can also be reset to
one of the pre-defined colour palettes. The default palettes include colour blind friendly palettes.
These palettes have colours chosen to help distinguish curves for the colour blind. The palettes
have been constructed as a best guess and may not be ideal, but will hopefully provide a useful
starting point.

# Expressions # {#uexpressions}
Expressions provide a way of generation new plots from existing plots. Simple expressions operate
on plots appearing in the [plots list] (#usourceplotslist) and can be used to scale, shift and
compare plots.

Current expressions appear in the @em Expressions list, while new expressions are added by editing
the text box below. The @em Functions tabs list the built in functions which can be used in
expressions.

Expressions are added by editing the expressions text box and pressing @em Add. Expressions are
removed by selecting an item in the @em Expressions list and clicking @em Remove. Expressions can
also be updated by selecting an expression, editing expression and pressing @em Update.

*/

/*!
@page uformats Supported Formats
Open Curves supports CSV style, text based data files as input. Input files are considered a tabular
format, where each line represents a new row in the table, and each delimited item corresponds to
the table columns. The columns are mapped to plotted items appearing in the
[plot view] (#usourceplotslist), and one column may optionally identify the timestamp for each row.

Open Curves does its best to deduce the table definition from the first few lines of the data file.
Columns may be whitespace or comma delimited and the first line of the file may optionally identify
column headings. However, each line of an input file must have the same number of columns. Each
column data item, after the headings line, must be of a numeric format; integer, floating point or
scientific notation (see below).

Column headings are parsed differently to data lines and support some additional whitespace
allowances. Specifically, whitespace may be used to delimit columns, however, the space character
may also be included in column names provided that the space characters are followed by an open
bracket '('. This is specifically to support column names of the form: "data (units)". That is
a named column followed by a specification of the units for that column. Valid column names are
shown below.

Unicode formats are supported provided the file includes a byte-order-mark (BOM).

This page documents how text file sources are loaded. For details of how real time sources are
handled see @ref urtsources.

# Valid Heading Formats #
Column headings are whitespace and/or comma separated. Additionally, a column heading may include
spaces after non-whitespace before the next open bracket character. Examples are provided below:
Headings Line                                   | Heading 1     | Heading 2   | Heading 3
----------------------------------------------- | ------------- | ----------- | --------------------
timestamp speed acceleration                    | timestamp     | speed       | acceleration
timestamp,speed,acceleration                    | timestamp     | speed       | acceleration
timestamp , speed , acceleration                | timestamp     | speed       | acceleration
timestamp(s) speed(m/s) acceleration(m/s/s)     | timestamp(s)  | speed(m/s)  | acceleration(m/s/s)
timestamp (s) speed (m/s) acceleration (m/s/s)  | timestamp (s) | speed (m/s) | acceleration (m/s/s)

# Valid Numeric Formats #
Open Curves data items can be of any string format which can be parsed to a valid double-precision
floating point type. Integer, floating point and scientific notation are all supported. Infinite
and not-a-number values are also supported. Numeric parsing uses Qt's
<a href="http://doc.qt.io/qt-5/qstring.html"><tt>QString</tt></a> double parsing. Hexadecimal
values are not supported and values which fail parsing are interpreted as zero.

Examples of supported values are:
- @c 42
- @c -382
- @c 3.141
- @c -2.8
- @c inf
- @c -inf
- @c nan
- @c nan(snan)

The last items correspond to infinite and not-a-number results.

*/


/*!
@page urtsources Real Time Sources
@b Note: Real time sources are an experimental feature and may fail or fail to function as
documented.

Open Curves may plot data in real time via an Ethernet or serial connection. A real time connection is
made by opening a real time source specification file (XML format) which identifies the connection
protocol, address and/or port and message formats. Specifically reading from TCP/IP, UDP and serial
ports are supported.

On establishing a connection, Open Curves will attempt to decode incoming data in the format specified
in the connection XML file. Open Curves may optionally send messages when the connection is established
and before closing the connection. These messages may, for example, be used to request data
transmission begin and end.

Open Curves maintains a fixed size ring buffer for incoming data samples. The plot is periodically
redrawn as new data arrive and are added to the ring buffer.

Expressions are not supported with real time data.

- @subpage urtformat
- @subpage urtex
*/

/*!
@page urtformat Real Time Source XML Specification

The XML format for connecting Open Curves to a real-time data source is described below. The root
element is "connection" and contains either a "serial" or "network" child element.

# <tt>connection</tt> # {#xmlconnection}
- @b Children  - <tt>serial, network</tt>

The root document element.


# <tt>serial</tt> # {#xmlserial}
- @b Parent - @c connection
- @b Children  - <tt>buffer, time, comms</tt>

The @c serial element specifies the use of a serial data connection. It is mutually exclusive of
the @c network element.

Attribute | Description
--------- | -----------------------------------------------------------------------
port      | Identifies the serial comm port to connect on ("COM3", "/dev/ttyUSB0").
baud      | Specifies the serial baud rate.


# <tt>network</tt> # {#xmlnetwork}
- @b Parent - @c connection
- @b Children  - <tt>buffer, time, comms</tt>

The @c network element specifies the use of a TCP or UDP connection. It is mutually exclusive of
the @c serial element.

Attribute | Description
--------- | -----------------------------------------------------------------------
address   | The connection address.
port      | The target connection port.
protocol  | Either "tcp" or "udp".


# <tt>buffer</tt> # {#xmlbuffer}
@b Optional: Determines the size of the ring buffer Open Curves uses to store data samples.

Attribute | Description
--------- | -----------------------------------------------------------------------
size      | The number of data elements to store in the ring buffer. This number of elements is stored for each "column". Zero size results in the default size being used.


# <tt>time</tt> # {#xmltime}
@b Optional: The @c time element can be used to identify the index of the timestamp column in the
incoming data, and/or to adjust the timescale. The timestamp column is used to timestamp each
incoming data value. The index is specified by the @c column attribute. The @c scale attribute is
used to scale the read time stamps into seconds. The default is 1.

Attribute | Description
--------- | -----------------------------------------------------------------------
column    | Optional, one based index identifying the timestamp column index (default zero, for no column).
scale     | Optional time scale multiplier used to convert the time column into seconds. The default is 1.


# <tt>comms</tt> # {#xmlcomms}
- @b Parent - @c serial or @c network
- @b Children  - <tt>onconnect, headings, receive, ondisconnect</tt>

Identifies how to communicate over the selected protocol. Communication may be binary or text-based
as indicated by the @c binary attribute. The binary selection affects how descendants of this
element are formatted.

Attribute | Description
--------- | -----------------------------------------------------------------------
binary    | Optional, "true" to use binary communications, "false" for text based communication. The default is "false".


# <tt>onconnect</tt> and <tt>ondisconnect</tt> # {#xmlconnect}
- @b Parent - @c comms
The @c onconnect element the message to send when a connection is established, while
@c ondisconnect specifies the message to send just prior to disconnecting. These are intended to
be used as handshaking messages, to request the start and end of data transmission. The content of
these elements depends on the @c binary attribute of its parent. In binary mode, elements require
a @c struct child element. In text mode, the element text is used for the message (the text between
the start and end tag).

In either communication mode both @c onconenct and @c ondisconnect elements are optional and may
be omitted.


# <tt>headings</tt> and <tt>receive</tt> # {#xmlreceive}
- @b Parent - @c comms
- @c Children - @c heading for @c headings, or @c struct for @c receive
@b Optional: The @c headings and @c receive elements specify details about the incoming data, but are mutually
exclusive. Use @c headings when the @c comms mode is text based and @c receive in binary mode.

The @c headings element specifies a list of headings to attribute to incoming data as well as
identifying the expected number of incoming data items per message. Incoming messages are parsed
in the same was as each line in a [text input file] (#uformats). The @c headings element has a
number of @c heading child elements, one per "column" as defined in the
[supported formats] (#uformats). Each @c heading element has a single @c name attribute which is
used as the name for that column.

When omitted, the number of text based samples per line is inferred from the first line, and
headings are labelled "Column 1", "Column 2", ...

The @c receive element defines a binary structure to read and has exactly one @c struct child.


# <tt>struct</tt> # {#xmlstruct}
- @b Parent - <tt>onconnect, receive, ondisconnect</tt>
- @b Children - field
The @c struct element identifies the binary structure of a message. It is used to define both
incoming and outgoing messages, though some attributes of @c field elements change for each use
case.

A @c struct has any number of @c field elements, each of which identify a data member of the
structure, including type, name and value for outgoing messages. The order of the fields controls
the order of serialisation.

The @c struct has the following attributes:
Attribute | Description
--------- | -----------------------------------------------------------------------
endian    | "network", "little" or "big". Network Endian is an alias for "big" as this is commonly used for network transmission. The default is "network".

A @c field element has the following attributes.
Attribute | Description
--------- | -----------------------------------------------------------------------
type      | The attribute data type. Also used for structure alignment and padding. See below.
name      | The display name for the field.
heading   | Optional, used with @c receive only. Only fields with @c heading set to "true" are plotted. All other fields are ignored. The default is "false".

A @c field may also have a text value (between the start and end tags), which sets the value for
that field. The text must be a valid string for the selected @c type. Fields with values are only
used for @c onconnect and @c ondisconnect structures or @c receive structure fields with a padding
type (see below).

The following field types are supported:
Type    | Description
------- | -----------------
int8    | 8-bit, signed integer
int16   | 16-bit, signed integer
int32   | 32-bit, signed integer
int64   | 64-bit, signed integer
uint8   | 8-bit, unsigned integer
uint16  | 16-bit, unsigned integer
uint32  | 32-bit, unsigned integer
uint64  | 64-bit, unsigned integer
float   | Single precision, 32-bit floating point number.
double  | Double precision, 64-bit floating point number.

Additionally, the type may be set to one of the following padding types. A padding type must have
a value specified in the element text, which controls how much padding to apply (always in bytes).
Padding Type    | Description
--------------- | -----------------
@c pad          | Pads the structure by skipping the specified number of bytes (relative padding).
@c padto        | Pads the structure out to the given byte value (absolute padding). No effect if already beyond this byte.
@c padbyfield   | Like @c pad, but but the number of padding bytes is read from a preceding field. The element text matches the name of the field, which must appear before this field.
@c padtofield   | Like @c padto, but but the padding byte is read from a preceding field. The element text matches the name of the field, which must appear before this field.

The @c padtofield may be used as a skip value for variable sized messages; e.g., skip to the end of
the message by using a "messagesize" member field for the padding byte position.

*/

/*!
@page urtex Real Time Source XML Examples

# Serial Example #
@include realtime-serial.xml

# TCP Example #
@include realtime-tcp.xml

# UDP Example #
@include realtime-udp.xml

*/

/*!
@page uexp Expressions
Expressions allow plots to be compared against each other using simple expressions. For example,
consider a file containing columns for "X" and "X_corrected". An expression comparing the error
between the two can be graphed by defining:
@verbatim
  X - X_corrected
@endverbatim

The [expressions UI] (#uexpressions) is used to add, remove and edit expressions.

Expressions are calculated after loading all pending data files and generally relate plots from
within the same data file. Expressions support the following syntax and features:
- Plots may be reference by name.
- Plots may be referenced by regular expression.
- Constant expressions.
- Mathematical operators. The following mathematical operations are supported:
  - division (/)
  - multiplication (*)
  - addition (+)
  - subtraction (-)
  - exponents (^) : e.g., x ^ 2 for x squared.
- Unary '-' operator.
- Brackets to control precedence.
- Functions. Function expressions are of the form: func(arg1, arg2, ...)
  - For available functions, run Open Curves and see the [expressions UI] (#uexpressions).

An expression plot will only be visible for files that contain columns matching all expression
items.

# Plot References # {#xplotreferences}

Plot references are matched by name, with optional single or double quotes surrounding the plot
name. Quotes are only required if the plot name contains spaces (see @ref uformats) or if the
name includes quotes. Below are some examples:
- v_North
- "alt(m)"
- 'imperial_measurement (")'

Regular expressions are also supported and perform exhaustive matching. Regular expressions require
single or double quotes and are prefixed by an upper or lower case an 'r' (or 'R'). Below are some
examples:
- r'v_.*'
- R'(pitch|roll|yaw).*'
- r"gravity.*"

Literal or constant values support any valid numeric string. This includes negative numbers and
scientific notation in the form '1.534e-3'.

# Advanced Plot References # {#xadvancedreferences}
Plot references also support the following advanced features:
- Indexing
- Slicing
- File specific references

## Indexing ## {#xindexing}
Plot references may be indexed using the following syntax:
@verbatim
  plot_name[time]
@endverbatim

The time may be an expression and uses the value of the 'time' expression to reference a sample in
'plot_name' at the matching time. Returns zero for out of range references.

## Slicing ## {#xslicing}
Slicing syntax may be used to display a smaller portion of the original plot. Slicing syntax is:
@verbatim
  plot_name[start:end]
@endverbatim

This draws the parts of 'plot_name' between the start and end times (inclusive). Specifying both
start and end is optional allowing only a start or end to be specified. The other is assumed to be
from first sample, or to the end sample respectively. Examples:
- plot_name[40:100] : slice 'plot_name' from the 40th to the 100th sample.
- plot_name[100:] : slice 'plot_name' from sample time 100 to the end.
- plot_name[:100] : slice 'plot_name' from the first sample to sample time 100.

## File specific references ## {#xfilereferences}
A plot reference may specify matching only a specific plot file using the following syntax:
@verbatim
  plot_file|plot_name
@endverbatim

This matches 'plot_name', but only when loaded from 'plot_file'. Note that the 'plot_file' string
matches the string which appears in the "Files" list in the Open Curves UI. The file name and plot name
are quoted separately. The following quoting patterns are acceptable:
- "plot_file"|plot_name
- 'plot_file'|'plot_name'
- 'plot_file'|"plot_name"

The following quoting pattern is not a file specific reference and will instead look for a column
matching the entire string:
- 'plot_file|plot_name' - wrong

Using file specific references, it is possible to generate plots which compare results between
different files. For example, consider the expression:
@verbatim
  file1|column1 - file2|column1
@endverbatim

This will plot the difference between column1 in file1 and file2. The file specification may
optionally be quoted usign either single or double quotes. The abs() function can be used to plot
only the magnitude of the result:
@verbatim
  abs(file1|column1 - file2|column1)
@endverbatim

File specific references also support regular expressions. It is possible to compare column1
between all loaded files using the expression:
@verbatim
  r'.*'|column1 - r'.*'|column1
@endverbatim

@b Note: since regular expression matching is exhaustive, this will result in mirrored graphs,
comparing file1 to file2 and file2 to file1.

*/

/*!
@page ubookmarks Bookmarks
Open Curves supports bookmarking the current state of the application including the UI splits and
layout and the loaded data files and views (real-time sources are excluded). Open Curves supports ten
seperate bookmarks, a bookmark for the last state on close and import/export of bookmarks.

To bookmark the current state, select the menu option "Bookmark -> Set -> Bookmark #" where
"Bookmark #" identifies the bookmark number to use. The bookmark may then optionally be named to
better distinguish it than by its number. To restore a bookmark, select
"Bookmarks -> Goto -> Bookmark #". Hot keys are also available.

Bookmarks may also be exported and imported from the "Bookmarks" menu.

A bookmark stores the following information about the current application state:
- The UI layout including windows sizes, visible panels and plot split views.
- The visible plots and the data files from which they are loaded.
- Expressions.

*/

#endif // DOCMAIN_H_
