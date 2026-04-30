/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetWriterService_h
#define mitkPointSetWriterService_h

#include <mitkAbstractFileWriter.h>
#include <mitkPointSet.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk
{
  class Geometry3D;

  /**
   * \internal
   *
   * \brief XML-based writer for mitk::PointSets.
   *
   * Serializes mitk::PointSet objects to an XML file format. Supports writing
   * multi-timestep point sets. The output includes point coordinates, IDs,
   * specifications, and geometry data for each time step.
   *
   * \ingroup IO
   * \sa PointSetReaderService
   * \sa AbstractFileWriter
   */
  class PointSetWriterService : public AbstractFileWriter
  {
  public:
    PointSetWriterService();
    ~PointSetWriterService() override;

    using AbstractFileWriter::Write;

    /**
     * \brief Write the input PointSet to XML format.
     *
     * Serializes all time steps of the PointSet into a TinyXML2 document
     * and writes it to the output location.
     */
    void Write() override;

  private:
    PointSetWriterService(const PointSetWriterService &other);

    mitk::PointSetWriterService *Clone() const override;

    /**
     * \brief Convert a numeric value to string.
     * \tparam T The numeric type.
     * \param[in] value The value to convert.
     * \return The string representation.
     */
    template <typename T>
    std::string ConvertToString(T value);

    /**
     * \brief Serialize a PointSet to a TinyXML2 element.
     * \param[in,out] doc The XML document that owns the created elements.
     * \param[in] pointSet The PointSet to serialize.
     * \return The root XML element representing the PointSet.
     */
    tinyxml2::XMLElement *ToXML(tinyxml2::XMLDocument &doc, const mitk::PointSet *pointSet);

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
