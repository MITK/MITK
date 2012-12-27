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

#include <vtkErrorCode.h>

#include "mitkSurfaceVtkWriter.h"
#include "mitkSurfaceVtkWriter.txx"


namespace mitk {

template<>
void SurfaceVtkWriter<vtkSTLWriter>::SetDefaultExtension()
{
  m_Extension = ".stl";
  m_WriterWriteHasReturnValue = false;
}

template<>
void SurfaceVtkWriter<vtkPolyDataWriter>::SetDefaultExtension()
{
  m_Extension = ".vtk";
  m_WriterWriteHasReturnValue = false;
}

template<>
void SurfaceVtkWriter<vtkXMLPolyDataWriter>::SetDefaultExtension()
{
  m_Extension = ".vtp";
  m_WriterWriteHasReturnValue = true;
}

template<>
void SurfaceVtkWriter<vtkXMLPolyDataWriter>::ExecuteWrite( VtkWriterType* vtkWriter )
{
  if ( vtkWriter->Write() == 0 || vtkWriter->GetErrorCode() != 0 )
  {
    itkExceptionMacro(<<"Error during surface writing: " << vtkErrorCode::GetStringFromErrorCode(vtkWriter->GetErrorCode()) );
  }
}

template<>
std::vector<std::string> SurfaceVtkWriter<vtkSTLWriter>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".stl");
  possibleFileExtensions.push_back(".obj");
  return possibleFileExtensions;
}

template<>
std::vector<std::string> SurfaceVtkWriter<vtkPolyDataWriter>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".vtk");
  possibleFileExtensions.push_back(".obj");
  return possibleFileExtensions;
}

template<>
std::vector<std::string> SurfaceVtkWriter<vtkXMLPolyDataWriter>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".vtp");
  possibleFileExtensions.push_back(".obj");
  return possibleFileExtensions;
}

template class SurfaceVtkWriter<vtkSTLWriter>;
template class SurfaceVtkWriter<vtkPolyDataWriter>;
template class SurfaceVtkWriter<vtkXMLPolyDataWriter>;

}


