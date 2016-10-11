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
#include "mitkDICOMSegmentationReaderService.h"
#include "mitkIOMimeTypes.h"

// STL
#include <iostream>
#include <fstream>
#include <mitkLocaleSwitch.h>


mitk::DICOMSegmentationReaderService::DICOMSegmentationReaderService()
  : AbstractFileReader(CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), "DICOM Segmentation Reader")
{
  RegisterService();
}

mitk::DICOMSegmentationReaderService::~DICOMSegmentationReaderService()
{}

std::vector< itk::SmartPointer<mitk::BaseData> > mitk::DICOMSegmentationReaderService::Read()
{
  //// Switch the current locale to "C"
  //LocaleSwitch localeSwitch("C");

  //std::vector< itk::SmartPointer<mitk::BaseData> > result;

  //InputStream stream(this);

  //TiXmlDocument doc;
  //stream >> doc;
  //if (!doc.Error())
  //{
  //  TiXmlHandle docHandle( &doc );
  //  //unsigned int pointSetCounter(0);
  //  for( TiXmlElement* currentPointSetElement = docHandle.FirstChildElement("point_set_file").FirstChildElement("point_set").ToElement();
  //       currentPointSetElement != NULL; currentPointSetElement = currentPointSetElement->NextSiblingElement())
  //  {
  //    mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();

  //    // time geometry assembled for addition after all points
  //    // else the SetPoint method would already transform the points that we provide it
  //    mitk::ProportionalTimeGeometry::Pointer timeGeometry = mitk::ProportionalTimeGeometry::New();

  //    if(currentPointSetElement->FirstChildElement("time_series") != NULL)
  //    {
  //      for( TiXmlElement* currentTimeSeries = currentPointSetElement->FirstChildElement("time_series")->ToElement();
  //           currentTimeSeries != NULL; currentTimeSeries = currentTimeSeries->NextSiblingElement())
  //      {
  //        unsigned int currentTimeStep(0);
  //        TiXmlElement* currentTimeSeriesID = currentTimeSeries->FirstChildElement("time_series_id");

  //        currentTimeStep = atoi(currentTimeSeriesID->GetText());

  //        timeGeometry->Expand( currentTimeStep + 1 ); // expand (default to identity) in any case
  //        TiXmlElement* geometryElem = currentTimeSeries->FirstChildElement("Geometry3D");
  //        if ( geometryElem )
  //        {
  //            Geometry3D::Pointer geometry = Geometry3DToXML::FromXML(geometryElem);
  //            if (geometry.IsNotNull())
  //            {
  //              timeGeometry->SetTimeStepGeometry(geometry,currentTimeStep);
  //            }
  //            else
  //            {
  //              MITK_ERROR << "Could not deserialize Geometry3D element.";
  //            }
  //        }
  //        else
  //        {
  //            MITK_WARN << "Fallback to legacy behavior: defining PointSet geometry as identity";
  //        }

  //        newPointSet = this->ReadPoints(newPointSet, currentTimeSeries, currentTimeStep);
  //      }
  //    }
  //    else
  //    {
  //      newPointSet = this->ReadPoints(newPointSet, currentPointSetElement, 0);
  //    }

  //    newPointSet->SetTimeGeometry(timeGeometry);

  //    result.push_back( newPointSet.GetPointer() );
  //  }
  //}
  //else
  //{
  //  mitkThrow() << "Parsing error at line " << doc.ErrorRow() << ", col " << doc.ErrorCol() << ": " << doc.ErrorDesc();
  //}

  //return result;
}

mitk::DICOMSegmentationReaderService::DICOMSegmentationReaderService(const mitk::DICOMSegmentationReaderService& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::DICOMSegmentationReaderService* mitk::DICOMSegmentationReaderService::Clone() const
{
  return new mitk::DICOMSegmentationReaderService(*this);
}
