/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 10025 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUnstructuredGridVtkWriter.h"
#include "mitkUnstructuredGridVtkWriter.txx"

#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

namespace mitk {


template<>
void UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::SetDefaultExtension()
{
  m_Extension = ".vtk";
  m_WriterWriteHasReturnValue = false;
}

template<>
void UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::SetDefaultExtension()
{
  m_Extension = ".vtu";
  m_WriterWriteHasReturnValue = true;
}

template<>
void UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::ExecuteWrite( VtkWriterType* m_VtkWriter, vtkTransformFilter* transformPointSet )
{
  if (!m_VtkWriter->Write())
  {
    transformPointSet->Delete();
    throw std::ios_base::failure("Error during unstructured grid writing.");
  }
}

template<>
std::vector<std::string> UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".vtk");
  return possibleFileExtensions;
}

template<>
std::vector<std::string> UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".vtu");
  return possibleFileExtensions;
}

template class MitkExt_EXPORT UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>;

template class MitkExt_EXPORT UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>;

}

