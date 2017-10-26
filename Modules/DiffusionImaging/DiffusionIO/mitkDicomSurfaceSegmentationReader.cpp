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

#include "mitkDicomSurfaceSegmentationReader.h"
#include <itkMetaDataObject.h>
#include <mitkCustomMimeType.h>
#include "mitkDiffusionIOMimeTypes.h"
#include "vtkGDCMPolyDataReader.h"
#include <vtkAppendPolyData.h>
#include <mitkSurface.h>

mitk::DicomSurfaceSegmentationReader::DicomSurfaceSegmentationReader()
  : mitk::AbstractFileReader( mitk::DiffusionIOMimeTypes::SURFACESEGMENTATION_DICOM_MIMETYPE_NAME(), "DICOM SegmentationSurface" )
{
  m_ServiceReg = this->RegisterService();
}

mitk::DicomSurfaceSegmentationReader::DicomSurfaceSegmentationReader(const DicomSurfaceSegmentationReader &other)
  :mitk::AbstractFileReader(other)
{
}

mitk::DicomSurfaceSegmentationReader * mitk::DicomSurfaceSegmentationReader::Clone() const
{
  return new DicomSurfaceSegmentationReader(*this);
}


std::vector<itk::SmartPointer<mitk::BaseData> > mitk::DicomSurfaceSegmentationReader::Read()
{
  std::vector<itk::SmartPointer<mitk::BaseData> > result;

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, nullptr );
  setlocale(LC_ALL, locale.c_str());

  std::string filename = this->GetInputLocation();

  try
  {
    MITK_INFO << "Loading DICOM surface segmentation.";

    vtkSmartPointer< vtkPolyData > test = vtkSmartPointer< vtkPolyData >::New();
    vtkSmartPointer< vtkGDCMPolyDataReader > reader = vtkSmartPointer< vtkGDCMPolyDataReader >::New();
    reader->SetFileName( filename.c_str() );
    reader->Update();

    vtkSmartPointer< vtkAppendPolyData > append = vtkSmartPointer< vtkAppendPolyData >::New();

    int n = reader->GetNumberOfOutputPorts();
    for(int i = 0; i < n; ++i)
    {
      append->AddInputData( reader->GetOutput(i) );
    }
    vtkSmartPointer< vtkPolyData > vtkSurface = append->GetOutput();

    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(vtkSurface);
    result.push_back(surface.GetPointer());
  }
  catch(...)
  {
    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Reading DICOM segmentation surface failed";
    throw;
  }

  setlocale(LC_ALL, currLocale.c_str());
  MITK_INFO << "DICOM segmentation surface read";

  return result;
}
