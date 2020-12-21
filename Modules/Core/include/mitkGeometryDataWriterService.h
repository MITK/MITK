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
   * @internal
   *
   * @brief XML-based writer for mitk::GeometryData
   *
   * XML-based writer for mitk::GeometryData. Multiple GeometryData instances
   * can be written in a single XML file by simply setting multiple inputs to the filter.
   *
   * @sa Geometry3DToXML
   *
   * @ingroup IO
   */
  class GeometryDataWriterService : public AbstractFileWriter
  {
  public:
    GeometryDataWriterService();
    ~GeometryDataWriterService() override;

    using AbstractFileWriter::Write;
    void Write() override;

  private:
    GeometryDataWriterService(const GeometryDataWriterService &other);

    mitk::GeometryDataWriterService *Clone() const override;
  };
}

#endif
