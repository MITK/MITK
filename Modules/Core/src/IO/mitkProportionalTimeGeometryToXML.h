/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProportionalTimeGeometryToXML_h
#define mitkProportionalTimeGeometryToXML_h

#include <mitkProportionalTimeGeometry.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk
{
  /**
   * \internal
   *
   * \brief Helper for ProportionalTimeGeometry related I/O classes.
   *
   * Creates TinyXML2 elements (blocks) that describe a ProportionalTimeGeometry.
   * Also offers a method to read such blocks and create a corresponding ProportionalTimeGeometry.
   *
   * \sa GeometryDataWriterService
   * \sa GeometryDataReaderService
   * \sa Geometry3DToXML
   *
   * \ingroup IO
   */
  class ProportionalTimeGeometryToXML
  {
  public:
    /**
     * \brief Serialize the given ProportionalTimeGeometry to an XML element.
     *
     * \param[in,out] doc The XML document that owns the created element.
     * \param[in] geometry The ProportionalTimeGeometry to serialize.
     * \return The XML element representing the time geometry.
     */
    static tinyxml2::XMLElement *ToXML(tinyxml2::XMLDocument& doc, const ProportionalTimeGeometry *geometry);

    /**
     * \brief Deserialize a ProportionalTimeGeometry from an XML element.
     *
     * Interprets only the format created by ToXML().
     *
     * \param[in] node The XML element containing the time geometry data.
     * \return A new ProportionalTimeGeometry instance, or nullptr on failure.
     */
    static ProportionalTimeGeometry::Pointer FromXML(const tinyxml2::XMLElement *node);
  };
}

#endif
