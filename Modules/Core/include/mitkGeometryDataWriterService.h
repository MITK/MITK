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


#ifndef mitkGeometryDataWriterService_h
#define mitkGeometryDataWriterService_h

#include <mitkAbstractFileWriter.h>
#include <mitkGeometryData.h>

// forward
class TiXmlNode;

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
  virtual ~GeometryDataWriterService();

  using AbstractFileWriter::Write;
  virtual void Write() override;

private:

  GeometryDataWriterService(const GeometryDataWriterService& other);

  virtual mitk::GeometryDataWriterService* Clone() const override;
};

}

#endif
