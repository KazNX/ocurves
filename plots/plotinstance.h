//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTINSTANCE_H_
#define PLOTINSTANCE_H_

#include "plotsconfig.h"

#include "plotsource.h"

#include <QColor>
#include <QMutex>
#include <QPointF>
#include <QString>

#include <cstdint>
#include <vector>

class PlotDataCurve;
class PointSeriesData;
class PlotExpression;

/// @ingroup plot
/// Holds data for a single curve.
///
/// Supports thread-safe, double buffered loading. The data source thread should call
/// @c addPoint() or @c addPoints() to add data. Meanwhile the main thread should periodically
/// call @c migrateBuffer() to move data into the front buffer. Note that only the back buffer
/// is thread-safe. The @c sample() function should only be called either by the
/// main thread (the same thread doing the migration) or once all data are loaded.
///
/// While data can be sampled directly via @c sample(), a @c PlotInstanceSampler should
/// be used to resolve time values and time scaling.
///
/// @par Ring Buffer Mode
/// The structure may be operating in ring buffer mode, in which case the @c data array
/// is fixed size and added to as a ring buffer. The @c ringHead marks the start of the
/// ring buffer. The ring buffer is full once the @c data size equals its capacity.
///
/// The @c PlotInstanceSampler handles sampling in ring buffer mode.
class PlotInstance
{
public:
  /// Status, control and display flags.
  enum Flag
  {
    /// Set when all data have been loaded and the curve is complete.
    /// No further calls to @c migrateBuffer() required.
    DataComplete = (1 << 0),
    /// Set if the @c data member acts as a ring buffer. Affects @c sample(), @c addPoint(), @c addPoints().
    RingBuffer = (1 << 1),
    /// Set if the graph has been assigned an explicit colour. No colour shift will be performed in plotting
    /// the curve.
    ExplicitColour = (1 << 2),
    /// Set to filter NaN values, replacing them with zero.
    FilterNaN = (1 << 3),
    /// Set to filter infinite values, replacing them zero.
    FilterInf = (1 << 4),
    /// Has the curve been generated with explicit time samples?
    /// Some generated curves have explicit times and are not subject to
    /// time scaling, time shift or time columns. Such curves may need to be
    /// renenerated if timing information is changed.
    ExplicitTime = (1 << 5),
  };

  /// Some default values for plots.
  enum Defaults
  {
    DefaultSymbolSize = 5   ///< Default size for displaying plot symbols.
  };

  /// Create a plot instance from the given source.
  /// @param source The plot source.
  PlotInstance(const PlotSource::Ptr &source);

  /// Copy constructor.
  PlotInstance(const PlotInstance &other);

  /// Destructor.
  ~PlotInstance();

  /// Access the plot source.
  /// @return The @c PlotSource owning this plot.
  PlotSource &source();

  /// Access the plot source.
  /// @return The @c PlotSource owning this plot.
  const PlotSource &source() const;

  /// Restore the default style, colour, etc properties.
  void setDefaultProperties();

  /// Get the plot name.
  /// @return The plot name.
  inline QString name() const { return _name; }

  /// Set the plot name.
  /// @param name The new plot name.
  inline void setName(QString name) { _name = name; }

  /// Get the draw style: matches @c QwtPlotCurve::CurveStyle.
  /// @return The style type.
  inline int style() const { return _style; }

  /// Set the draw style: matches @c QwtPlotCurve::CurveStyle.
  /// @param style The new style.
  inline void setStyle(unsigned style) { _style = std::int8_t(style); }

  /// Get the draw width.
  /// @return The line width.
  inline unsigned width() const { return _width; }

  /// Set the pen draw width: limited to 255.
  /// @param width The new width.
  inline void setWidth(unsigned width) { _width = std::uint8_t(width); }

  /// Get the draw symbol: matches @c QwtSymbol::Style.
  /// @return The symbol type.
  inline int symbol() const { return _symbol; }

  /// Set the draw symbol: matches @c QwtSymbol::Style.
  /// @param symbol The new symbol.
  inline void setSymbol(int symbol) { _symbol = std::int8_t(symbol); }

  /// Get the symbol size.
  /// @return The symbol size.
  inline unsigned symbolSize() const { return _symbolSize; }

  /// Set the symbol size: limited to 255.
  /// @param size The new symbol size.
  inline void setSymbolSize(unsigned size) { _symbolSize = std::uint8_t(size); }

  /// Direct access to the data buffer. Use @c sample() for controlled access including ring buffer handling.
  /// @return The internal data buffer (visible buffer).
  inline const std::vector<QPointF> &data() const { return _data; }

  /// Get the display colour for the plot. May be colour shifted when @c explicitColour() is false.
  /// @return The preferred display colour.
  inline const QRgb &colour() const { return _colour; }

  /// Set the display colour, making it explicit unless requested otherwise.
  /// @param colour The new colour.
  /// @param markExplicit True to make @c explictColour() true, preventing colour shift (default).
  void setColour(const QRgb &colour, bool markExplicit = true);

  /// Get the expression which generated this plot (@c Expression @c source() only).
  ///
  /// Note: this is a shared pointer.
  /// @return The generating expression.
  inline const PlotExpression *expression() const { return _expression; }

