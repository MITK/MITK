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

// MITK
#include "mitkGeometryDataReaderService.h"
#include "mitkIOMimeTypes.h"

// STL
#include <iostream>
#include <fstream>
#include <mitkLocaleSwitch.h>

#include <tinyxml.h>

mitk::CustomMimeType mitk::GeometryDataReaderService::GEOMETRY_DATA_MIMETYPE()
{
  mitk::CustomMimeType mimeType( IOMimeTypes::DEFAULT_BASE_NAME() + ".geometrydata" );
  mimeType.AddExtension("mitkgeometry");
  mimeType.SetCategory("Geometries");
  mimeType.SetComment("GeometryData object");
  return mimeType;
}

mitk::GeometryDataReaderService::GeometryDataReaderService()
  : AbstractFileReader( GEOMETRY_DATA_MIMETYPE(), "MITK Geometry Data Reader")
{
  RegisterService();
}

mitk::GeometryDataReaderService::~GeometryDataReaderService()
{
}

std::vector< itk::SmartPointer<mitk::BaseData> > mitk::GeometryDataReaderService::Read()
{
  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  std::vector< itk::SmartPointer<BaseData> > result;

  InputStream stream(this);

  TiXmlDocument doc;
  stream >> doc;
  if (!doc.Error())
  {
    TiXmlHandle docHandle( &doc );

    for( TiXmlElement* currentGeometryElement = docHandle.FirstChild("GeometryData").FirstChild("Geometry3D").ToElement();
         currentGeometryElement != NULL;
         currentGeometryElement = currentGeometryElement->NextSiblingElement())
    {
      AffineTransform3D::MatrixType matrix;
      AffineTransform3D::OffsetType offset;
      bool isImageGeometry(false);
      unsigned int frameOfReferenceID(0);
      BaseGeometry::BoundsArrayType bounds;
      Point3D origin;
      Vector3D spacing;

      if ( TIXML_SUCCESS != currentGeometryElement->QueryUnsignedAttribute("FrameOfReferenceID", &frameOfReferenceID) )
      {
          MITK_WARN << "Missing FrameOfReference for Geometry3D.";
      }

      if ( TIXML_SUCCESS != currentGeometryElement->QueryBoolAttribute("ImageGeometry", &isImageGeometry) )
      {
          MITK_WARN << "Missing bool ImageGeometry for Geometry3D.";
      }

      // matrix
      if (TiXmlElement* matrixElem = currentGeometryElement->FirstChildElement("IndexToWorld")->ToElement())
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

        if (!matrixComplete)
        {
            MITK_ERROR << "Could not parse all Geometry3D matrix coefficients!";
            break;
        }
      }
      else
      {
        MITK_ERROR << "Parse error: expected Matrix3x3 child below Geometry3D node";
      }

      // offset
      if (TiXmlElement* offsetElem = currentGeometryElement->FirstChildElement("Offset")->ToElement())
      {
        bool vectorComplete = true;
        vectorComplete &= TIXML_SUCCESS == offsetElem->QueryDoubleAttribute("x", &offset[0]);
        vectorComplete &= TIXML_SUCCESS == offsetElem->QueryDoubleAttribute("y", &offset[1]);
        vectorComplete &= TIXML_SUCCESS == offsetElem->QueryDoubleAttribute("z", &offset[2]);

        if (!vectorComplete)
        {
            MITK_ERROR << "Could not parse complete Geometry3D offset!";
            break;
        }
      }
      else
      {
        MITK_ERROR << "Parse error: expected Offset3D child below Geometry3D node";
      }

      // bounds
      if (TiXmlElement* boundsElem = currentGeometryElement->FirstChildElement("Bounds")->ToElement())
      {
          bool vectorsComplete(true);
          if (TiXmlElement* minElem = boundsElem->FirstChildElement("Min")->ToElement())
          {
            vectorsComplete &= TIXML_SUCCESS == minElem->QueryDoubleAttribute("x", &bounds[0]);
            vectorsComplete &= TIXML_SUCCESS == minElem->QueryDoubleAttribute("y", &bounds[2]);
            vectorsComplete &= TIXML_SUCCESS == minElem->QueryDoubleAttribute("z", &bounds[4]);
          }
          else
          {
              vectorsComplete = false;
          }

          if (TiXmlElement* maxElem = boundsElem->FirstChildElement("Max")->ToElement())
          {
            vectorsComplete &= TIXML_SUCCESS == maxElem->QueryDoubleAttribute("x", &bounds[1]);
            vectorsComplete &= TIXML_SUCCESS == maxElem->QueryDoubleAttribute("y", &bounds[3]);
            vectorsComplete &= TIXML_SUCCESS == maxElem->QueryDoubleAttribute("z", &bounds[5]);
          }
          else
          {
              vectorsComplete = false;
          }

          if (!vectorsComplete)
          {
            MITK_ERROR << "Could not parse complete Geometry3D bounds!";
            break;
          }
       }

      // origin + spacing
      if (TiXmlElement* originElem = currentGeometryElement->FirstChildElement("Origin")->ToElement())
      {
        bool vectorComplete = true;
        vectorComplete &= TIXML_SUCCESS == originElem->QueryDoubleAttribute("x", &origin[0]);
        vectorComplete &= TIXML_SUCCESS == originElem->QueryDoubleAttribute("y", &origin[1]);
        vectorComplete &= TIXML_SUCCESS == originElem->QueryDoubleAttribute("z", &origin[2]);

        if (!vectorComplete)
        {
            MITK_ERROR << "Could not parse complete Geometry3D origin!";
            break;
        }
      }
      else
      {
        MITK_ERROR << "Parse error: expected Origin child below Geometry3D node";
      }

      if (TiXmlElement* spacingElem = currentGeometryElement->FirstChildElement("Spacing")->ToElement())
      {
        bool vectorComplete = true;
        vectorComplete &= TIXML_SUCCESS == spacingElem->QueryDoubleAttribute("x", &spacing[0]);
        vectorComplete &= TIXML_SUCCESS == spacingElem->QueryDoubleAttribute("y", &spacing[1]);
        vectorComplete &= TIXML_SUCCESS == spacingElem->QueryDoubleAttribute("z", &spacing[2]);

        if (!vectorComplete)
        {
            MITK_ERROR << "Could not parse complete Geometry3D spacing!";
            break;
        }
      }
      else
      {
        MITK_ERROR << "Parse error: expected Spacing child below Geometry3D node";
      }


      // build GeometryData from matrix/offset
      AffineTransform3D::Pointer newTransform = AffineTransform3D::New();
      newTransform->SetMatrix( matrix );
      newTransform->SetOffset( offset );
      
      Geometry3D::Pointer newGeometry = Geometry3D::New();
      newGeometry->SetFrameOfReferenceID( frameOfReferenceID );
      newGeometry->SetImageGeometry(isImageGeometry);
      
      newGeometry->SetIndexToWorldTransform( newTransform );

      newGeometry->SetBounds(bounds);
      newGeometry->SetOrigin(origin);
      newGeometry->SetSpacing( spacing ); // TODO parameter?!


      GeometryData::Pointer newGeometryData = GeometryData::New();
      newGeometryData->SetGeometry( newGeometry );

      result.push_back( newGeometryData.GetPointer() );
    }
  }
  else
  {
    mitkThrow() << "Parsing error at line " << doc.ErrorRow() << ", col " << doc.ErrorCol() << ": " << doc.ErrorDesc();
  }

  return result;
}

mitk::GeometryDataReaderService::GeometryDataReaderService(const mitk::GeometryDataReaderService& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::GeometryDataReaderService* mitk::GeometryDataReaderService::Clone() const
{
  return new GeometryDataReaderService(*this);
}