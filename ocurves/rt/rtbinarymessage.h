//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef RTBINARYMESSAGE_H_
#define RTBINARYMESSAGE_H_

#include "ocurvesconfig.h"

#include "rtmessage.h"

#include <QVariant>
#include <QVector>

class QDataStream;

/// @ingroup realtime
/// Represents a binary based message for real-time data plots.
///
/// This stores a number of fields and may read or write these appropriately.
/// Each field has a known type and a default value. When writing, each field
/// has its default value written to the buffer. When reading, each field is
/// read from the stream, changing its value.
///
/// A field may also be a padding field. Padding may be specified
/// - @c Padding - pad by adding a fixed byte count
/// - @c PadTo - pad the buffer out to the specified byte count
/// - @c PadToField - pad the buffer out to a specified byte count.
///   The byte count is attained by reading another field. The padding
///   field name is stored as a string in the original field's value.
///
/// The same padding process is used on reading or writing, with zero bytes
/// used for writing padding, and padding bytes skipped while reading.
///
/// Supports writing little or big Endian.
///
/// Registered fields may be marked as headings. Only those marked as such
/// are reported by @c headings() and @c populateValues().
class RTBinaryMessage : public RTMessage
{
public:
  /// Fixed field types.
  enum FieldType
  {
    Padding,  ///< Padding. Integer value specifies how much.
    PadTo,  ///< Pad out to the specified amount.
    PadByField,  ///< Relative padding like @c Padding, but lookup a field for the amount.
    PadToField,  ///< Pad out to the specified amount, but lookup a field for the amount.
    Int8,
    Int16,
    Int32,
    Int64,
    Uint8,
    Uint16,
    Uint32,
    Uint64,
    Float32,
    Float64
  };

  /// Sizes for various @c FieldType items (bytes).
  static const unsigned TypeSizes[];

  /// Create a binary message.
  /// @param littleEndian True to write little Endian, false to write big.
  RTBinaryMessage(bool littleEndian = false);

  /// Does the message write little Endian?
  /// @return True if writing little Endian, false if writing big.
  inline bool littleEndian() const { return _littleEndian; }

  /// Writes the binary message to @p buffer.
  /// @param buffer The buffer to write to.
  void setMessage(QByteArray &buffer) override;

  /// Read and update values from the @p buffer.
  /// @param buffer The buffer to read from.
  /// @return The number of bytes read on success, negative on error. Zero if nothing to read.
  int readMessage(const QByteArray &buffer) override;

  /// Return the list of headings. This is the list of fields marked as headings.
  /// @return The list of fields used as headings.
  QStringList headings() const override;

  /// Request the most up to date field values. Only exposes those marked as headings.
  /// @param values The values array to populate.
  /// @return The number of values written.
  unsigned populateValues(std::vector<double> &values) const override;

  /// Registers a field.
  /// @param name The field name. May be used as a heading.
  /// @param type The field type.
  /// @param heading True if this field is exposed as a heading.
  /// @param value The default or initial value.
  void addField(const QString &name, FieldType type, bool heading = false, const QVariant &value = QVariant());

  /// Query the index of a field named @p key.
  /// @param key The field name to query.
  /// @return The index of @p key or -1 if there is no such field.
  int fieldIndex(const QString &key) const;

  /// Requests a field value by name, converting to a double.
  /// @param key The field name requested.
  /// @return The field value converted to a double. Returns zero on any failure.
  double fieldValueD(const QString &key) const;

  /// Requests a field value by name, returning a @c QVariant.
  /// @param key The field name requested.
  /// @return The field value as a @c QVariant. Returns a null value on any failure, but
  ///   a valid field value may also be null.
  QVariant fieldValueV(const QString &key) const;

private:
  /// Defines a message field.
  struct Field
  {
    QString name;     ///< The field name. May be mapped to a heading name.
    FieldType type;   ///< Type.
    QVariant value;   ///< Value for sending.
    bool heading;     ///< True if this is a heading (something to display).
  };

  /// Write a field to the data stream.
  /// @param stream The stream to write to.
  /// @param field The field to write.
  /// @param pos The position in the stream being written. Used for padding calculations.
  /// @return The number of bytes written.
  uint writeField(QDataStream &stream, const Field &field, uint pos);

  /// Read a field to the data stream.
  /// @param stream The stream to read from.
  /// @param field The field to read into.
  /// @param pos The position in the stream being read. Used for padding calculations.
  /// @return The number of bytes read.
  int readField(QDataStream &stream, Field &field, uint pos);

  QVector<Field> _fields;     ///< Message fields.
  unsigned _messageMinSize;   ///< Minimum message size based on the @c _fields.
  bool _littleEndian;         ///< Read/write little Endian?
  QStringList _headings;      ///< Heading fields extracted into a string list.
};




#endif // RTBINARYMESSAGE_H_
