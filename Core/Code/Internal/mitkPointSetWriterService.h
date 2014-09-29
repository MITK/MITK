/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef _MITK_POINT_SET_WRITER_SERVICE__H_
#define _MITK_POINT_SET_WRITER_SERVICE__H_

#include <mitkAbstractFileWriter.h>
#include <mitkPointSet.h>

namespace mitk
{

/**
 * @brief XML-based writer for mitk::PointSets
 *
 * XML-based writer for mitk::PointSet. Multiple PointSets can be written in
 * a single XML file by simply setting multiple inputs to the filter.
 *
 * @ingroup IO
 */
class PointSetWriterService : public AbstractFileWriter
{
public:

  PointSetWriterService();
  virtual ~PointSetWriterService();

  using AbstractFileWriter::Write;
  virtual void Write();

private:

  PointSetWriterService(const PointSetWriterService& other);

  virtual mitk::PointSetWriterService* Clone() const;

  /**
   * Converts an arbitrary type to a string. The type has to
   * support the << operator. This works fine at least for integral
   * data types as float, int, long etc.
   * @param value the value to convert
   * @returns the string representation of value
   */
  template < typename T>
  std::string ConvertToString( T value );

  /**
   * Writes an XML representation of the given point set to
   * an outstream. The XML-Header an root node is not included!
   * @param pointSet the point set to be converted to xml
   * @param out the stream to write to.
   */
  void WriteXML( const mitk::PointSet* pointSet, std::ostream& out );

  /**
   * Writes an standard xml header to the given stream.
   * @param file the stream in which the header is written.
   */
  void WriteXMLHeader( std::ostream &file );

  /**
   * Write an end element tag
   * End-Elements following character data should pass indent = false.
   */
  void WriteEndElement( const std::string& tag, std::ostream &file, const bool& indent = true );

  /** Write a start element tag */
  void WriteStartElement( const std::string &tag, std::ostream &file );

  /** Write character data inside a tag. */
  void WriteCharacterData( const std::string &data, std::ostream &file );

  /** Writes empty spaces to the stream according to m_IndentDepth and m_Indent */
  void WriteIndent( std::ostream& file );

  unsigned int m_IndentDepth;
  const unsigned int m_Indent;

  static const std::string XML_POINT_SET;
  static const std::string XML_TIME_SERIES;
  static const std::string XML_TIME_SERIES_ID;
  static const std::string XML_POINT_SET_FILE;
  static const std::string XML_FILE_VERSION;
  static const std::string XML_POINT;
  static const std::string XML_SPEC;
  static const std::string XML_ID;
  static const std::string XML_X;
  static const std::string XML_Y;
  static const std::string XML_Z;
  static const std::string VERSION_STRING;

};

}

#endif
