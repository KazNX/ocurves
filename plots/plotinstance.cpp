//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotinstance.h"

PlotInstance::PlotInstance(const PlotSource::Ptr &source)
  : _source(source)
  , _expression(nullptr)
  , _ringHead(0u)
  , _flags(0)
  , _style(0)
  , _width(0)
  , _symbol(-1)
  , _symbolSize(DefaultSymbolSize)
{
}


PlotInstance::PlotInstance(const PlotInstance &other)
{
  *this = other;
}


PlotInstance::~PlotInstance()
{
}


void PlotInstance::setDefaultProperties()
{
  setFlagsState(ExplicitColour | FilterInf | FilterNaN, false);
  _width = 0;
  _style = 0;
  _symbol = -1;
  _symbolSize = DefaultSymbolSize;
}


void PlotInstance::makeRingBuffer(size_t bufferSize)
{
  if (_data.size() <= bufferSize)
  {
    _data.reserve(bufferSize);
  }
  else
  {
    _data.resize(bufferSize);
  }
  setFlagsState(RingBuffer, true);
  _ringHead = std::min(_ringHead, _data.size());
}


QPointF PlotInstance::sample(size_t index) const
{
  QPointF sampl;
  if (!_data.empty())
  {
    if (!isRingBuffer())
    {
      sampl = _data[std::min(index, _data.size() - 1)];
    }
    else
    {
      const size_t rotatedIndex = (index + _ringHead) % _data.size();
      sampl = _data[rotatedIndex];
    }
  }

  return sampl;
}


void PlotInstance::addPoint(const QPointF &p)
{
  QMutexLocker guard(&_mutex);
  _buffer.push_back(p);
}


void PlotInstance::addPoints(const QPointF *points, size_t pointCount)
{
  QMutexLocker guard(&_mutex);
  _buffer.reserve(_buffer.size() + pointCount);
  _buffer.insert(_buffer.end(), points, points + pointCount);
}


bool PlotInstance::migrateBuffer()
{
  QMutexLocker guard(&_mutex);
  if (!_buffer.empty())
  {
    if (!isRingBuffer())
    {
      _data.reserve(_data.size() + _buffer.size());
      _data.insert(_data.end(), _buffer.begin(), _buffer.end());
    }
    else
    {
      // Adding in ring buffer mode.
      // Capacity check.
      const QPointF *samples = _buffer.data();
      size_t addCount = _buffer.size();
      if (_buffer.size() >= _data.capacity())
      {
        // Number of new samples equals or exceeds our capacity. Reset.
        size_t startIndex = addCount - _data.capacity();
        _ringHead = 0;
        _data.resize(_data.capacity());
        memcpy(_data.data(), samples + startIndex, sizeof(QPointF) * _data.capacity());
      }
      else
      {
        // Inserting less than capacity.
        size_t insertAt;
        const bool full = _data.size() >= size_t(_data.capacity());
        if (!full)
        {
          // Insert before buffer is full. Add to fill up first.
          insertAt = _data.size();
          const size_t remaining = _data.capacity() - _data.size();
          size_t insertCount = std::min<size_t>(remaining, addCount);
          _data.resize(_data.size() + insertCount);
          memcpy(_data.data() + insertAt, samples, sizeof(QPointF) * insertCount);
          addCount -= insertCount;
          samples += insertCount;
        }

        // More to insert?
        if (addCount)
        {
          // We are full now and have more to insert. Will overwrite samples.
          insertAt = _ringHead;
          _ringHead = (_ringHead + addCount) % _data.capacity();

          // First insertion from the read head
          const size_t copyCount1 = std::min<size_t>(addCount, _data.capacity() - insertAt);
          memcpy(_data.data() + insertAt, samples, sizeof(QPointF) * copyCount1);
          const size_t copyCount2 = addCount - copyCount1;
          if (copyCount2)
          {
            // Second insert: overflow.
            memcpy(_data.data(), samples + copyCount1, sizeof(QPointF) * copyCount2);
          }
        }
      }
    }
    _buffer.clear();
    return true;
  }

  return false;
}


PlotInstance &PlotInstance::operator=(const PlotInstance &other)
{
  _name = other._name;
  _source = other._source;
  _data = other._data;
  _colour = other._colour;
  _expression = other._expression;
  _ringHead = other._ringHead;
  _source = other._source;
  _flags = other._flags;
  return *this;
}
