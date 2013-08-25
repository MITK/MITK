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

#include "mitkFiberBundleXWriter.h"
#include <vtkSmartPointer.h>
#include <vtkCleanPolyData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTrackvis.h>
#include <itkSize.h>

mitk::FiberBundleXWriter::FiberBundleXWriter()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
    this->SetNumberOfRequiredInputs( 1 );
}


mitk::FiberBundleXWriter::~FiberBundleXWriter()
{}


void mitk::FiberBundleXWriter::GenerateData()
{
  try
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );
    setlocale(LC_ALL, locale.c_str());

    m_Success = false;
    InputType* input = this->GetInput();
    if (input == NULL)
    {
        itkWarningMacro(<<"Sorry, input to FiberBundleXWriter is NULL!");
        return;
    }
    else if ( m_FileName == "" )
    {
        itkWarningMacro( << "Sorry, filename has not been set!" );
        return ;
    }

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);

    if (ext==".fib" || ext==".vtk")
    {
        MITK_INFO << "Writing fiber bundle as binary VTK";
        vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
        writer->SetInput(input->GetFiberPolyData());
        writer->SetFileName(m_FileName.c_str());
        writer->SetFileTypeToBinary();
        writer->Write();
    }
    else if (ext==".afib")
    {
        itksys::SystemTools::ReplaceString(m_FileName,".afib",".fib");
        MITK_INFO << "Writing fiber bundle as ascii VTK";
        vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
        writer->SetInput(input->GetFiberPolyData());
        writer->SetFileName(m_FileName.c_str());
        writer->SetFileTypeToASCII();
        writer->Write();
    }
    else if (ext==".avtk")
    {
        itksys::SystemTools::ReplaceString(m_FileName,".avtk",".vtk");
        MITK_INFO << "Writing fiber bundle as ascii VTK";
        vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
        writer->SetInput(input->GetFiberPolyData());
        writer->SetFileName(m_FileName.c_str());
        writer->SetFileTypeToASCII();
        writer->Write();
    }
    else if (ext==".trk")
    {
        MITK_INFO << "Writing fiber bundle as TRK";
        TrackVis trk;
        itk::Size<3> size;
        size.SetElement(0,input->GetGeometry()->GetExtent(0));
        size.SetElement(1,input->GetGeometry()->GetExtent(1));
        size.SetElement(2,input->GetGeometry()->GetExtent(2));
        trk.create(m_FileName,size,input->GetGeometry()->GetOrigin());
        trk.writeHdr();
        trk.append(input);
    }

    setlocale(LC_ALL, currLocale.c_str());
    m_Success = true;
    MITK_INFO << "Fiber bundle written";
  }
  catch(...)
  {
    throw;
  }
}


void mitk::FiberBundleXWriter::SetInputFiberBundleX( InputType* diffVolumes )
{
    this->ProcessObject::SetNthInput( 0, diffVolumes );
}


mitk::FiberBundleX* mitk::FiberBundleXWriter::GetInput()
{
    if ( this->GetNumberOfInputs() < 1 )
    {
        return NULL;
    }
    else
    {
        return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( 0 ) );
    }
}


std::vector<std::string> mitk::FiberBundleXWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".fib");
  possibleFileExtensions.push_back(".afib");
  possibleFileExtensions.push_back(".vtk");
  possibleFileExtensions.push_back(".avtk");
  possibleFileExtensions.push_back(".trk");
  return possibleFileExtensions;
}
