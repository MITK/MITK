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

#ifndef _MITK_CONTOURMODEL_WRITER__H_
#define _MITK_CONTOURMODEL_WRITER__H_

#include <mitkAbstractFileWriter.h>
#include <mitkContourModel.h>

//DEPRECATED
#include <mitkTimeGeometry.h>

namespace mitk
{

/**
 * @brief XML-based writer for mitk::ContourModels
 *
 * XML-based writer for mitk::ContourModels. Multiple ContourModels can be written in
 * a single XML file by simply setting multiple inputs to the filter.
 *
 * The xml file will look like:
 *
 *   <?xml version="1.0" encoding="utf-8"?>
 *   <contourModel>
 *      <head>
 *        <geometryInfo>
 *        </geometryInfo>
 *      </head>
 *      <data>
 *        <timestep n="0">
 *          <controlPoints>
 *            <point>
 *              <x></x>
 *              <y></y>
 *              <z></z>
 *            </point>
 *          </controlPoint>
 *        </timestep>
 *      </data>
 *    </contourModel>
 *
 * @ingroup PSIO
 * @ingroup Process
 */

class TimeSlicedGeometry;

class ContourModelWriter : public mitk::AbstractFileWriter
{
public:

  ContourModelWriter();
  virtual ~ContourModelWriter();

  using AbstractFileWriter::Write;
  virtual void Write();

protected:

  ContourModelWriter(const ContourModelWriter& other);

  virtual mitk::ContourModelWriter* Clone() const;

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
     * @param contourModel the point set to be converted to xml
     * @param out the stream to write to.
     */
    void WriteXML( const mitk::ContourModel* contourModel, std::ostream& out );

    /**
    * Writes the geometry information of the TimeGeometry to an outstream.
    * The root tag is not included.
    * @param geometry the TimeGeometry of the contour.
    * @param the stream to write to.
    */
    void WriteGeometryInformation( const mitk::TimeGeometry* geometry, std::ostream& out );

    /**
    * Writes the geometry information of the TimeGeometry to an outstream.
    * The root tag is not included.
    * @param geometry the TimeGeometry of the contour.
    * @param the stream to write to.
    *
    * \deprecatedSince{2013_09} Please use TimeGeometry instead of TimeSlicedGeometry. For more information see http://www.mitk.org/Development/Refactoring%20of%20the%20Geometry%20Classes%20-%20Part%201
    */
    DEPRECATED(void WriteGeometryInformation(const mitk::TimeSlicedGeometry* geometry, std::ostream& out ));

    /**
     * Writes an standard xml header to the given stream.
     * @param file the stream in which the header is written.
     */
    void WriteXMLHeader( std::ostream &file );

    /** Write a start element tag */
    void WriteStartElement( const char *const tag, std::ostream &file );

    void WriteStartElementWithAttribut( const char *const tag, std::vector<std::string> attributes, std::vector<std::string> values, std::ostream &file );

    /**
     * Write an end element tag
     * End-Elements following character data should pass indent = false.
     */
    void WriteEndElement( const char *const tag, std::ostream &file, const bool& indent = true );

    /** Write character data inside a tag. */
    void WriteCharacterData( const char *const data, std::ostream &file );

    /** Write a start element tag */
    void WriteStartElement( std::string &tag, std::ostream &file );

    /** Write an end element tag */
    void WriteEndElement( std::string &tag, std::ostream &file, const bool& indent = true );

    /** Write character data inside a tag. */
    void WriteCharacterData( std::string &data, std::ostream &file );

    /** Writes empty spaces to the stream according to m_IndentDepth and m_Indent */
    void WriteIndent( std::ostream& file );

    unsigned int m_IndentDepth;

    unsigned int m_Indent;

public:

    static const char* XML_CONTOURMODEL;

    static const char* XML_HEAD;

    static const char* XML_GEOMETRY_INFO;

    static const char* XML_DATA;

    static const char* XML_TIME_STEP;

    static const char* XML_CONTROL_POINTS;

    static const char* XML_POINT;

    static const char* XML_X;

    static const char* XML_Y;

    static const char* XML_Z;


};



}


#endif
