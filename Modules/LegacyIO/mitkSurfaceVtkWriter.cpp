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
const char * SurfaceVtkWriter<vtkSTLWriter>::GetDefaultFilename()
{
  return "surface";
}

template<>
const char * SurfaceVtkWriter<vtkSTLWriter>::GetFileDialogPattern()
{
  return "STL Surface (*.stl)";
}

template<>
const char * SurfaceVtkWriter<vtkSTLWriter>::GetDefaultExtension()
{
  return ".stl";
}

template<>
bool SurfaceVtkWriter<vtkSTLWriter>::CanWriteBaseDataType(BaseData::Pointer data)
{
  return dynamic_cast<mitk::Surface*>(data.GetPointer()) != NULL;
}

template<>
void SurfaceVtkWriter<vtkSTLWriter>::DoWrite(BaseData::Pointer data)
{
  if (this->CanWriteBaseDataType(data))
  {
    this->SetInput(data.GetPointer());
    this->Update();
  }
}

template<>
const char * SurfaceVtkWriter<vtkPolyDataWriter>::GetDefaultFilename()
{
  return "surface";
}

template<>
const char * SurfaceVtkWriter<vtkPolyDataWriter>::GetFileDialogPattern()
{
  return "VTK Polydata (*.vtk)";
}

template<>
const char * SurfaceVtkWriter<vtkPolyDataWriter>::GetDefaultExtension()
{
  return ".vtk";
}

template<>
bool SurfaceVtkWriter<vtkPolyDataWriter>::CanWriteBaseDataType(BaseData::Pointer data)
{
  return dynamic_cast<mitk::Surface*>(data.GetPointer()) != NULL;
}

template<>
void SurfaceVtkWriter<vtkPolyDataWriter>::DoWrite(BaseData::Pointer data)
{
  if (this->CanWriteBaseDataType(data))
  {
    this->SetInput(data.GetPointer());
    this->Update();
  }
}

template<>
const char * SurfaceVtkWriter<vtkXMLPolyDataWriter>::GetDefaultFilename()
{
  return "surface";
}

template<>
const char * SurfaceVtkWriter<vtkXMLPolyDataWriter>::GetFileDialogPattern()
{
  return "VTK XML Polydata (*.vtp)";
}

template<>
const char * SurfaceVtkWriter<vtkXMLPolyDataWriter>::GetDefaultExtension()
{
  return ".vtp";
}

template<>
bool SurfaceVtkWriter<vtkXMLPolyDataWriter>::CanWriteBaseDataType(BaseData::Pointer data)
{
  return dynamic_cast<mitk::Surface*>(data.GetPointer()) != NULL;
}

template<>
void SurfaceVtkWriter<vtkXMLPolyDataWriter>::DoWrite(BaseData::Pointer data)
{
  if (this->CanWriteBaseDataType(data))
  {
    this->SetInput(data.GetPointer());
    this->Update();
  }
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
