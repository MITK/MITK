/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGeometry3DToXML.h"

#include <mitkLexicalCast.h>

#include <tinyxml2.h>

#include <array>

tinyxml2::XMLElement *mitk::Geometry3DToXML::ToXML(tinyxml2::XMLDocument& doc, const Geometry3D *geom3D)
{
  assert(geom3D);

  // really serialize
  const AffineTransform3D *transform = geom3D->GetIndexToWorldTransform();

  // get transform parameters that would need to be serialized
  AffineTransform3D::MatrixType matrix = transform->GetMatrix();
  AffineTransform3D::OffsetType offset = transform->GetOffset();

  bool isImageGeometry = geom3D->GetImageGeometry();
  BaseGeometry::BoundsArrayType bounds = geom3D->GetBounds();

  // create XML file
  // construct XML tree describing the geometry
  auto *geomElem = doc.NewElement("Geometry3D");
  geomElem->SetAttribute("ImageGeometry", isImageGeometry);
  geomElem->SetAttribute("FrameOfReferenceID", geom3D->GetFrameOfReferenceID());

  // coefficients are matrix[row][column]!
  auto *matrixElem = doc.NewElement("IndexToWorld");
  matrixElem->SetAttribute("type", "Matrix3x3");
  matrixElem->SetAttribute("m_0_0", boost::lexical_cast<std::string>(matrix[0][0]).c_str());
  matrixElem->SetAttribute("m_0_1", boost::lexical_cast<std::string>(matrix[0][1]).c_str());
  matrixElem->SetAttribute("m_0_2", boost::lexical_cast<std::string>(matrix[0][2]).c_str());
  matrixElem->SetAttribute("m_1_0", boost::lexical_cast<std::string>(matrix[1][0]).c_str());
  matrixElem->SetAttribute("m_1_1", boost::lexical_cast<std::string>(matrix[1][1]).c_str());
  matrixElem->SetAttribute("m_1_2", boost::lexical_cast<std::string>(matrix[1][2]).c_str());
  matrixElem->SetAttribute("m_2_0", boost::lexical_cast<std::string>(matrix[2][0]).c_str());
  matrixElem->SetAttribute("m_2_1", boost::lexical_cast<std::string>(matrix[2][1]).c_str());
  matrixElem->SetAttribute("m_2_2", boost::lexical_cast<std::string>(matrix[2][2]).c_str());
  geomElem->InsertEndChild(matrixElem);

  auto *offsetElem = doc.NewElement("Offset");
  offsetElem->SetAttribute("type", "Vector3D");
  offsetElem->SetAttribute("x", boost::lexical_cast<std::string>(offset[0]).c_str());
  offsetElem->SetAttribute("y", boost::lexical_cast<std::string>(offset[1]).c_str());
  offsetElem->SetAttribute("z", boost::lexical_cast<std::string>(offset[2]).c_str());
  geomElem->InsertEndChild(offsetElem);

  auto *boundsElem = doc.NewElement("Bounds");
  auto *boundsMinElem = doc.NewElement("Min");
  boundsMinElem->SetAttribute("type", "Vector3D");
  boundsMinElem->SetAttribute("x", boost::lexical_cast<std::string>(bounds[0]).c_str());
  boundsMinElem->SetAttribute("y", boost::lexical_cast<std::string>(bounds[2]).c_str());
  boundsMinElem->SetAttribute("z", boost::lexical_cast<std::string>(bounds[4]).c_str());
  boundsElem->InsertEndChild(boundsMinElem);
  auto *boundsMaxElem = doc.NewElement("Max");
  boundsMaxElem->SetAttribute("type", "Vector3D");
  boundsMaxElem->SetAttribute("x", boost::lexical_cast<std::string>(bounds[1]).c_str());
  boundsMaxElem->SetAttribute("y", boost::lexical_cast<std::string>(bounds[3]).c_str());
  boundsMaxElem->SetAttribute("z", boost::lexical_cast<std::string>(bounds[5]).c_str());
  boundsElem->InsertEndChild(boundsMaxElem);
  geomElem->InsertEndChild(boundsElem);

  return geomElem;
}

