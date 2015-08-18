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

#include "mitkGeometry3DToXML.h"

#include <tinyxml.h>

TiXmlElement* mitk::Geometry3DToXML::ToXML(const Geometry3D* geom3D)
{
  assert(geom3D);

  // really serialize
  const AffineTransform3D* transform = geom3D->GetIndexToWorldTransform();

  // get transform parameters that would need to be serialized
  AffineTransform3D::MatrixType matrix = transform->GetMatrix();
  AffineTransform3D::OffsetType offset = transform->GetOffset();

  bool isImageGeometry = geom3D->GetImageGeometry();
  BaseGeometry::BoundsArrayType bounds = geom3D->GetBounds();

  // create XML file
  // construct XML tree describing the geometry
  TiXmlElement* geomElem = new TiXmlElement("Geometry3D");
  geomElem->SetAttribute("ImageGeometry", isImageGeometry ? "true" : "false" );
  geomElem->SetAttribute("FrameOfReferenceID", geom3D->GetFrameOfReferenceID());

  // coefficients are matrix[row][column]!
  TiXmlElement* matrixElem = new TiXmlElement("IndexToWorld");
  matrixElem->SetAttribute("type", "Matrix3x3");
  matrixElem->SetDoubleAttribute("m_0_0", matrix[0][0]);
  matrixElem->SetDoubleAttribute("m_0_1", matrix[0][1]);
  matrixElem->SetDoubleAttribute("m_0_2", matrix[0][2]);
  matrixElem->SetDoubleAttribute("m_1_0", matrix[1][0]);
  matrixElem->SetDoubleAttribute("m_1_1", matrix[1][1]);
  matrixElem->SetDoubleAttribute("m_1_2", matrix[1][2]);
  matrixElem->SetDoubleAttribute("m_2_0", matrix[2][0]);
  matrixElem->SetDoubleAttribute("m_2_1", matrix[2][1]);
  matrixElem->SetDoubleAttribute("m_2_2", matrix[2][2]);
  geomElem->LinkEndChild(matrixElem);

  TiXmlElement* offsetElem = new TiXmlElement("Offset");
  offsetElem->SetAttribute("type", "Vector3D");
  offsetElem->SetDoubleAttribute("x", offset[0]);
  offsetElem->SetDoubleAttribute("y", offset[1]);
  offsetElem->SetDoubleAttribute("z", offset[2]);
  geomElem->LinkEndChild(offsetElem);

  TiXmlElement* boundsElem = new TiXmlElement("Bounds");
  TiXmlElement* boundsMinElem = new TiXmlElement("Min");
  boundsMinElem->SetAttribute("type", "Vector3D");
  boundsMinElem->SetDoubleAttribute("x", bounds[0]);
  boundsMinElem->SetDoubleAttribute("y", bounds[2]);
  boundsMinElem->SetDoubleAttribute("z", bounds[4]);
  boundsElem->LinkEndChild(boundsMinElem);
  TiXmlElement* boundsMaxElem = new TiXmlElement("Max");
  boundsMaxElem->SetAttribute("type", "Vector3D");
  boundsMaxElem->SetDoubleAttribute("x", bounds[1]);
  boundsMaxElem->SetDoubleAttribute("y", bounds[3]);
  boundsMaxElem->SetDoubleAttribute("z", bounds[5]);
  boundsElem->LinkEndChild(boundsMaxElem);
  geomElem->LinkEndChild(boundsElem);

  return geomElem;
}

