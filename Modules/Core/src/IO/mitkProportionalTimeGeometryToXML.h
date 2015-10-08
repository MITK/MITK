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
  static TiXmlElement* ToXML( const ProportionalTimeGeometry* geometry);

  /**
   * @brief Create a ProportionalTimeGeometry from XML.
   * Interprets only the format created by ToXML().
   */
  static ProportionalTimeGeometry::Pointer FromXML( TiXmlElement* node );
};

}

#endif
