--------------------------------------------------------------------------------
Introduction
--------------------------------------------------------------------------------
Open Curves is a graphing utility for plotting text based, delimited data files such as CSV files. Use Open Curves to display and compare time series data.

Key Features:
- Easily open and plot CSV style time-series data files.
- Pan, zoom and inspect the plotted curves.
- Compare plots visually.
- Compare differences in plots using the expressions editor.
- Plot multiple curves across multiple split views.
- Synchronise view panning and zooming.
- Use bookmarks to save and restore the current view state.

Consult the user documentation for more details on Open Curves features and data formats.

--------------------------------------------------------------------------------
Getting Started:
--------------------------------------------------------------------------------
0. Build Open Curves (see build instructions in ocurves/doc/docbuild.h).
1. Run Open Curves.
2. Drag and drop data files into the Open Curves window, or use File->Open.

Use the file lists and column lists to control which items are plotted. Clicking on an item toggles selection. All columns from all loaded files appear in the columns list, with duplicates removed.

Note: By default data files are sub-sampled to around 200,000 samples. To load all data, locate "Max Samples:" on the toolbar and change the entry to "0".

--------------------------------------------------------------------------------
Graph Interaction:
--------------------------------------------------------------------------------
Mouse interaction is enabled on the graph for panning and zooming.
- Zoom a section by left-click and drag.
- Restore zoom by right-click.
- Move the zoomed window by middle-click and drag.
- Mouse wheel zooms in and out (course).
- Select Pan only or Zoom only tools to use only the left mouse button.

--------------------------------------------------------------------------------
Configuration:
--------------------------------------------------------------------------------
The Open Curves configuration is saved in the user home directory, or C:\Users\<user>\AppData\Roaming on Windows in a 'ocurves.ini' file. It contains the following sections:
- [ui] general user interface settings such as the window geometry.
- [load] stores the last directory and filter and time column and scaling options.
- [stream] last directory and filter for opening a stream description file.
- [plot] section, colours value - lists comma separated, hexadecimal colour values used to control the graph colours.
- [expressions] contains derived graph expressions. See expressions.