mitk::Geometry3D::Pointer mitk::Geometry3DToXML::FromXML( TiXmlElement* geometryElement )
{
  if (!geometryElement)
  {
    MITK_ERROR << "Cannot deserialize Geometry3D from nullptr.";
    return nullptr;
  }

  AffineTransform3D::MatrixType matrix;
  AffineTransform3D::OffsetType offset;
  bool isImageGeometry(false);
  unsigned int frameOfReferenceID(0);
  BaseGeometry::BoundsArrayType bounds;

  if ( TIXML_SUCCESS != geometryElement->QueryUnsignedAttribute("FrameOfReferenceID", &frameOfReferenceID) )
  {
    MITK_WARN << "Missing FrameOfReference for Geometry3D.";
  }

  if ( TIXML_SUCCESS != geometryElement->QueryBoolAttribute("ImageGeometry", &isImageGeometry) )
  {
    MITK_WARN << "Missing bool ImageGeometry for Geometry3D.";
  }

  // matrix
  if ( TiXmlElement* matrixElem = geometryElement->FirstChildElement("IndexToWorld")->ToElement() )
  {
    bool matrixComplete = true;
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_0_0", &matrix[0][0]);
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_0_1", &matrix[0][1]);
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_0_2", &matrix[0][2]);
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_1_0", &matrix[1][0]);
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_1_1", &matrix[1][1]);
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_1_2", &matrix[1][2]);
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_2_0", &matrix[2][0]);
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_2_1", &matrix[2][1]);
    matrixComplete &= TIXML_SUCCESS == matrixElem->QueryDoubleAttribute("m_2_2", &matrix[2][2]);

    if ( !matrixComplete )
    {
      MITK_ERROR << "Could not parse all Geometry3D matrix coefficients!";
      return nullptr;
    }
  } else
  {
    MITK_ERROR << "Parse error: expected Matrix3x3 child below Geometry3D node";
    return nullptr;
  }

  // offset
  if ( TiXmlElement* offsetElem = geometryElement->FirstChildElement("Offset")->ToElement() )
  {
    bool vectorComplete = true;
    vectorComplete &= TIXML_SUCCESS == offsetElem->QueryDoubleAttribute("x", &offset[0]);
    vectorComplete &= TIXML_SUCCESS == offsetElem->QueryDoubleAttribute("y", &offset[1]);
    vectorComplete &= TIXML_SUCCESS == offsetElem->QueryDoubleAttribute("z", &offset[2]);

    if ( !vectorComplete )
    {
      MITK_ERROR << "Could not parse complete Geometry3D offset!";
      return nullptr;
    }
  } else
  {
    MITK_ERROR << "Parse error: expected Offset3D child below Geometry3D node";
    return nullptr;
  }

  // bounds
  if ( TiXmlElement* boundsElem = geometryElement->FirstChildElement("Bounds")->ToElement() )
  {
    bool vectorsComplete(true);
    if ( TiXmlElement* minElem = boundsElem->FirstChildElement("Min")->ToElement() )
    {
      vectorsComplete &= TIXML_SUCCESS == minElem->QueryDoubleAttribute("x", &bounds[0]);
      vectorsComplete &= TIXML_SUCCESS == minElem->QueryDoubleAttribute("y", &bounds[2]);
      vectorsComplete &= TIXML_SUCCESS == minElem->QueryDoubleAttribute("z", &bounds[4]);
    } else
    {
      vectorsComplete = false;
    }

    if ( TiXmlElement* maxElem = boundsElem->FirstChildElement("Max")->ToElement() )
    {
      vectorsComplete &= TIXML_SUCCESS == maxElem->QueryDoubleAttribute("x", &bounds[1]);
      vectorsComplete &= TIXML_SUCCESS == maxElem->QueryDoubleAttribute("y", &bounds[3]);
      vectorsComplete &= TIXML_SUCCESS == maxElem->QueryDoubleAttribute("z", &bounds[5]);
    } else
    {
      vectorsComplete = false;
    }

    if ( !vectorsComplete )
    {
      MITK_ERROR << "Could not parse complete Geometry3D bounds!";
      return nullptr;
    }
  }

  // build GeometryData from matrix/offset
  AffineTransform3D::Pointer newTransform = AffineTransform3D::New();
  newTransform->SetMatrix(matrix);
  newTransform->SetOffset(offset);

  Geometry3D::Pointer newGeometry = Geometry3D::New();
  newGeometry->SetFrameOfReferenceID(frameOfReferenceID);
  newGeometry->SetImageGeometry(isImageGeometry);

  newGeometry->SetIndexToWorldTransform(newTransform);

  newGeometry->SetBounds(bounds);

  return newGeometry;
}