  /// Sets the expression which generated this plot. Should only be called for
  /// @c Expression @c source().
  /// @param expression The generating expression.
  inline void setExpression(const PlotExpression *expression) { _expression = expression; }

  /// The read head of the ring buffer. Always zero if not @c isRingBuffer().
  /// @return The read head index.
  inline size_t ringHead() const { return _ringHead; }

  /// Get the status, control and display @c Flag values.
  /// @return The set @c Flag values.
  inline std::uint16_t flags() const { return _flags; }

  /// Sets that all @p flags are set in the @c flags() member.
  /// @return True if all bits an @p values are set internally.
  inline bool testFlags(std::uint16_t flags) { return (_flags & flags) == flags; }

  /// Is data loading complete?
  /// @return True if all data are present. No further calls to @c migrateBuffer()
  ///   required.
  inline bool dataComplete() const { return (_flags & DataComplete) != 0; }

  /// Marks data complete.
  inline void setComplete() { setFlagsState(DataComplete, true);  }

  /// Is this a ring buffer?
  /// @return True if using a ring buffer.
  inline bool isRingBuffer() const { return (_flags & RingBuffer) != 0; }

  /// Converts the plot data to a ring buffer of the given size.
  ///
  /// This may (safely) resize the existing ring buffer if already a ring buffer.
  /// @param bufferSize The size of the ring buffer.
  void makeRingBuffer(size_t bufferSize);

  /// Using an explicit colour?
  /// @return True if using an explicit colour.
  inline bool explicitColour() const { return (_flags & ExplicitColour) != 0; }

  /// Filter NaN values on display?
  /// @return True if filtering NaN.
  inline bool filterNaN() const { return (_flags & FilterNaN) != 0; }

  /// Set or clear NaN filtering.
  /// @param filter True to turn on filtering, false to turn off.
  inline void setFilterNaN(bool filter) { setFlagsState(FilterNaN, filter); }

  /// Filter infinite values on display?
  /// @return True if filtering infinite values.
  inline bool filterInf() const { return (_flags & FilterInf) != 0; }

  /// Set or clear infinite value filtering.
  /// @param filter True to turn on filtering, false to turn off.
  inline void setFilterInf(bool filter) { setFlagsState(FilterInf, filter); }

  /// Has the curve been generated with explicit time values?
  /// @return True if generated with explicit time values.
  inline bool explicitTime() const { return (_flags & ExplicitTime) != 0; }

  /// Set the state of the @c ExplicitTime @c Flag.
  /// @param explct True to set @c ExplicitTime, false to clear.
  inline void setExplicitTime(bool explct) { setFlagsState(ExplicitTime, explct); }

  /// Samples the point at the given index. This caters for ring buffer sampling.
  ///
  /// For a non-ring buffer, the @c index must be in range. For a ring buffer,
  /// the index is wrapped into the valid range, except when the buffer is empty.
  ///
  /// @param index The sampling index.
  QPointF sample(size_t index) const;

  /// Add a point to the back buffer (thread-safe).
  /// @param p The point to add.
  void addPoint(const QPointF &p);

  /// Add multiple points to the back buffer.
  /// @param points The points array to add.
  /// @param pointCount The element count of @p points.
  void addPoints(const QPointF *points, size_t pointCount);

  /// Migrate from the back buffer to @c data(). Main thread only.
  bool migrateBuffer();

  /// Assignment operator.
  /// Copies all members excluding the data back buffer.
  /// @param other The object to copy.
  PlotInstance &operator=(const PlotInstance &other);

private:
  /// Set or clear @c flags.
  /// @param flags The bit field of interest.
  /// @param set True to set, false to clear.
  void setFlagsState(std::uint16_t flags, bool set);

  std::vector<QPointF> _data;  ///< Plot data: only for access on the main thread.
  PlotSource::Ptr _source;  ///< The owning source of this plot instance.
  QString _name;       ///< Name or heading of the curve.
  QRgb _colour;
  const PlotExpression *_expression; ///< Set if generated from an expression.
  /// For when @c data array is used as a ring buffer. Marks the read head.
  size_t _ringHead;
  std::uint16_t _flags;     ///< Various @c Flag values set.
  std::int8_t _style;       ///< Style, matching @c QwtPlotCurve::CurveStyle.
  std::uint8_t _width;      ///< Draw width.
  std::int8_t _symbol;      ///< Overlay symbol.
  std::uint8_t _symbolSize; ///< Size for symbols.

  QMutex _mutex;
  std::vector<QPointF> _buffer;  ///< Back buffer for loading thread.
};


inline PlotSource &PlotInstance::source()
{
  return *_source;
}


inline const PlotSource &PlotInstance::source() const
{
  return *_source;
}


inline void PlotInstance::setColour(const QRgb &colour, bool markExplicit)
{
  _colour = colour;
  // Set ExplicitColour flag as requested.
  setFlagsState(ExplicitColour, markExplicit);
}


inline void PlotInstance::setFlagsState(std::uint16_t flags, bool set)
{
  _flags = (_flags & ~flags) | (!!set * flags);
}

#endif // PLOTINSTANCE_H_
