/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryDataReaderService_h
#define mitkGeometryDataReaderService_h

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkGeometryData.h>

namespace mitk
{
  /**
   * @internal
   *
   * @brief reads XML representations of mitk::GeometryData from a file/stream.
   *
   * To be used via IOUtil.
   *
   * Reader for XML files containing one or multiple XML representations of
   * mitk::GeometryData. If multiple mitk::GeometryData objects are stored in one file,
   * these are assigned to multiple BaseData objects.
   *
   * @sa Geometry3DToXML
   *
   * @ingroup IO
   */
  class GeometryDataReaderService : public AbstractFileReader
  {
  public:
    GeometryDataReaderService();
    ~GeometryDataReaderService() override;

    using AbstractFileReader::Read;

    /**
     * @brief Provides the MIME type for reader and writer.
     */
    static CustomMimeType GEOMETRY_DATA_MIMETYPE();
  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    GeometryDataReaderService(const GeometryDataReaderService &other);

    GeometryDataReaderService *Clone() const override;
  };
}

#endif
