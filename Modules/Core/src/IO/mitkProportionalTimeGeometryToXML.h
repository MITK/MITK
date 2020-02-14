/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProportionalTimeGeometryToXML_h
#define mitkProportionalTimeGeometryToXML_h

// MITK
#include "mitkProportionalTimeGeometry.h"

class TiXmlElement;

namespace mitk
{
  /**
   * @internal
   *
   * @brief Helper for ProportionalTimeGeometry related I/O classes.
   *
   * Creates TinyXML elements (blocks) that describe a ProportionalTimeGeometry.
   * Also offers a method to read such blocks and create a corresponding ProportionalTimeGeometry.
   *
   * @sa GeometryDataWriterService, GeometryDataReaderService, Geometry3DToXML
   *
   * @ingroup IO
   */
  class ProportionalTimeGeometryToXML
  {
  public:
    /**
     * @brief Serialize given geometry to XML.
     */
    static TiXmlElement *ToXML(const ProportionalTimeGeometry *geometry);

    /**
     * @brief Create a ProportionalTimeGeometry from XML.
     * Interprets only the format created by ToXML().
     */
    static ProportionalTimeGeometry::Pointer FromXML(TiXmlElement *node);
  };
}

#endif
