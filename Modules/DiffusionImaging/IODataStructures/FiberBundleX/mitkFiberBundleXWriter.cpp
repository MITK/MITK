/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-12-10 18:05:13 +0100 (Mi, 10 Dez 2008) $
Version:   $Revision: 15922 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkFiberBundleXWriter.h"
#include <vtkSmartPointer.h>
#include <vtkCleanPolyData.h>

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

    MITK_INFO << "Writing fiber bundle";
    m_Success = false;
    InputType* input = this->GetInput();
    if (input == NULL)
    {
        itkWarningMacro(<<"Sorry, input to FiberBundleXWriter is NULL!");
        return;
    }
    if ( m_FileName == "" )
    {
        itkWarningMacro( << "Sorry, filename has not been set!" );
        return ;
    }
    vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();

    vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
    cleaner->SetInput(input->GetFiberPolyData());
    cleaner->Update();

    writer->SetInput(cleaner->GetOutput());
    writer->SetFileName(m_FileName.c_str());
    writer->SetFileTypeToASCII();
    writer->Write();

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
  possibleFileExtensions.push_back(".vtk");
  return possibleFileExtensions;
}
