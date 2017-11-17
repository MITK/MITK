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
#include "mitkPointSetReaderService.h"
#include "mitkGeometry3DToXML.h"
#include "mitkIOMimeTypes.h"
#include "mitkProportionalTimeGeometry.h"

// STL
#include <fstream>
#include <iostream>
#include <mitkLocaleSwitch.h>

#include <tinyxml.h>

mitk::PointSetReaderService::PointSetReaderService()
  : AbstractFileReader(CustomMimeType(IOMimeTypes::POINTSET_MIMETYPE()), "MITK Point Set Reader")
{
  RegisterService();
}

mitk::PointSetReaderService::~PointSetReaderService()
{
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::PointSetReaderService::Read()
{
  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  std::vector<itk::SmartPointer<mitk::BaseData>> result;

  InputStream stream(this);

  TiXmlDocument doc;
  stream >> doc;
  if (!doc.Error())
  {
    TiXmlHandle docHandle(&doc);
    // unsigned int pointSetCounter(0);
    for (TiXmlElement *currentPointSetElement =
           docHandle.FirstChildElement("point_set_file").FirstChildElement("point_set").ToElement();
         currentPointSetElement != nullptr;
         currentPointSetElement = currentPointSetElement->NextSiblingElement())
    {
      mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();

      // time geometry assembled for addition after all points
      // else the SetPoint method would already transform the points that we provide it
      mitk::ProportionalTimeGeometry::Pointer timeGeometry = mitk::ProportionalTimeGeometry::New();

      if (currentPointSetElement->FirstChildElement("time_series") != nullptr)
      {
        for (TiXmlElement *currentTimeSeries = currentPointSetElement->FirstChildElement("time_series")->ToElement();
             currentTimeSeries != nullptr;
             currentTimeSeries = currentTimeSeries->NextSiblingElement())
        {
          unsigned int currentTimeStep(0);
          TiXmlElement *currentTimeSeriesID = currentTimeSeries->FirstChildElement("time_series_id");

          currentTimeStep = atoi(currentTimeSeriesID->GetText());

          timeGeometry->Expand(currentTimeStep + 1); // expand (default to identity) in any case
          TiXmlElement *geometryElem = currentTimeSeries->FirstChildElement("Geometry3D");
          if (geometryElem)
          {
            Geometry3D::Pointer geometry = Geometry3DToXML::FromXML(geometryElem);
            if (geometry.IsNotNull())
            {
              timeGeometry->SetTimeStepGeometry(geometry, currentTimeStep);
            }
            else
            {
              MITK_ERROR << "Could not deserialize Geometry3D element.";
            }
          }
          else
          {
            MITK_WARN << "Fallback to legacy behavior: defining PointSet geometry as identity";
          }

          newPointSet = this->ReadPoints(newPointSet, currentTimeSeries, currentTimeStep);
        }
      }
      else
      {
        newPointSet = this->ReadPoints(newPointSet, currentPointSetElement, 0);
      }

      newPointSet->SetTimeGeometry(timeGeometry);

      result.push_back(newPointSet.GetPointer());
    }
  }
  else
  {
    mitkThrow() << "Parsing error at line " << doc.ErrorRow() << ", col " << doc.ErrorCol() << ": " << doc.ErrorDesc();
  }

  return result;
}

mitk::BaseGeometry::Pointer mitk::PointSetReaderService::ReadGeometry(TiXmlElement *parentElement)
{
  TiXmlElement *geometryElem = parentElement->FirstChildElement("geometry3d");
  if (!geometryElem)
    return nullptr;

  // data to generate
  AffineTransform3D::MatrixType matrix;
  AffineTransform3D::OffsetType offset;
  bool isImageGeometry(false);
  unsigned int frameOfReferenceID(0);
  BaseGeometry::BoundsArrayType bounds;

  bool somethingMissing(false);

  // find data in xml structure
  TiXmlElement *imageGeometryElem = geometryElem->FirstChildElement("image_geometry");
  if (imageGeometryElem)
  {
    std::string igs = imageGeometryElem->GetText();
    isImageGeometry = igs == "true" || igs == "TRUE" || igs == "1";
  }
  else
    somethingMissing = true;

  TiXmlElement *frameOfReferenceElem = geometryElem->FirstChildElement("frame_of_reference_id");
  if (frameOfReferenceElem)
  {
    frameOfReferenceID = atoi(frameOfReferenceElem->GetText());
  }
  else
    somethingMissing = true;

  TiXmlElement *indexToWorldElem = geometryElem->FirstChildElement("index_to_world");
  if (indexToWorldElem)
  {
    TiXmlElement *matrixElem = indexToWorldElem->FirstChildElement("matrix3x3");
    TiXmlElement *offsetElem = indexToWorldElem->FirstChildElement("offset");
    if (indexToWorldElem && offsetElem)
    {
      TiXmlElement *col0 = matrixElem->FirstChildElement("column_0");
      TiXmlElement *col1 = matrixElem->FirstChildElement("column_1");
      TiXmlElement *col2 = matrixElem->FirstChildElement("column_2");

      if (col0 && col1 && col2)
      {
        somethingMissing |= TIXML_SUCCESS != col0->QueryDoubleAttribute("x", &matrix[0][0]);
        somethingMissing |= TIXML_SUCCESS != col0->QueryDoubleAttribute("y", &matrix[1][0]);
        somethingMissing |= TIXML_SUCCESS != col0->QueryDoubleAttribute("z", &matrix[2][0]);

        somethingMissing |= TIXML_SUCCESS != col1->QueryDoubleAttribute("x", &matrix[0][1]);
        somethingMissing |= TIXML_SUCCESS != col1->QueryDoubleAttribute("y", &matrix[1][1]);
        somethingMissing |= TIXML_SUCCESS != col1->QueryDoubleAttribute("z", &matrix[2][1]);

        somethingMissing |= TIXML_SUCCESS != col2->QueryDoubleAttribute("x", &matrix[0][2]);
        somethingMissing |= TIXML_SUCCESS != col2->QueryDoubleAttribute("y", &matrix[1][2]);
        somethingMissing |= TIXML_SUCCESS != col2->QueryDoubleAttribute("z", &matrix[2][2]);
      }
      else
        somethingMissing = true;

      somethingMissing |= TIXML_SUCCESS != offsetElem->QueryDoubleAttribute("x", &offset[0]);
      somethingMissing |= TIXML_SUCCESS != offsetElem->QueryDoubleAttribute("y", &offset[1]);
      somethingMissing |= TIXML_SUCCESS != offsetElem->QueryDoubleAttribute("z", &offset[2]);
    }
    else
      somethingMissing = true;

    TiXmlElement *boundsElem = geometryElem->FirstChildElement("bounds");
    if (boundsElem)
    {
      TiXmlElement *minBoundsElem = boundsElem->FirstChildElement("min");
      TiXmlElement *maxBoundsElem = boundsElem->FirstChildElement("max");

      if (minBoundsElem && maxBoundsElem)
      {
        somethingMissing |= TIXML_SUCCESS != minBoundsElem->QueryDoubleAttribute("x", &bounds[0]);
        somethingMissing |= TIXML_SUCCESS != minBoundsElem->QueryDoubleAttribute("y", &bounds[2]);
        somethingMissing |= TIXML_SUCCESS != minBoundsElem->QueryDoubleAttribute("z", &bounds[4]);

        somethingMissing |= TIXML_SUCCESS != maxBoundsElem->QueryDoubleAttribute("x", &bounds[1]);
        somethingMissing |= TIXML_SUCCESS != maxBoundsElem->QueryDoubleAttribute("y", &bounds[3]);
        somethingMissing |= TIXML_SUCCESS != maxBoundsElem->QueryDoubleAttribute("z", &bounds[5]);
      }
      else
        somethingMissing = true;
    }
    else
      somethingMissing = true;
  }
  else
    somethingMissing = true;

  if (somethingMissing)
  {
    MITK_ERROR << "XML structure of geometry inside a PointSet file broken. Refusing to build Geometry3D";
    return nullptr;
  }
  else
  {
    Geometry3D::Pointer g = Geometry3D::New();
    g->SetImageGeometry(isImageGeometry);
    g->SetFrameOfReferenceID(frameOfReferenceID);
    g->SetBounds(bounds);

    AffineTransform3D::Pointer transform = AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(offset);

    g->SetIndexToWorldTransform(transform);

    return g.GetPointer();
  }
}

mitk::PointSet::Pointer mitk::PointSetReaderService::ReadPoints(mitk::PointSet::Pointer newPointSet,
                                                                TiXmlElement *currentTimeSeries,
                                                                unsigned int currentTimeStep)
{
  if (currentTimeSeries->FirstChildElement("point") != nullptr)
  {
    for (TiXmlElement *currentPoint = currentTimeSeries->FirstChildElement("point")->ToElement(); currentPoint != nullptr;
         currentPoint = currentPoint->NextSiblingElement())
    {
      unsigned int id(0);
      auto spec((mitk::PointSpecificationType)0);
      double x(0.0);
      double y(0.0);
      double z(0.0);

      id = atoi(currentPoint->FirstChildElement("id")->GetText());
      if (currentPoint->FirstChildElement("specification") != nullptr)
      {
        spec = (mitk::PointSpecificationType)atoi(currentPoint->FirstChildElement("specification")->GetText());
      }
      x = atof(currentPoint->FirstChildElement("x")->GetText());
      y = atof(currentPoint->FirstChildElement("y")->GetText());
      z = atof(currentPoint->FirstChildElement("z")->GetText());

      mitk::Point3D point;
      mitk::FillVector3D(point, x, y, z);
      newPointSet->SetPoint(id, point, spec, currentTimeStep);
    }
  }
  else
  {
    if (currentTimeStep != newPointSet->GetTimeSteps() + 1)
    {
      newPointSet->Expand(currentTimeStep + 1); // expand time step series with empty time step
    }
  }
  return newPointSet;
}

mitk::PointSetReaderService::PointSetReaderService(const mitk::PointSetReaderService &other)
  : mitk::AbstractFileReader(other)
{
}

mitk::PointSetReaderService *mitk::PointSetReaderService::Clone() const
{
  return new mitk::PointSetReaderService(*this);
}
