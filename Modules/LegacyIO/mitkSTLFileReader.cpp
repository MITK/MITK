/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkSTLFileReader.h"
#include <mitkSurface.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSTLReader.h>
#include <vtkSmartPointer.h>

mitk::STLFileReader::STLFileReader() : mitk::SurfaceSource(), m_FileName("")
{
}

mitk::STLFileReader::~STLFileReader()
{
}

void mitk::STLFileReader::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();

  if (m_FileName != "")
  {
    MITK_INFO("mitkSTLFileReader") << "Loading " << m_FileName << " as stl..." << std::endl;
    vtkSmartPointer<vtkSTLReader> stlReader = vtkSmartPointer<vtkSTLReader>::New();
    stlReader->SetFileName(m_FileName.c_str());

    vtkSmartPointer<vtkPolyDataNormals> normalsGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalsGenerator->SetInputConnection(stlReader->GetOutputPort());

    vtkSmartPointer<vtkCleanPolyData> cleanPolyDataFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanPolyDataFilter->SetInputConnection(normalsGenerator->GetOutputPort());
    cleanPolyDataFilter->PieceInvariantOff();
    cleanPolyDataFilter->ConvertLinesToPointsOff();
    cleanPolyDataFilter->ConvertPolysToLinesOff();
    cleanPolyDataFilter->ConvertStripsToPolysOff();
    cleanPolyDataFilter->PointMergingOn();
    cleanPolyDataFilter->Update();

    if ((stlReader->GetOutput() != nullptr) && (cleanPolyDataFilter->GetOutput() != nullptr))
    {
      vtkSmartPointer<vtkPolyData> surfaceWithNormals = cleanPolyDataFilter->GetOutput();
      output->SetVtkPolyData(surfaceWithNormals);
    }
  }
}

bool mitk::STLFileReader::CanReadFile(const std::string filename,
                                      const std::string /*filePrefix*/,
                                      const std::string /*filePattern*/)
{
  // First check the extension
  if (filename == "")
  {
    // MITK_INFO<<"No filename specified."<<std::endl;
    return false;
  }

  bool extensionFound = false;
  std::string::size_type STLPos = filename.rfind(".stl");
  if ((STLPos != std::string::npos) && (STLPos == filename.length() - 4))
  {
    extensionFound = true;
  }

  STLPos = filename.rfind(".STL");
  if ((STLPos != std::string::npos) && (STLPos == filename.length() - 4))
  {
    extensionFound = true;
  }

  if (!extensionFound)
  {
    // MITK_INFO<<"The filename extension is not recognized."<<std::endl;
    return false;
  }

  return true;
}
