/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometry3DToXML_h
#define mitkGeometry3DToXML_h

// MITK
#include "mitkGeometry3D.h"

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

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
    static tinyxml2::XMLElement *ToXML(tinyxml2::XMLDocument& doc, const Geometry3D *geometry);

    /**
     * @brief Create a Geometry3D from XML.
     * Interprets only the format created by ToXML().
     */
    static Geometry3D::Pointer FromXML(const tinyxml2::XMLElement *node);
  };
}

#endif
