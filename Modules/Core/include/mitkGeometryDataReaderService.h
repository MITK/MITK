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

#ifndef mitkGeometryDataReaderService_h
#define mitkGeometryDataReaderService_h

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkGeometryData.h>

class TiXmlElement;

namespace mitk
{

/**
 * @internal
 *
 * @brief reads XML representations of mitk::GeometryData from a file/stream.
 *
 * To be used via IOUtil.
 *
 * Reader for XML files containing one or multiple XML represenations of
 * mitk::GeometryData. If multiple mitk::GeometryData objects are stored in one file,
 * these are assigned to multiple BaseData objects.
 *
 * @sa Geometry3DToXML
 *
 * @ingroup IO
 */
class GeometryDataReaderService: public AbstractFileReader
{
public:

  GeometryDataReaderService();
  virtual ~GeometryDataReaderService();

  using AbstractFileReader::Read;
  virtual std::vector< itk::SmartPointer<BaseData> > Read() override;

  /**
   * @brief Provides the MIME type for reader and writer.
   */
  static CustomMimeType GEOMETRY_DATA_MIMETYPE();

private:

  GeometryDataReaderService(const GeometryDataReaderService& other);

  virtual GeometryDataReaderService* Clone() const override;
};

}

#endif
