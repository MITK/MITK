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

#include <boost/lexical_cast.hpp>

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
  matrixElem->SetAttribute("m_0_0", boost::lexical_cast<std::string>(matrix[0][0]));
  matrixElem->SetAttribute("m_0_1", boost::lexical_cast<std::string>(matrix[0][1]));
  matrixElem->SetAttribute("m_0_2", boost::lexical_cast<std::string>(matrix[0][2]));
  matrixElem->SetAttribute("m_1_0", boost::lexical_cast<std::string>(matrix[1][0]));
  matrixElem->SetAttribute("m_1_1", boost::lexical_cast<std::string>(matrix[1][1]));
  matrixElem->SetAttribute("m_1_2", boost::lexical_cast<std::string>(matrix[1][2]));
  matrixElem->SetAttribute("m_2_0", boost::lexical_cast<std::string>(matrix[2][0]));
  matrixElem->SetAttribute("m_2_1", boost::lexical_cast<std::string>(matrix[2][1]));
  matrixElem->SetAttribute("m_2_2", boost::lexical_cast<std::string>(matrix[2][2]));
  geomElem->LinkEndChild(matrixElem);

  TiXmlElement* offsetElem = new TiXmlElement("Offset");
  offsetElem->SetAttribute("type", "Vector3D");
  offsetElem->SetAttribute("x", boost::lexical_cast<std::string>(offset[0]));
  offsetElem->SetAttribute("y", boost::lexical_cast<std::string>(offset[1]));
  offsetElem->SetAttribute("z", boost::lexical_cast<std::string>(offset[2]));
  geomElem->LinkEndChild(offsetElem);

  TiXmlElement* boundsElem = new TiXmlElement("Bounds");
  TiXmlElement* boundsMinElem = new TiXmlElement("Min");
  boundsMinElem->SetAttribute("type", "Vector3D");
  boundsMinElem->SetAttribute("x", boost::lexical_cast<std::string>(bounds[0]));
  boundsMinElem->SetAttribute("y", boost::lexical_cast<std::string>(bounds[2]));
  boundsMinElem->SetAttribute("z", boost::lexical_cast<std::string>(bounds[4]));
  boundsElem->LinkEndChild(boundsMinElem);
  TiXmlElement* boundsMaxElem = new TiXmlElement("Max");
  boundsMaxElem->SetAttribute("type", "Vector3D");
  boundsMaxElem->SetAttribute("x", boost::lexical_cast<std::string>(bounds[1]));
  boundsMaxElem->SetAttribute("y", boost::lexical_cast<std::string>(bounds[3]));
  boundsMaxElem->SetAttribute("z", boost::lexical_cast<std::string>(bounds[5]));
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
    for ( unsigned int r = 0; r < 3; ++r )
    {
      for ( unsigned int c = 0; c < 3; ++c )
      {
        std::stringstream element_namer;
        element_namer << "m_" << r << "_" << c;

        std::string string_value;
        if (TIXML_SUCCESS == matrixElem->QueryStringAttribute(element_namer.str().c_str(),
                                                              &string_value))
        {
          try
          {
            matrix[r][c] = boost::lexical_cast<double>(string_value);
          }
          catch (boost::bad_lexical_cast& e)
          {
            MITK_ERROR << "Could not parse '" << string_value << "' as number: " << e.what();
            return nullptr;
          }
        }
        else
        {
          matrixComplete = false;
        }
      }
    }

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
    std::string offset_string[3];
    vectorComplete &= TIXML_SUCCESS == offsetElem->QueryStringAttribute("x", &offset_string[0]);
    vectorComplete &= TIXML_SUCCESS == offsetElem->QueryStringAttribute("y", &offset_string[1]);
    vectorComplete &= TIXML_SUCCESS == offsetElem->QueryStringAttribute("z", &offset_string[2]);

    if ( !vectorComplete )
    {
      MITK_ERROR << "Could not parse complete Geometry3D offset!";
      return nullptr;
    }

    for ( unsigned int d = 0; d < 3; ++d )
      try
      {
        offset[d] = boost::lexical_cast<double>(offset_string[d]);
      }
      catch ( boost::bad_lexical_cast& e )
      {
        MITK_ERROR << "Could not parse '" << offset_string[d] << "' as number: " << e.what();
        return nullptr;
      }
  }
  else
  {
    MITK_ERROR << "Parse error: expected Offset3D child below Geometry3D node";
    return nullptr;
  }

  // bounds
  if ( TiXmlElement* boundsElem = geometryElement->FirstChildElement("Bounds")->ToElement() )
  {
    bool vectorsComplete(true);
    std::string bounds_string[6];
    if ( TiXmlElement* minElem = boundsElem->FirstChildElement("Min")->ToElement() )
    {
      vectorsComplete &= TIXML_SUCCESS == minElem->QueryStringAttribute("x", &bounds_string[0]);
      vectorsComplete &= TIXML_SUCCESS == minElem->QueryStringAttribute("y", &bounds_string[2]);
      vectorsComplete &= TIXML_SUCCESS == minElem->QueryStringAttribute("z", &bounds_string[4]);
    } else
    {
      vectorsComplete = false;
    }

    if ( TiXmlElement* maxElem = boundsElem->FirstChildElement("Max")->ToElement() )
    {
      vectorsComplete &= TIXML_SUCCESS == maxElem->QueryStringAttribute("x", &bounds_string[1]);
      vectorsComplete &= TIXML_SUCCESS == maxElem->QueryStringAttribute("y", &bounds_string[3]);
      vectorsComplete &= TIXML_SUCCESS == maxElem->QueryStringAttribute("z", &bounds_string[5]);
    } else
    {
      vectorsComplete = false;
    }

    if ( !vectorsComplete )
    {
      MITK_ERROR << "Could not parse complete Geometry3D bounds!";
      return nullptr;
    }

    for (unsigned int d = 0; d < 6; ++d)
      try
      {
        bounds[d] = boost::lexical_cast<double>(bounds_string[d]);
      }
      catch (boost::bad_lexical_cast& e)
      {
        MITK_ERROR << "Could not parse '" << bounds_string[d] << "' as number: " << e.what();
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
