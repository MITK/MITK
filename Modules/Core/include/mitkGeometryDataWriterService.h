/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryDataWriterService_h
#define mitkGeometryDataWriterService_h

#include <mitkAbstractFileWriter.h>
#include <mitkGeometryData.h>

namespace mitk
{
  // forward
  class Geometry3D;

  /**
   * \internal
   *
   * \brief XML-based writer for mitk::GeometryData.
   *
   * XML-based writer for mitk::GeometryData. Multiple GeometryData instances
   * can be written in a single XML file by simply setting multiple inputs to the filter.
   *
   * \sa Geometry3DToXML
   *
   * \ingroup IO
   */
  class GeometryDataWriterService : public AbstractFileWriter
  {
  public:
    /** \brief Default constructor. Registers the writer service for geometry data MIME type. */
    GeometryDataWriterService();

    /** \brief Destructor. */
    ~GeometryDataWriterService() override;

    using AbstractFileWriter::Write;

    /** \brief Write the input GeometryData to the configured output stream.
     *
     * Serializes the ProportionalTimeGeometry of the input GeometryData as XML.
     * \throws mitk::Exception if the output stream is not valid.
     */
    void Write() override;

  private:
    /** \brief Copy constructor. */
    GeometryDataWriterService(const GeometryDataWriterService &other);

    /** \brief Clone this writer instance. */
    mitk::GeometryDataWriterService *Clone() const override;
  };
}

#endif
