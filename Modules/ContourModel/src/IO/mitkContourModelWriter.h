/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelWriter_h
#define mitkContourModelWriter_h

#include <mitkAbstractFileWriter.h>
#include <mitkContourModel.h>
#include <mitkTimeGeometry.h>

namespace mitk
{
  /**
   * \brief XML-based writer for mitk::ContourModels.
   *
   * XML-based writer for mitk::ContourModels. Multiple ContourModels can be written in
   * a single XML file by simply setting multiple inputs to the filter.
   *
   * The xml file will look like:
   *
   * \code{.unparsed}
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
   * \endcode
   *
   * \ingroup MitkContourModelModule
   */

  class ContourModelWriter : public mitk::AbstractFileWriter
  {
  public:
    /**
     * \brief Construct the writer, optionally including an XML header.
     * \param writeXMLHeader If true (default), the XML declaration is written at the top of the file.
     */
    explicit ContourModelWriter(bool writeXMLHeader = true);
    ~ContourModelWriter() override;

    using AbstractFileWriter::Write;

    /** \brief Write the ContourModel to the configured output location. */
    void Write() override;

  protected:
    ContourModelWriter(const ContourModelWriter &other);

    mitk::ContourModelWriter *Clone() const override;

    /**
     * \brief Convert an arbitrary type to a string.
     *
     * The type must support the << operator. Works for integral data types
     * such as float, int, long, etc.
     *
     * \tparam T The type to convert.
     * \param value The value to convert.
     * \return The string representation of value.
     */
    template <typename T>
    std::string ConvertToString(T value);

    /**
     * \brief Write an XML representation of the given contour model to a stream.
     *
     * The XML header and root node are not included.
     *
     * \param contourModel The contour model to be serialized.
     * \param out The output stream to write to.
     */
    void WriteXML(const mitk::ContourModel *contourModel, std::ostream &out);

    /**
     * \brief Write the geometry information of a TimeGeometry to a stream.
     *
     * The root tag is not included.
     *
     * \param geometry The TimeGeometry of the contour.
     * \param out The output stream to write to.
     */
    void WriteGeometryInformation(const mitk::TimeGeometry *geometry, std::ostream &out);

    /**
     * \brief Write a standard XML header to the given stream.
     * \param file The output stream.
     */
    void WriteXMLHeader(std::ostream &file);

    /**
     * \brief Write a start element tag.
     * \param tag The element tag name.
     * \param file The output stream.
     */
    void WriteStartElement(const char *const tag, std::ostream &file);

    /**
     * \brief Write a start element tag with attributes.
     * \param tag The element tag name.
     * \param attributes The attribute names.
     * \param values The attribute values (parallel to attributes).
     * \param file The output stream.
     */
    void WriteStartElementWithAttribut(const char *const tag,
                                       std::vector<std::string> attributes,
                                       std::vector<std::string> values,
                                       std::ostream &file);

    /**
     * \brief Write an end element tag.
     *
     * End-elements following character data should pass indent = false.
     *
     * \param tag The element tag name.
     * \param file The output stream.
     * \param indent Whether to write indentation before the tag.
     */
    void WriteEndElement(const char *const tag, std::ostream &file, const bool &indent = true);

    /**
     * \brief Write character data inside a tag.
     * \param data The character data string.
     * \param file The output stream.
     */
    void WriteCharacterData(const char *const data, std::ostream &file);

    /**
     * \brief Write a start element tag (std::string overload).
     * \param tag The element tag name.
     * \param file The output stream.
     */
    void WriteStartElement(std::string &tag, std::ostream &file);

    /**
     * \brief Write an end element tag (std::string overload).
     * \param tag The element tag name.
     * \param file The output stream.
     * \param indent Whether to write indentation before the tag.
     */
    void WriteEndElement(std::string &tag, std::ostream &file, const bool &indent = true);

    /**
     * \brief Write character data inside a tag (std::string overload).
     * \param data The character data string.
     * \param file The output stream.
     */
    void WriteCharacterData(std::string &data, std::ostream &file);

    /** \brief Write indentation spaces to the stream according to m_IndentDepth and m_Indent. */
    void WriteIndent(std::ostream &file);

    bool m_WriteXMLHeader;

    unsigned int m_IndentDepth;

    unsigned int m_Indent;

  public:
    static const char *XML_CONTOURMODEL;

    static const char *XML_HEAD;

    static const char *XML_GEOMETRY_INFO;

    static const char *XML_DATA;

    static const char *XML_TIME_STEP;

    static const char *XML_CONTROL_POINTS;

    static const char *XML_POINT;

    static const char *XML_X;

    static const char *XML_Y;

    static const char *XML_Z;
  };
}

#endif
