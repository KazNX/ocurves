//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef STRINGITEMS_H
#define STRINGITEMS_H

#include "ocurvesconfig.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

/// An array of string pointers (into another array) using the given character type.
///
/// Used in place of QVector because the latter deallocates on resize to zero.
/// We want to reuse the memory.
/// @tparam CHAR The character type. Generally @c char or @c wchar_t.
template <typename CHAR>
class StringItems
{
public:
  /// Constructor.
  StringItems();
  /// Destructor.
  ~StringItems();

  /// Return the number of items in the array.
  /// @return The number of items.
  inline size_t size() const { return _count; }

  /// Return the number of items in the array.
  /// @return The number of items.
  inline size_t count() const { return _count; }

  /// Return the current capacity of the array.
  /// @return The current capacity.
  inline size_t capacity() const { return _capacity; }

  /// Array accessor. Returns the item at @p index.
  /// @param index The requested item index, in the range [0, @c count()).
  ///   Out of range requests result in undefined behaviour.
  /// @return The string at @p index.
  const CHAR *operator[](size_t index) const { return _items[index]; }
  /// @overload
  const CHAR *operator[](int index) const { return _items[index]; }
  /// @overload
  const CHAR *operator[](int64_t index) const { return _items[index]; }

  /// Resets the number of items to zero, leaving the capacity unchanged.
  inline void reset() { _count = 0; }

  /// Resizes such that the count matches @p size. The capacity is adjusted
  /// up if required, but not down.
  /// @param size The requested size.
  void resize(size_t size);

  /// Ensures that the capacity is at least equal to @p capacity.
  ///
  /// The capacity does not change if @p capacity is less than or equal to
  /// the current capaccity.
  /// @param capacity The requested capacity.
  void reserve(size_t capacity);

  /// Adds an item to the array, incrementing the count and adjusting the capacity
  /// if required.
  /// @param item The string to insert.
  void push_back(const CHAR *item);

private:
  const CHAR **_items;  ///< String pointer array.
  size_t _count;  ///< Number of current items.
  size_t _capacity; ///< Current maximum number of items.
};


template <typename CHAR>
StringItems<CHAR>::StringItems()
  : _items(nullptr)
  , _count(0)
  , _capacity(0)
{
}


template <typename CHAR>
StringItems<CHAR>::~StringItems()
{
  delete[] _items;
}


template <typename CHAR>
void StringItems<CHAR>::resize(size_t size)
{
  reserve(size);
  _count = size;
}


template <typename CHAR>
void StringItems<CHAR>::reserve(size_t capacity)
{
  if (_capacity < capacity)
  {
    // Reallocate.
    const CHAR **newItems = new const CHAR *[capacity];
    if (_count)
    {
      memcpy(newItems, _items, sizeof(*newItems) * _count);
    }
    delete[] _items;
    _items = newItems;
    _capacity = capacity;
  }
}


template <typename CHAR>
void StringItems<CHAR>::push_back(const CHAR *item)
{
  if (_count == _capacity)
  {
    reserve((_capacity) ? _capacity << 1 : 8);
  }
  _items[_count++] = item;
}


#endif  // STRINGITEMS_H
