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
#include <mitkGeometry3D.h>

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
   * \brief Helper for Geometry3D related I/O classes.
   *
   * Creates TinyXML2 elements (blocks) that describe a Geometry3D.
   * Also offers a method to read such blocks and create a corresponding Geometry3D.
   *
   * \sa GeometryDataReaderService
   * \sa GeometryDataWriterService
   * \sa PointSetWriterService
   * \sa PointSetReaderService
   *
   * \ingroup IO
   */
  class Geometry3DToXML
  {
  public:
    /**
     * \brief Serialize the given Geometry3D to an XML element.
     *
     * \param[in,out] doc The XML document that owns the created element.
     * \param[in] geometry The Geometry3D to serialize.
     * \return The XML element representing the geometry.
     */
    static tinyxml2::XMLElement *ToXML(tinyxml2::XMLDocument& doc, const Geometry3D *geometry);

    /**
     * \brief Deserialize a Geometry3D from an XML element.
     *
     * Interprets only the format created by ToXML().
     *
     * \param[in] node The XML element containing the geometry data.
     * \return A new Geometry3D instance, or nullptr on failure.
     */
    static Geometry3D::Pointer FromXML(const tinyxml2::XMLElement *node);
  };
}

#endif
