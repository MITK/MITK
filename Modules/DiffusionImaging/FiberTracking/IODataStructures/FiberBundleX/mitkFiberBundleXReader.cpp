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

#include "mitkFiberBundleXReader.h"
#include <itkMetaDataObject.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <itksys/SystemTools.hxx>
#include <tinyxml.h>
#include <vtkCleanPolyData.h>
#include <mitkTrackvis.h>
#include <mitkCustomMimeType.h>


mitk::FiberBundleXReader::FiberBundleXReader()
  :mitk::AbstractFileReader()
{
  std::string category = "Fiber Bundle File";
  mitk::CustomMimeType customMimeType;
  customMimeType.SetCategory(category);
  customMimeType.AddExtension("fib");
  customMimeType.AddExtension("trk");
  customMimeType.AddExtension("vtk");

  this->SetDescription(category);
  this->SetMimeType(customMimeType);

  m_ServiceReg = this->RegisterService();
}

mitk::FiberBundleXReader::FiberBundleXReader(const FiberBundleXReader &other)
  :mitk::AbstractFileReader(other)
{
}

mitk::FiberBundleXReader * mitk::FiberBundleXReader::Clone() const
{
  return new FiberBundleXReader(*this);
}


std::vector<itk::SmartPointer<mitk::BaseData> > mitk::FiberBundleXReader::Read()
{

  std::vector<itk::SmartPointer<mitk::BaseData> > result;
  try
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );
    setlocale(LC_ALL, locale.c_str());

    std::string filename = this->GetInputLocation();

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext==".trk")
    {
      FiberBundleX::Pointer image = FiberBundleX::New();
      TrackVisFiberReader reader;
      reader.open(this->GetInputLocation().c_str());
      reader.read(image.GetPointer());
      result.push_back(image.GetPointer());
      return result;
    }

    vtkSmartPointer<vtkDataReader> chooser=vtkSmartPointer<vtkDataReader>::New();
    chooser->SetFileName( this->GetInputLocation().c_str() );
    if( chooser->IsFilePolyData())
    {
      vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
      reader->SetFileName( this->GetInputLocation().c_str() );
      reader->Update();

      if ( reader->GetOutput() != NULL )
      {
        vtkSmartPointer<vtkPolyData> fiberPolyData = reader->GetOutput();
        FiberBundleX::Pointer image = FiberBundleX::New(fiberPolyData);
        result.push_back(image.GetPointer());
        return result;
      }
    }
    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Fiber bundle read";
  }
  catch(...)
  {
    throw;
  }
  return result;
}
