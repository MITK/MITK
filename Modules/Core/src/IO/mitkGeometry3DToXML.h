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

#ifndef mitkGeometry3DToXML_h
#define mitkGeometry3DToXML_h

// MITK
#include "mitkGeometry3D.h"

class TiXmlElement;

namespace mitk
{
  /**
   * @internal
   *
   * @brief Helper for Geometry3D related I/O classes.
   *
   * Creates TinyXML elements (blocks) that describe a Geometry3D.
   * Also offers a method to read such blocks and create a corresponding Geometry3D.
   *
   * @sa GeometryDataReaderService, GeometryDataWriterService, PointSetWriterService, PointSetReaderService
   *
   * @ingroup IO
   */
  class Geometry3DToXML
  {
  public:
    /**
     * @brief Serialize given geometry to XML.
     */
    static TiXmlElement *ToXML(const Geometry3D *geometry);

    /**
     * @brief Create a Geometry3D from XML.
     * Interprets only the format created by ToXML().
     */
    static Geometry3D::Pointer FromXML(TiXmlElement *node);
  };
}

#endif