mitk::Geometry3D::Pointer mitk::Geometry3DToXML::FromXML(const tinyxml2::XMLElement *geometryElement)
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

  if (tinyxml2::XML_SUCCESS != geometryElement->QueryUnsignedAttribute("FrameOfReferenceID", &frameOfReferenceID))
  {
    MITK_WARN << "Missing FrameOfReference for Geometry3D.";
  }

  if (tinyxml2::XML_SUCCESS != geometryElement->QueryBoolAttribute("ImageGeometry", &isImageGeometry))
  {
    MITK_WARN << "Missing bool ImageGeometry for Geometry3D.";
  }

  // matrix
  if (auto *matrixElem = geometryElement->FirstChildElement("IndexToWorld"))
  {
    bool matrixComplete = true;
    for (unsigned int r = 0; r < 3; ++r)
    {
      for (unsigned int c = 0; c < 3; ++c)
      {
        std::stringstream element_namer;
        element_namer << "m_" << r << "_" << c;

        const char* string_value = matrixElem->Attribute(element_namer.str().c_str());
        if (nullptr != string_value)
        {
          try
          {
            matrix[r][c] = boost::lexical_cast<double>(string_value);
          }
          catch ( const boost::bad_lexical_cast &e )
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

    if (!matrixComplete)
    {
      MITK_ERROR << "Could not parse all Geometry3D matrix coefficients!";
      return nullptr;
    }
  }
  else
  {
    MITK_ERROR << "Parse error: expected Matrix3x3 child below Geometry3D node";
    return nullptr;
  }

  // offset
  if (auto *offsetElem = geometryElement->FirstChildElement("Offset"))
  {
    std::array<const char*, 3> offset_string = {
      offsetElem->Attribute("x"),
      offsetElem->Attribute("y"),
      offsetElem->Attribute("z")
    };

    if (nullptr == offset_string[0] || nullptr == offset_string[1] || nullptr == offset_string[2])
    {
      MITK_ERROR << "Could not parse complete Geometry3D offset!";
      return nullptr;
    }

    for (unsigned int d = 0; d < 3; ++d)
      try
      {
        offset[d] = boost::lexical_cast<double>(offset_string[d]);
      }
      catch ( const boost::bad_lexical_cast &e )
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
  if (auto *boundsElem = geometryElement->FirstChildElement("Bounds"))
  {
    bool vectorsComplete;
    std::array<const char*, 6> bounds_string;
    if (auto* minElem = boundsElem->FirstChildElement("Min"))
    {
      bounds_string[0] = minElem->Attribute("x");
      bounds_string[2] = minElem->Attribute("y");
      bounds_string[4] = minElem->Attribute("z");

      vectorsComplete = !(nullptr == bounds_string[0] || nullptr == bounds_string[2] || nullptr == bounds_string[4]);
    }
    else
    {
      vectorsComplete = false;
    }

    if (auto *maxElem = boundsElem->FirstChildElement("Max"))
    {
      bounds_string[1] = maxElem->Attribute("x");
      bounds_string[3] = maxElem->Attribute("y");
      bounds_string[5] = maxElem->Attribute("z");

      vectorsComplete = !(nullptr == bounds_string[1] || nullptr == bounds_string[3] || nullptr == bounds_string[5]);
    }
    else
    {
      vectorsComplete = false;
    }

    if (!vectorsComplete)
    {
      MITK_ERROR << "Could not parse complete Geometry3D bounds!";
      return nullptr;
    }

    for (unsigned int d = 0; d < 6; ++d)
      try
      {
        bounds[d] = boost::lexical_cast<double>(bounds_string[d]);
      }
      catch ( const boost::bad_lexical_cast &e )
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
