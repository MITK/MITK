/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkSTLFileReader.h"
#include <mitkSurface.h>
#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>


mitk::STLFileReader::STLFileReader() : mitk::SurfaceSource(), m_FileName("")
{
}

mitk::STLFileReader::~STLFileReader()
{
}

void mitk::STLFileReader::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();

  if( m_FileName != "")
  {
    MITK_INFO << "Loading " << m_FileName << " as stl..." << std::endl;
    vtkSmartPointer<vtkSTLReader> stlReader = vtkSmartPointer<vtkSTLReader>::New();
    stlReader->SetFileName( m_FileName.c_str() );

    vtkSmartPointer<vtkPolyDataNormals> normalsGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalsGenerator->SetInput( stlReader->GetOutput() );

    vtkSmartPointer<vtkCleanPolyData> cleanPolyDataFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanPolyDataFilter->SetInput(normalsGenerator->GetOutput());
    cleanPolyDataFilter->PieceInvariantOff();
    cleanPolyDataFilter->ConvertLinesToPointsOff();
    cleanPolyDataFilter->ConvertPolysToLinesOff();
    cleanPolyDataFilter->ConvertStripsToPolysOff();
    cleanPolyDataFilter->PointMergingOn();
    cleanPolyDataFilter->Update();

    if ( ( stlReader->GetOutput() != NULL ) && ( cleanPolyDataFilter->GetOutput() != NULL ) )
    {
      vtkSmartPointer<vtkPolyData> surfaceWithNormals = cleanPolyDataFilter->GetOutput();
      output->SetVtkPolyData( surfaceWithNormals );
    }
  }
}

bool mitk::STLFileReader::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/) 
{
  // First check the extension
  if(  filename == "" )
    {
      //MITK_INFO<<"No filename specified."<<std::endl;
    return false;
    }

  bool extensionFound = false;
  std::string::size_type STLPos = filename.rfind(".stl");
  if ((STLPos != std::string::npos)
      && (STLPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  STLPos = filename.rfind(".STL");
  if ((STLPos != std::string::npos)
      && (STLPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  if( !extensionFound )
    {
      //MITK_INFO<<"The filename extension is not recognized."<<std::endl;
    return false;
    }

  return true;
}
